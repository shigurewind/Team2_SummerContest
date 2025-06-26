//=============================================================================
//
// �e���ˏ��� [enemyBullet.cpp]
// Author : 
//
//=============================================================================
#include "debugproc.h"

#include "main.h"
#include "renderer.h"
#include "shadow.h"
#include "enemyBullet.h"
#include "camera.h"
#include "player.h"
#include <math.h>
#include <vector>


//=============================================================================
// �e�̊�{�f�[�^�\���i�����Ȃǁj //�ǉ��ӏ�
//=============================================================================
//                                  ��ށ@�@�@�@����  DMG  scl  lifetime    �@�@���f���@�@�@�@�@�@�@�@RGB
EnemyBulletData bulletData_String = { BULLET_STRING,  15.0f, 10, 0.2f, 200.0f, "data/MODEL/kumoattack.obj", /*XMFLOAT3(1.0f, 0.0f, 0.0f)*/};

// ����C���X�^���X 
Enemy e_String;

// �e�̃C���X�^���X�z��
ENEMYBULLET g_Bullet[MAX_BULLET];

//=============================================================================
// ������
//=============================================================================
HRESULT InitEnemyBullet(void)
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        g_Bullet[i].use = FALSE;
    }

    // ���킲�Ƃ̒e���Z�b�g //�ǉ��ӏ�
    e_String.enemyType = E_SPIDER;
    e_String.bulletData = &bulletData_String;


    return S_OK;
}


//=====================================================
//
//=====================================================
void UninitEnemyBullet()
{
    for (int i = 0; i < MAX_BULLET; i++)
    {
        UnloadModel(&g_Bullet[i].model);
    }
}

//=============================================================================
// �e�̔��ˁi���ʁj
//=============================================================================
int SetEnemyBullet(XMFLOAT3 pos, XMFLOAT3 rot, EnemyBulletData data)
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


//=============================================================================
// �e�̍X�V
//=============================================================================
void UpdateEnemyBullet(void)
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

void DrawEnemyBullet(void)
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
ENEMYBULLET* GetEnemyBullet(void)
{
    return g_Bullet;
}

