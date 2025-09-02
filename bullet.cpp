//=============================================================================
//
// 弾発射処理 [bullet.cpp]
// Author : 
//
//=============================================================================
#include "debugproc.h"

#include "main.h"
#include "renderer.h"
#include "shadow.h"
#include "bullet.h"
#include "camera.h"
#include "player.h"
#include "Octree.h"
#include "FBXmodel.h"
#include "meshfield.h"
#include <math.h>
#include <vector>
#include "enemy.h"
#include "item.h"


//=============================================================================
// 弾の基本データ構造（属性など） //追加箇所
//=============================================================================
//                                  種類　　　　速さ  DMG  scl  lifetime    　　モデル　　　　　　　　RGB
BulletData bulletData_Normal = { BULLET_NORMAL,  15.0f, 10, 0.2f, 200.0f, "data/MODEL/NormalBullet.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/ };
BulletData bulletData_Fire = { BULLET_FIRE,     8.0f, 20, 0.6f, 200.0f, "data/MODEL/FireBullet.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/ };


// 武器インスタンス 
Weapon g_Revolver;
Weapon g_Shotgun;
Weapon g_RocketLauncher;

// 弾のインスタンス配列
BULLET g_Bullet[MAX_BULLET];

//==========================================================================
// 爆風
//==========================================================================


namespace {
    // 爆発チューニング用パラメータ（必要に応じて調整）
    constexpr float kExplosionRadius = 100.0f;   // 爆風半径
    constexpr float kExplosionForce = 30.0f;   // 吹き飛ばし強さ
    constexpr float kUpwardBoost = 0.6f;   // 上向き成分の強さ（ちょっと浮かせる）

    inline float Length3(const XMFLOAT3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
    inline XMFLOAT3 Normalize(const XMFLOAT3& v) {
        float len = Length3(v);
        if (len < 1e-5f) return XMFLOAT3(0, 0, 0);
        return XMFLOAT3(v.x / len, v.y / len, v.z / len);
    }

    // 任意の Object にラジアルなノックバックを与える
    void ApplyImpulseToObject(Object* obj, const XMFLOAT3& center,
        float radius, float force)
    {
        if (!obj) return;
        XMFLOAT3 p = obj->GetPosition();
        XMFLOAT3 dir = XMFLOAT3(p.x - center.x, p.y - center.y, p.z - center.z);
        float d = Length3(dir);
        if (d > radius) return;

        // 線形減衰（中心ほど強く、端で0）
        float falloff = 1.0f - (d / radius);
        XMFLOAT3 n = Normalize(dir);

        // 少し上向き成分を足す（“吹き飛ぶ”見た目に）
        n.y += kUpwardBoost;
        n = Normalize(n);

        XMFLOAT3 impulse = XMFLOAT3(n.x * force * falloff,
            n.y * force * falloff,
            n.z * force * falloff);

        // 既存速度に加算（AddForce）。速度を即時置換したいなら SetVelocity を使う
        obj->AddForce(impulse);
    }
}

// 爆風本体：敵・アイテムに適用（必要ならプレイヤー等にも拡張可）
static void ApplyExplosionImpulse(const XMFLOAT3& center,
    float radius = kExplosionRadius,
    float force = kExplosionForce)
{
    // 1) 敵
    auto& enemies = GetEnemies(); // vector<BaseEnemy*>
    for (auto* e : enemies) {
        if (!e) continue;
        ApplyImpulseToObject(e, center, radius, force);
    }

    // 2) アイテム（配列管理で IsUsed() を見られる前提）
    ITEM_OBJ* items = GetItemOBJ();
    if (items) {
        const int n = GetItemCount();  // ← 追加
        for (int i = 0; i < n; ++i) {
            if (!items[i].IsUsed()) continue;
            ApplyImpulseToObject(&items[i], center, radius, force * 0.6f);
        }
    }
}

void ApplyExplosionAt(const XMFLOAT3& center, float radius, float force) {
    ApplyExplosionImpulse(center, radius, force);
}


//=============================================================================
// 初期化
//=============================================================================
HRESULT InitBullet(void)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        g_Bullet[i].use = FALSE;
        g_Bullet[i].isLoaded = FALSE;
    }

    // 武器ごとの弾をセット 
    g_Revolver.weaponType = WEAPON_REVOLVER;
    g_Revolver.bulletData = &bulletData_Normal;
    g_Revolver.clipSize = 5;      //リロードできる弾数

    g_Shotgun.weaponType = WEAPON_SHOTGUN;
    g_Shotgun.bulletData = &bulletData_Normal;
    g_Shotgun.clipSize = 3;       //リロードできる弾数

    g_RocketLauncher.weaponType = WEAPON_ROCKET_LAUNCHER;
    g_RocketLauncher.bulletData = &bulletData_Normal;
    g_RocketLauncher.clipSize = 5;

    return S_OK;
}


//=====================================================
//
//=====================================================
void UninitBullet()
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (g_Bullet[i].isLoaded)
        {
            UnloadModel(&g_Bullet[i].model);
            g_Bullet[i].isLoaded = FALSE;
        }
        g_Bullet[i].use = FALSE;
    }
}

