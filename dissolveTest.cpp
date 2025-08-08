#include "dissolveTest.h"
#include "main.h"
#include "camera.h"
#include "shaderManager.h"

#define TEXTURE_MAX (2)

#define TEST_WIDTH (40.0f)	
#define TEST_HEIGHT (40.0f)	
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexDissolve(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static DissolveTest g_dissolveTest;

static BOOL					g_bAlpaTest;

static ID3D11Buffer* g_VertexBuffer = NULL;	// ���_�o�b�t�@
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TextureName[] =
{
	"data/TEXTURE/tree001.png",// �e�N�X�`��1
	"data/TEXTURE/sampleNoise.png",// noise�e�N�X�`��

	
};



HRESULT InitDissolveTest()
{
	MakeVertexDissolve();

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	ZeroMemory(&g_dissolveTest.material, sizeof(g_dissolveTest.material));
	g_dissolveTest.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	g_dissolveTest.pos = XMFLOAT3(10.0f, 0.0f, 10.0f);
	g_dissolveTest.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_dissolveTest.dissolve = 0.0f;	// �n��
	g_dissolveTest.dissolveSpeed = 0.01f;	// �n�𑬓x
	g_dissolveTest.dissolveTime = 0.0f;	// �n������
	g_dissolveTest.isDissolving = FALSE;	// �n�𒆃t���O



	g_bAlpaTest = TRUE;	// ���e�X�g��L���ɂ���


	return S_OK;
}


void UninitDissolveTest()
{
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{// �e�N�X�`���̉��
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if (g_VertexBuffer != NULL)
	{// ���_�o�b�t�@�̉��
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}


void UpdateDissolveTest()
{
	if (g_dissolveTest.isDissolving)
	{
		g_dissolveTest.dissolve += g_dissolveTest.dissolveSpeed;
		if (g_dissolveTest.dissolve > 1.0f)
			g_dissolveTest.dissolve = 1.0f;
	}
	else 
	{
		g_dissolveTest.dissolve = 0.0f;
	}

	
	
}


void DrawDissolveTest()
{
	if (g_bAlpaTest == TRUE)
	{
		// ���e�X�g��L����
		SetAlphaTestEnable(TRUE);
	}

	SetLightEnable(FALSE);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA* cam = GetCamera();

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �r���[�}�g���b�N�X���擾
	mtxView = XMLoadFloat4x4(&cam->mtxView);

	// �����s��i�����s��j��]�u�s�񂳂��ċt�s�������Ă��(����)
	mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];


	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_dissolveTest.scl.x, g_dissolveTest.scl.y, g_dissolveTest.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_dissolveTest.pos.x, g_dissolveTest.pos.y, g_dissolveTest.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	SetMaterial(g_dissolveTest.material);

	//�f�B�]���u
	//SetDissolveValue(g_dissolveTest.dissolve, g_dissolveTest.material.Diffuse);
	EffectManager::SetDissolveEffect(g_dissolveTest.dissolve, XMFLOAT4(1.0f, 0.2f, 0.0f, 1.0f));
	
	//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
	ID3D11ShaderResourceView* textures[] = { g_Texture[0], g_Texture[1] };
	GetDeviceContext()->PSSetShaderResources(0, 2, textures);

	GetDeviceContext()->Draw(4, 0);

	//�f�t�H���g�f�B�]���u�֎~
	//SetDissolveValue(-1.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	EffectManager::SetDissolveEffect(-1.0f, XMFLOAT4(1.0f, 0.2f, 0.0f, 1.0f));
	
}

void StartDissolve()
{

}


HRESULT MakeVertexDissolve(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = TEST_WIDTH;
	float fHeight = TEST_HEIGHT;

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// �@���̐ݒ�
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}


DissolveTest* GetDissolveTest()
{
	return &g_dissolveTest;
}