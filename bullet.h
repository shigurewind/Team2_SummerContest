//=============================================================================
//
// �e���ˏ��� [bullet.h]
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
// �e��� //�ǉ��ӏ�
//=============================================================================
enum BulletType {
    BULLET_NORMAL,
    BULLET_FIRE
};

// �e�f�[�^�\���i�����t���j
struct BulletData {
    BulletType type;
    float speed;
    int damage;
    float size;
    float lifetime;
    const char* modelPath;

    //XMFLOAT3 color;  //�e��RGB
};

// ������ 
enum WeaponType {
    WEAPON_REVOLVER,
    WEAPON_SHOTGUN
};

// ����\���� 
struct Weapon {
    WeaponType weaponType;
    BulletData* bulletData;
};

//=============================================================================
// �e�\����
//=============================================================================
struct BULLET {
    bool use;
    XMFLOAT3 pos;
    XMFLOAT3 rot;
    float spd;
    float size;
    float lifetime; 
    DX11_MODEL model;
    XMFLOAT3 vel; // �� ���x�x�N�g����ǉ�
    float fWidth;
    float fHeight;
    XMMATRIX mtxWorld;

    //XMFLOAT3 color;  // �� �e�̐F��ێ����邽�߂̕ϐ�

};

//=============================================================================
// �֐��v���g�^�C�v�錾
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