//=============================================================================
// 弾の発射（共通）
//=============================================================================
int SetBullet(XMFLOAT3 pos, XMFLOAT3 rot, BulletData data, WeaponType firedBy)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (!g_Bullet[i].use)
        {
            g_Bullet[i].use = TRUE;
            g_Bullet[i].pos = pos;
            g_Bullet[i].rot = rot;
            g_Bullet[i].spd = data.speed;
            g_Bullet[i].size = data.size;
            LoadModel(const_cast<char*>(data.modelPath), &g_Bullet[i].model);
            g_Bullet[i].isLoaded = TRUE;
            g_Bullet[i].fWidth = 1.0f;
            g_Bullet[i].fHeight = 1.0f;
            g_Bullet[i].lifetime = data.lifetime;
            g_Bullet[i].firedByWeapon = firedBy;

            //g_Bullet[i].color = data.color;

            XMVECTOR dir = XMVectorSet(
                sinf(rot.y) * cosf(rot.x),
                sinf(rot.x),
                cosf(rot.y) * cosf(rot.x),
                0.0f
            );
            dir = XMVector3Normalize(dir);
            dir = XMVectorScale(dir, data.speed);
            XMStoreFloat3(&g_Bullet[i].vel, dir);
            break;
        }
    }
    return 0;
}

//弾の情報（data）をもとに弾を発射する関数//
int SetBulletWithData(const BulletData& data, XMFLOAT3 pos, XMFLOAT3 rot, WeaponType firedBy)
{
    return SetBullet(pos, rot, data, firedBy);
}
//=============================================================================
// リボルバー弾の発射関数（分かりやすさのため） //追加箇所
//=============================================================================
void SetRevolverBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot)
{
    const BulletData& data = (type == BULLET_NORMAL) ? bulletData_Normal : bulletData_Fire;
    SetBullet(pos, rot, data, WEAPON_REVOLVER);
}
//=============================================================================
// ショットガン弾の発射関数（複数同時発射） //追加箇所
//=============================================================================
void SetShotgunBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot)
{
    const BulletData& data = (type == BULLET_NORMAL) ? bulletData_Normal : bulletData_Fire;

    for (int i = 0; i < 8; ++i)
    {
        // 弾ごとにランダムな角度のばらけを与える（前方円錐状）
        XMFLOAT3 randRot = rot;
        randRot.x += XMConvertToRadians((float)(rand() % 11 - 5));   // -5～5度の縦方向ばらけ
        randRot.y += XMConvertToRadians((float)(rand() % 21 - 10));  // -10～10度の横方向ばらけ

        SetBullet(pos, randRot, data, WEAPON_SHOTGUN);
    }
}

//=============================================================================
// ロケットランチャーの発射
//=============================================================================
void SetRocketLauncherBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot)
{
    const BulletData& data = (type == BULLET_NORMAL) ? bulletData_Normal : bulletData_Fire;
    SetBullet(pos, rot, data, WEAPON_ROCKET_LAUNCHER);
}

