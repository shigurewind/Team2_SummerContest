#include "FBXmodel.h"



//-------------------------------------------------------------------------

static FBXTESTMODEL g_FBXTestModel;	// FBX���f���̃f�[�^

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;
	
	//g_FBXTestModel.model = ModelLoad("data/MODEL/model.fbx");	// FBX���f���̓ǂݍ���
	g_FBXTestModel.model = ModelLoad("data/MODEL/rockkk.fbx");	// FBX���f���̓ǂݍ���

	


	g_FBXTestModel.pos = XMFLOAT3(-10.0f, 20.0f, -50.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(10.0f, 10.0f, 10.0f);

	g_FBXTestModel.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_FBXTestModel.alive = TRUE;			// TRUE:�����Ă�


	return S_OK;
}

void UninitFBXTestModel(void)
{
	// ���f���̉������
	if (g_FBXTestModel.load == TRUE)
	{
		ModelRelease(g_FBXTestModel.model);	// FBX���f���̉��
		g_FBXTestModel.load = FALSE;
	}

}

void UpdateFBXTestModel(void)
{
	//g_FBXTestModel.rot.y += 0.01f;	// ��]�����Ă݂�
	//g_FBXTestModel.rot.x += 0.01f;
	//g_FBXTestModel.pos.x +=  0.1f;	// X�������Ɉړ�
}

void DrawFBXTestModel(void)
{
	

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_FBXTestModel.scl.x, g_FBXTestModel.scl.y, g_FBXTestModel.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_FBXTestModel.rot.x, g_FBXTestModel.rot.y + XM_PI, g_FBXTestModel.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �N�H�[�^�j�I���𔽉f
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_FBXTestModel.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_FBXTestModel.pos.x, g_FBXTestModel.pos.y, g_FBXTestModel.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_FBXTestModel.mtxWorld, mtxWorld);


	// �����̐ݒ�
	//SetFuchi(1);

	// ���f���`��
	ModelDraw(g_FBXTestModel.model);


	//SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
FBXTESTMODEL* GetFBXTestModel(void)
{
	return &g_FBXTestModel;
}






