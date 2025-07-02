//=============================================================================
//
// �e���ˏ��� [bullet.cpp]
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
// �e�̊�{�f�[�^�\���i�����Ȃǁj //�ǉ��ӏ�
//=============================================================================
//                                  ��ށ@�@�@�@����  DMG  scl  lifetime    �@�@���f���@�@�@�@�@�@�@�@RGB
BulletData bulletData_Normal = { BULLET_NORMAL,  15.0f, 10, 0.2f, 200.0f, "data/MODEL/NormalBullet.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/};
BulletData bulletData_Fire   = { BULLET_FIRE,     5.0f, 20, 1.0f, 200.0f, "data/MODEL/FireBullet.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/};

// ����C���X�^���X 
Weapon g_Revolver;
Weapon g_Shotgun;

// �e�̃C���X�^���X�z��
BULLET g_Bullet[MAX_BULLET];

//=============================================================================
// ������
//=============================================================================
HRESULT InitBullet(void)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        g_Bullet[i].use = FALSE;
    }

    // ���킲�Ƃ̒e���Z�b�g 
    g_Revolver.weaponType = WEAPON_REVOLVER;
    g_Revolver.bulletData = &bulletData_Normal;
    g_Revolver.clipSize   = 5;      //�����[�h�ł���e��

    g_Shotgun.weaponType = WEAPON_SHOTGUN;
    g_Shotgun.bulletData = &bulletData_Normal;
    g_Shotgun.clipSize   = 3;       //�����[�h�ł���e��

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
// �e�̔��ˁi���ʁj
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

//�e�̏��idata�j�����Ƃɒe�𔭎˂���֐�//
int SetBulletWithData(const BulletData& data, XMFLOAT3 pos, XMFLOAT3 rot)
{
    return SetBullet(pos, rot, data);
}

//=============================================================================
// ���{���o�[�e�̔��ˊ֐��i������₷���̂��߁j //�ǉ��ӏ�
//=============================================================================
void SetRevolverBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot)
{
    const BulletData& data = (type == BULLET_NORMAL) ? bulletData_Normal : bulletData_Fire;
    SetBullet(pos, rot, data);
}
//=============================================================================
// �V���b�g�K���e�̔��ˊ֐��i�����������ˁj //�ǉ��ӏ�
//=============================================================================
void SetShotgunBullet(BulletType type, XMFLOAT3 pos, XMFLOAT3 rot)
{
    const BulletData& data = (type == BULLET_NORMAL) ? bulletData_Normal : bulletData_Fire;

    for (int i = 0; i < 8; ++i)
    {
        // �e���ƂɃ����_���Ȋp�x�̂΂炯��^����i�O���~����j
        XMFLOAT3 randRot = rot;
        randRot.x += XMConvertToRadians((float)(rand() % 11 - 5));   // -5�`5�x�̏c�����΂炯
        randRot.y += XMConvertToRadians((float)(rand() % 21 - 10));  // -10�`10�x�̉������΂炯

        SetBullet(pos, randRot, data);
    }
}
//=============================================================================
// �e�̍X�V
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
// �e�̕`��
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
// �e�̎擾
//=============================================================================
BULLET* GetBullet(void)
{
    return g_Bullet;
}

//=============================================================================
// ����̎擾 //�ǉ��ӏ�
//=============================================================================
Weapon* GetRevolver()
{
    return &g_Revolver;
}

Weapon* GetShotgun()
{
    return &g_Shotgun;
}