//=============================================================================
// 弾の更新
//=============================================================================
void UpdateBullet(void)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        const float rocketGravity = -0.1f;

        if (g_Bullet[i].use)
        {
            // 次の位置を計算
            XMFLOAT3 nextPos = {
                g_Bullet[i].pos.x + g_Bullet[i].vel.x,
                g_Bullet[i].pos.y + g_Bullet[i].vel.y,
                g_Bullet[i].pos.z + g_Bullet[i].vel.z
            };

            // 弾のAABB（半径はsizeの半分）
            float r = g_Bullet[i].size * 0.5f;
            XMFLOAT3 boxMin = { nextPos.x - r, nextPos.y - r, nextPos.z - r };
            XMFLOAT3 boxMax = { nextPos.x + r, nextPos.y + r, nextPos.z + r };

            // 壁当たり判定
            if (AABBHitOctree(GetWallTree(), GetWallTriangles(), boxMin, boxMax, 0, 5, 5))
            {
                if (g_Bullet[i].firedByWeapon == WEAPON_ROCKET_LAUNCHER) {
                    // 直前に計算した nextPos を着弾点として爆発
                    ApplyExplosionAt(nextPos);
                }
                g_Bullet[i].use = FALSE;
                continue; // この弾の処理終了
            }

            // 床当たり判定
            if (AABBHitOctree(GetFloorTree(), GetFloorTriangles(), boxMin, boxMax, 0, 5, 5))
            {
                if (g_Bullet[i].firedByWeapon == WEAPON_ROCKET_LAUNCHER) {
                    ApplyExplosionAt(nextPos);
                }
                g_Bullet[i].use = FALSE;
                continue;
            }

            // ロケットランチャーの弾だけ重力をかける
            if (g_Bullet[i].firedByWeapon == WEAPON_ROCKET_LAUNCHER)
            {
                g_Bullet[i].vel.y += rocketGravity;
            }

            // 位置更新
            g_Bullet[i].pos = nextPos;


            // 寿命処理
            g_Bullet[i].lifetime -= 1.0f;
            if (g_Bullet[i].lifetime <= 0)
            {
                g_Bullet[i].use = FALSE;

                if (g_Bullet[i].isLoaded)
                {
                    UnloadModel(&g_Bullet[i].model);
                    g_Bullet[i].isLoaded = FALSE;
                }
            }
        }
    }
}
//=============================================================================
// 弾の描画
//=============================================================================

void DrawBullet(void)
{
    SetCullingMode(CULL_MODE_NONE);
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (g_Bullet[i].use)
        {
            XMMATRIX mtxScl = XMMatrixScaling(g_Bullet[i].size, g_Bullet[i].size, g_Bullet[i].size);
            XMMATRIX mtxRot = XMMatrixRotationRollPitchYaw(g_Bullet[i].rot.x, g_Bullet[i].rot.y, g_Bullet[i].rot.z);
            XMMATRIX mtxTrans = XMMatrixTranslation(g_Bullet[i].pos.x, g_Bullet[i].pos.y, g_Bullet[i].pos.z);

            XMMATRIX mtxWorld = mtxScl * mtxRot * mtxTrans;
            SetWorldMatrix(&mtxWorld);

            //MATERIAL material = {};
            //material.Diffuse = XMFLOAT4(g_Bullet[i].color.x, g_Bullet[i].color.y, g_Bullet[i].color.z, 1.0f);
            //material.Ambient = material.Diffuse; 
            //material.noTexSampling = 1;
            //SetMaterial(material); 

            DrawModel(&g_Bullet[i].model);

        }
    }
}

//=============================================================================
// 弾の取得
//=============================================================================
BULLET* GetBullet(void)
{
    return g_Bullet;
}

//=============================================================================
// 武器の取得 //追加箇所
//=============================================================================
Weapon* GetRevolver()
{
    return &g_Revolver;
}

Weapon* GetShotgun()
{
    return &g_Shotgun;
}

Weapon* GetRocket_Launcher()
{
    return &g_RocketLauncher;
}