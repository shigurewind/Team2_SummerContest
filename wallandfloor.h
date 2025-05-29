#pragma once

#include "model.h"
//0522kabe
//shinnki

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_WALLANDFLOOR		(2)					// �G�l�~�[�̐�//0526floor

#define	WALLANDFLOOR_SIZE		(5.0f)				// �����蔻��̑傫��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct WALLANDFLOOR
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�


};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitWallAndFloor(void);
void UninitWallAndFloor(void);
void UpdateWallAndFloor(void);
void DrawWallAndFloor(void);

WALLANDFLOOR* GetWallAndFloor(void);

