//=============================================================================
//
// �G�l�~�[���f������ [enemy.h]
// Author : 
//
//=============================================================================
#pragma once
#include "shadow.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_ENEMY		(5)					// �G�l�~�[�̐�

#define	ENEMY_SIZE		(5.0f)				// �����蔻��̑傫��

#define	MODEL_ENEMY			"data/MODEL/enemy.obj"		// �ǂݍ��ރ��f����
#define ENEMY_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��
#define ENEMY_SHADOW_SIZE	(0.4f)						// �e�̑傫��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct ENEMY 
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)
	XMFLOAT3			dir;				// �����̌���
	int					type;


	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

	float				time;				// ���`��ԗp
	int					tblNo;				// �s���f�[�^�̃e�[�u���ԍ�
	int					tblMax;				// ���̃e�[�u���̃f�[�^��

	int					moveCounter;		// �����ς��^�C�}�[
};

enum ENEMY_TYPE
{
	GHOST = 0,
	SKELETON,
	SPIDER,
	NONE
};



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY *GetEnemy(void);

void ChangeEnemyDirection(int i);
void ChasingPlayer(int i);
void GhostMovement(int i);
void SkeletonMovement(int i);

