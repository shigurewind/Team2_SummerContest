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
#include <math.h>
#include <vector>


//=============================================================================
// 弾の基本データ構造（属性など） //追加箇所
//=============================================================================
//                                  種類　　　　速さ  DMG  scl  lifetime    　　モデル　　　　　　　　RGB
BulletData bulletData_Normal = { BULLET_NORMAL,  15.0f, 10, 0.2f, 200.0f, "data/MODEL/NormalBullet.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/};
BulletData bulletData_Fire   = { BULLET_FIRE,     5.0f, 20, 1.0f, 200.0f, "data/MODEL/FireBullet.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/};

// 武器インスタンス 
Weapon g_Revolver;
Weapon g_Shotgun;

// 弾のインスタンス配列
BULLET g_Bullet[MAX_BULLET];

//=============================================================================
// 初期化
//=============================================================================
HRESULT InitBullet(void)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        g_Bullet[i].use = FALSE;
    }

    // 武器ごとの弾をセット 
    g_Revolver.weaponType = WEAPON_REVOLVER;
    g_Revolver.bulletData = &bulletData_Normal;
    g_Revolver.clipSize   = 5;      //リロードできる弾数

    g_Shotgun.weaponType = WEAPON_SHOTGUN;
    g_Shotgun.bulletData = &bulletData_Normal;
    g_Shotgun.clipSize   = 3;       //リロードできる弾数

    return S_OK;
}


//=====================================================
//
//=====================================================
void UninitBullet()
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        UnloadModel(&g_Bullet[i].model);
    }
}

//=============================================================================
// 弾の発射（共通）
//=============================================================================
int SetBullet(XMFLOAT3 pos, XMFLOAT3 rot, BulletData data)
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
            g_Bullet[i].fWidth = 1.0f;
            g_Bullet[i].fHeight = 1.0f;
            g_Bullet[i].lifetime = data.lifetime;

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
int SetBulletWithData(const BulletData& data, XMFLOAT3 pos, XMFLOAT3 rot)
{
    return SetBullet(pos, rot, data);
}

//=============================================================================
// リボルバー弾の発射関数（分かりやすさのため） //追加箇所
//=============================================================================
void SetRevolverBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot)
{
    const BulletData& data = (type == BULLET_NORMAL) ? bulletData_Normal : bulletData_Fire;
    SetBullet(pos, rot, data);
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

        SetBullet(pos, randRot, data);
    }
}
//=============================================================================
// 弾の更新
//=============================================================================
void UpdateBullet(void)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (g_Bullet[i].use)
        {
            g_Bullet[i].pos.x += g_Bullet[i].vel.x;
            g_Bullet[i].pos.y += g_Bullet[i].vel.y;
            g_Bullet[i].pos.z += g_Bullet[i].vel.z;

            g_Bullet[i].lifetime -= 1.0f;
            if (g_Bullet[i].lifetime <= 0)
            {
                g_Bullet[i].use = FALSE;
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
