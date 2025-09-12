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
BulletData bulletData_Normal = { BULLET_NORMAL,  40.0f, 10, 0.2f, 200.0f, "data/MODEL/NormalBullet.obj" };
BulletData bulletData_Fire = { BULLET_FIRE,     15.0f, 20, 0.6f, 200.0f, "data/MODEL/FireBullet.obj" };

// 武器インスタンス 
Weapon g_Revolver;
Weapon g_Shotgun;
Weapon g_RocketLauncher;

// 弾のインスタンス配列
BULLET g_Bullet[MAX_BULLET];

static std::vector<BULLET*> s_activeBullets;



int GetBulletCount() {
    return MAX_BULLET;
}

std::vector<BULLET*> GetActiveBullets() {
    std::vector<BULLET*> out;
    out.reserve(MAX_BULLET);
    for (int i = 0; i < MAX_BULLET; ++i) {
        if (g_Bullet[i].use) out.push_back(&g_Bullet[i]);
    }
    return out; 
}
//==========================================================================
// 爆風
//==========================================================================


namespace 
{
    // 爆発チューニング用パラメータ（必要に応じて調整）
    constexpr float kExplosionRadius = 200.0f;   // 爆風半径
    constexpr float kExplosionForce = 20.0f;   // 吹き飛ばし強さ
    constexpr float kUpwardBoost = 0.6f;   // 上向き成分の強さ（ちょっと浮かせる）

    inline float Length3(const XMFLOAT3& v) 
    {
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
        if (auto* enemy = dynamic_cast<BaseEnemy*>(obj)) {
            // 水平成分だけでノックバック方向を作る
            XMFLOAT3 dirXZ{ n.x, 0.0f, n.z };
            float lenXZ = sqrtf(dirXZ.x * dirXZ.x + dirXZ.z * dirXZ.z);
            if (lenXZ > 0.0001f) {
                dirXZ.x /= lenXZ;
                dirXZ.z /= lenXZ;
            }
            else {
                dirXZ = { 0.0f, 0.0f, 1.0f }; // 万一のゼロ除算回避
            }

            const float kbStrength = force * falloff * 7.0f; // ノックバック強さ
            const float kbDuration = 0.50f;                  // ノックバック継続時間

            enemy->ApplyKnockback(dirXZ, kbStrength, kbDuration);
            return; // 敵はここで処理終了（下の AddForce は使わない）
        }

        obj->AddForce(impulse);
        if (auto* item = dynamic_cast<ITEM_OBJ*>(obj)) {
            item->SetSleeping(false);
        }
    }
}

// ===== ロケット弾の軌道チューニング =====
constexpr float kRocketRefSpeed = 200.0f;   // “元のロケラン速度”の基準（あなたの初期値）
constexpr float kRocketBaseDrop = -5.00f;  // 基準速度のときに毎フレーム落下させる量
constexpr float kRocketDragXY = 0.03f;   // 水平ドラッグ（前進をわずかに減速）
constexpr bool  kRocketFaceVelocity = true;  // 見た目：速度方向に向けるか

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

