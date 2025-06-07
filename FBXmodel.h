#pragma once

#include <fbxsdk.h>
#include "model.h"

void Destroy(FbxManager** manager, FbxIOSettings** iosetting, FbxScene** scene, FbxImporter** importer);
const char* GetNodeAttributeName(FbxNodeAttribute::EType attribute);
void PrintNode(FbxNode* node, int hierarchy);


void ProcessNode(FbxNode* node);
void ProcessMesh(FbxMesh* mesh);


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
	DX11_MODEL			model;				// ���f�����

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

