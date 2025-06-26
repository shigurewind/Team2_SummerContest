//=============================================================================
//
// �e���ˏ��� [EnemyBullet.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"
#include <DirectXMath.h>

using namespace DirectX;

#define MAX_BULLET (100)

//=============================================================================
// �e��� //�ǉ��ӏ�
//=============================================================================
enum EnemyBulletType {
    BULLET_STRING
};

// �e�f�[�^�\���i�����t���j
struct EnemyBulletData {
    EnemyBulletType type;
    float speed;
    int damage;
    float size;
    float lifetime;
    const char* modelPath;

    //XMFLOAT3 color;  //�e��RGB
};

// ������ 
enum EnemyType {
    E_SPIDER,
};

// ����\���� 
struct Enemy {
    EnemyType enemyType;
    EnemyBulletData* bulletData;
};

//=============================================================================
// �e�\����
//=============================================================================
struct ENEMYBULLET {
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
HRESULT InitEnemyBullet(void);
void UninitEnemyBullet();
void UpdateEnemyBullet(void);
void DrawEnemyBullet(void);
int SetEnemyBullet(XMFLOAT3 pos, XMFLOAT3 rot, EnemyBulletData data);
ENEMYBULLET* GetEnemyBullet(void);

extern EnemyBulletData bulletData_String;
