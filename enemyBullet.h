//=============================================================================
//
// 弾発射処理 [EnemyBullet.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"
#include <DirectXMath.h>

using namespace DirectX;

#define MAX_BULLET (100)

//=============================================================================
// 弾種別 //追加箇所
//=============================================================================
enum EnemyBulletType {
    BULLET_STRING
};

// 弾データ構造（属性付き）
struct EnemyBulletData {
    EnemyBulletType type;
    float speed;
    int damage;
    float size;
    float lifetime;
    const char* modelPath;

    //XMFLOAT3 color;  //弾のRGB
};

// 武器種別 
enum EnemyType {
    E_SPIDER,
};

// 武器構造体 
struct Enemy {
    EnemyType enemyType;
    EnemyBulletData* bulletData;
};

//=============================================================================
// 弾構造体
//=============================================================================
struct ENEMYBULLET {
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
HRESULT InitEnemyBullet(void);
void UninitEnemyBullet();
void UpdateEnemyBullet(void);
void DrawEnemyBullet(void);
int SetEnemyBullet(XMFLOAT3 pos, XMFLOAT3 rot, EnemyBulletData data);
ENEMYBULLET* GetEnemyBullet(void);

extern EnemyBulletData bulletData_String;
