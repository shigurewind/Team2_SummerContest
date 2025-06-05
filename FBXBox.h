#pragma once
#include "FbxMeshFile.h"

struct Box
{
	//XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	Vector3			pos;				// ���f���̈ʒu
	Vector3			rot;				// ���f���̌���(��])
	Vector3			scl;				// ���f���̑傫��(�X�P�[��)

	FbxMeshFile* model;


};


HRESULT InitBox(void);
void UninitBox(void);
void UpdateBox(void);
void DrawBox(void);

Box* GetBox(void);