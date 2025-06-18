//=============================================================================
//
// 弾発射処理 [bullet.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"
#include <DirectXMath.h>
#include <d3dx9math.h>

using namespace DirectX;

#define MAX_BULLET (100)

//=============================================================================
// 弾種別 //追加箇所
//=============================================================================
enum BulletType {
    BULLET_NORMAL,
    BULLET_FIRE
};

// 弾データ構造（属性付き）
struct BulletData {
    BulletType type;
    float speed;
    int damage;
    float size;
    float lifetime;
    const char* modelPath;

    //XMFLOAT3 color;  //弾のRGB
};

// 武器種別 
enum WeaponType {
    WEAPON_REVOLVER,
    WEAPON_SHOTGUN
};

// 武器構造体 
struct Weapon {
    WeaponType weaponType;
    BulletData* bulletData;
};

//=============================================================================
// 弾構造体
//=============================================================================
struct BULLET {
    bool use;
    XMFLOAT3 pos;
    XMFLOAT3 rot;
    float spd;
    float size;
    float lifetime; 
    DX11_MODEL model;
    XMFLOAT3 vel; // ← 速度ベクトルを追加
    float fWidth;
    float fHeight;
    XMMATRIX mtxWorld;

    //XMFLOAT3 color;  // ← 弾の色を保持するための変数

};

//=============================================================================
// 関数プロトタイプ宣言
//=============================================================================
HRESULT InitBullet(void);
void UninitBullet();
void UpdateBullet(void);
void DrawBullet(void);
int SetBullet(XMFLOAT3 pos, XMFLOAT3 rot, BulletData data);
int SetBulletWithData(const BulletData& data, XMFLOAT3 pos, XMFLOAT3 rot);
void SetRevolverBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot);
void SetShotgunBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot);

BULLET* GetBullet(void);
Weapon* GetRevolver(void);
Weapon* GetShotgun(void);