static inline bool RaycastMap(
    const XMFLOAT3& start,
    const XMFLOAT3& step,  
    int lod,
    float& outHitDist,
    XMFLOAT3& outHitPos,
    XMFLOAT3& outHitNor)
{
    outHitDist = std::sqrt(step.x * step.x + step.y * step.y + step.z * step.z);
    if (outHitDist <= 1e-6f) return false;

    OctreeNode* wallTree = GetWallTree();
    OctreeNode* floorTree = GetFloorTree();
    const auto& wallTris = GetWallTriangles();
    const auto& floorTris = GetFloorTriangles();

    bool hit = false;
    float hitDistW = outHitDist, hitDistF = outHitDist;
    XMFLOAT3 hitPosW, hitNorW;
    XMFLOAT3 hitPosF, hitNorF;

    if (wallTree && !wallTris.empty()) {
        if (RayHitOctreeLOD(wallTree, wallTris, start, step,
            &hitDistW, &hitPosW, &hitNorW,
            0, 6, 1, lod)) {
            hit = true;
        }
    }

    if (floorTree && !floorTris.empty()) {
        if (RayHitOctreeLOD(floorTree, floorTris, start, step,
            &hitDistF, &hitPosF, &hitNorF,
            0, 6,1, lod)) {
            if (!hit || hitDistF < hitDistW) {
                hit = true;
            }
        }
    }

    if (!hit) return false;

    if (hitDistW <= hitDistF) {
        outHitDist = hitDistW; outHitPos = hitPosW; outHitNor = hitNorW;
    }
    else {
        outHitDist = hitDistF; outHitPos = hitPosF; outHitNor = hitNorF;
    }
    return true;
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
    constexpr float backEps = 0.1f;

    OctreeNode* wallTree = GetWallTree();
    OctreeNode* floorTree = GetFloorTree();
    const auto& wallTris = GetWallTriangles();
    const auto& floorTris = GetFloorTriangles();

    for (int i = 0; i < MAX_BULLET; i++)
    {
        BULLET& b = g_Bullet[i];
        if (!b.use) continue;

        // ロケットランチャーの弾だけ重力をかける
        if (g_Bullet[i].firedByWeapon == WEAPON_ROCKET_LAUNCHER)
        {
            // 速度に比例して落下を強める（速いほど強く落ちる）
            const float speed = std::sqrt(b.vel.x * b.vel.x + b.vel.y * b.vel.y + b.vel.z * b.vel.z);

            // 例）基準速度に対する比で落下量をスケール
            const float drop = kRocketBaseDrop * (speed / (kRocketRefSpeed + 1e-6f)); // kRocketBaseDrop は負の値
            b.vel.y += drop;  // 重力（強め）

            // 水平ドラッグで前進を少しずつ減速（横方向のみ）
            b.vel.x *= (1.0f - kRocketDragXY);       
            b.vel.z *= (1.0f - kRocketDragXY);

            // 見た目を速度方向に向ける（ロケットが進行方向を向く）
            if (kRocketFaceVelocity && speed > 1e-6f) {
                const float yaw = atan2f(b.vel.x, b.vel.z);
                const float pitch = atan2f(b.vel.y, std::sqrt(b.vel.x * b.vel.x + b.vel.z * b.vel.z));
                b.rot.y = yaw;
                b.rot.x = pitch;
            }
        }

        const XMFLOAT3 start = b.pos;
        const XMFLOAT3 step = b.vel;                
        const float    tmax = std::sqrt(step.x * step.x + step.y * step.y + step.z * step.z);


        //speed=0=dead

        if (tmax <= 1e-6f) {
            b.lifetime -= 1.0f;
            if (b.lifetime <= 0.0f) {
                b.use = FALSE;
                if (b.isLoaded) { UnloadModel(&b.model); b.isLoaded = FALSE; }
            }
            continue;
        }

        const float speedXZ = std::sqrt(b.vel.x * b.vel.x + b.vel.z * b.vel.z);
        int lod = 1;                 
        if (speedXZ > 6.0f) lod = 2; 
        if (speedXZ > 12.0f) lod = 3;


        float    hitDist;
        XMFLOAT3 hitPos, hitNor;
        const bool hit = RaycastMap(start, step, lod, hitDist, hitPos, hitNor);

        if (!hit) {
            b.pos.x += step.x;
            b.pos.y += step.y;
            b.pos.z += step.z;
        }
        else {
            const float invLen = 1.0f / tmax;
            const XMFLOAT3 dirN = { step.x * invLen, step.y * invLen, step.z * invLen };

            b.pos.x = hitPos.x - dirN.x * backEps;
            b.pos.y = hitPos.y - dirN.y * backEps;
            b.pos.z = hitPos.z - dirN.z * backEps;

            if (b.firedByWeapon == WEAPON_ROCKET_LAUNCHER) {
                ApplyExplosionAt(hitPos); 
            }

            b.use = FALSE;
            if (b.isLoaded) { UnloadModel(&b.model); b.isLoaded = FALSE; }
            continue;
        }


        b.lifetime -= 1.0f;
        if (b.lifetime <= 0.0f) {
            b.use = FALSE;
            if (b.isLoaded) { UnloadModel(&b.model); b.isLoaded = FALSE; }
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
//=================================================================
//
//==============================================================
void RebuildActiveBulletList() {
    s_activeBullets.clear();

    BULLET* bullets = GetBullet(); 
    for (int i = 0; i < MAX_BULLET; ++i) {
        if (bullets[i].use) {
            s_activeBullets.push_back(&bullets[i]);
        }
    }
}