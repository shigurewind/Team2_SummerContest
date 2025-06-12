#include "FBXmodel.h"



//-------------------------------------------------------------------------

static FBXTESTMODEL g_FBXTestModel;	// FBX���f���̃f�[�^

static SHADER g_shaderCustom;

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;

	//g_FBXTestModel.model = ModelLoad("data/MODEL/model.fbx");	// FBX���f���̓ǂݍ���
	g_FBXTestModel.model = ModelLoad("data/MODEL/stage1.fbx");	// FBX���f���̓ǂݍ���

	LoadShaderFromFile("testShader.hlsl", "VertexShaderPolygon", "PixelShaderPolygon", &g_shaderCustom);
	g_FBXTestModel.shader = &g_shaderCustom;


	g_FBXTestModel.pos = XMFLOAT3(-10.0f, 20.0f, -50.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(15.0f, 15.0f, 15.0f);

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

	// �V�F�[�_�[�̐ݒ�
	ID3D11DeviceContext* context = GetDeviceContext();
	if (g_FBXTestModel.shader)
	{
		context->IASetInputLayout(g_FBXTestModel.shader->inputLayout);
		context->VSSetShader(g_FBXTestModel.shader->vertexShader, nullptr, 0);
		context->PSSetShader(g_FBXTestModel.shader->pixelShader, nullptr, 0);
	}

	// ���f���`��
	ModelDraw(g_FBXTestModel.model);

	//�f�t�H���gShader�ɖ߂�
	SetDefaultShader();


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






