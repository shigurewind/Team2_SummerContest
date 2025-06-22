#pragma once

#include <fbxsdk.h>
#include "model.h"
#include "DirectXMath.h"
#include "Octree.h"




//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct FBXTESTMODEL
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	float				spd;				// �ړ��X�s�[�h

	BOOL				load;
	AMODEL*				model;				// FBX���f�����

	SHADER*				shader;				//�g��Shader

	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

	BOOL				alive;

	float				size;

	// �K�w�A�j���[�V�����p�̃����o�[�ϐ�
	float				time;				// ���`��ԗp
	int					tblNo;				// �s���f�[�^�̃e�[�u���ԍ�
	int					tblMax;				// ���̃e�[�u���̃f�[�^��


	// �N�H�[�^�j�I��
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// �����������Ă��鏊


};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitFBXTestModel(void);
void UninitFBXTestModel(void);
void UpdateFBXTestModel(void);
void DrawFBXTestModel(void);

FBXTESTMODEL* GetFBXTestModel(void);

void ExtractTriangleData(AMODEL* model, const XMMATRIX& worldMatrix);
const std::vector<TriangleData>& GetTriangleList();

OctreeNode* GetWallTree();
OctreeNode* GetFloorTree();