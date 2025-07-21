//=============================================================================
//
// �X�R�A���� [score.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "GameUI.h"
#include "sprite.h"
#include "player.h"
#include "bullet.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(16)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(32)	// 
#define TEXTURE_MAX					(6)		// �e�N�X�`���̐�


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/number16x32.png",
	"data/TEXTURE/HP00.png",
	"data/TEXTURE/HP01.png",
	"data/TEXTURE/revolver.png",
	"data/TEXTURE/shotgun.png",
	"data/2Dpicture/enemy/enemyWeb.png",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static int						g_Score;					// �X�R�A

static BOOL						g_Load = FALSE;

int Min2(int a, int b) {
	return (a < b) ? a : b;
}

static float g_WebEffectTimer = 0.0f;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitScore(void)
{
	ID3D11Device* pDevice = GetDevice();

	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �v���C���[�̏�����
	g_Use = TRUE;
	g_w = TEXTURE_WIDTH;
	g_h = TEXTURE_HEIGHT;
	g_Pos = { 500.0f, 20.0f, 0.0f };
	g_TexNo = 0;

	g_Score = 0;	// �X�R�A�̏�����

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitScore(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateScore(void)
{
	if (g_WebEffectTimer > 0.0f)
	{
		g_WebEffectTimer -= 0.05f / 60.0f;
		if (g_WebEffectTimer < 0.0f) g_WebEffectTimer = 0.0f;
	}


#ifdef _DEBUG	// �f�o�b�O����\������
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);

#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawScore(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// �e�N�X�`���ݒ�
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);





	//�P�[�W��HP�o�[
	{// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
		//�Q�[�W�̈ʒu��e�N�X�`���[���W�𔽉f
		float pw = 280;		// �Q�[�W�̕\����
		pw = pw * ((float)GetPlayer()->HP / GetPlayer()->HP_MAX);
		float x = ((float)GetPlayer()->HP / GetPlayer()->HP_MAX);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteLeftTop(g_VertexBuffer, 2.0f, 6.0f, pw, 60, 0.0f, 0.0f, x, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	//HP��UI
	{// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, 130.0f, 30.0f, 400, 180, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	//�N���̍U���̃G�t�F�N�g
	if (g_WebEffectTimer > 0.0f)
	{
		MATERIAL m = {};
		m.Diffuse = XMFLOAT4(1, 1, 1, 1);
		SetMaterial(m);

		SetWorldViewProjection2D();
		SetAlphaTestEnable(FALSE);
		SetBlendState(BLEND_MODE_ALPHABLEND);
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[5]);

		float alpha = g_WebEffectTimer; // 1.0 -> 0.0
		SetSpriteColor(g_VertexBuffer, 640.0f, 360.0f, 1277.0f, 770.0f, 0, 0, 1, 1, XMFLOAT4(1, 1, 1, alpha));

		GetDeviceContext()->Draw(4, 0);
	}


	//�e���\���̌Ăяo��
	DrawAmmoUI();

}

//========================================================
// ����ƒe��UI�\��
//========================================================
void DrawAmmoUI(void)
{

	Weapon* weapon = (GetCurrentWeaponType() == WEAPON_REVOLVER) ? GetRevolver() : GetShotgun();

	// === ����A�C�R���\�� ===
	const float weaponIconX = 1025.0f;  //�\���ʒu
	const float weaponIconY = 610.0f;

	int weaponTexNo = (GetCurrentWeaponType() == WEAPON_REVOLVER) ? 3 : 4;
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[weaponTexNo]);

	SetSprite(g_VertexBuffer,
		weaponIconX, weaponIconY,
		60, 60,  // �T�C�Y
		0.0f, 0.0f, 1.0f, 1.0f
	);
	GetDeviceContext()->Draw(4, 0);

	// === �e���\�� === 
	int clipSize = weapon->clipSize;

	int ammoInClip = 0;
	int ammoSpare = 0;

	if (GetCurrentBulletType() == BULLET_NORMAL) {
		ammoInClip = Min2(GetPlayer()->ammoNormal, clipSize);
		ammoSpare = GetPlayer()->maxAmmoNormal;
	}
	else {
		ammoInClip = Min2(GetPlayer()->ammoFire, clipSize);
		ammoSpare = GetPlayer()->maxAmmoFire;
	}

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));

	if (GetCurrentBulletType() == BULLET_FIRE) {
		material.Diffuse = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);  //  ��
	}
	else {
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  //  ���i�m�[�}���e�j
	}

	SetMaterial(material);

	// �����X�v���C�g�ݒ�i0?9��1��ɕ���ł���j
	const float digitWidth = 16.0f;
	const float digitHeight = 32.0f;

	const float baseX = 1000.0f;  // �\���ʒu�i�E���ɒ����j
	const float baseY = 650.0f;

	char text[16];
	sprintf(text, "%d/%d", ammoInClip, ammoSpare);

	// ������1�������`��
	for (int i = 0; text[i] != '\0'; ++i) {
		char c = text[i];
		if (c == '/') {
			continue;  // �X���b�V���͍��͕\�����Ȃ��i�K�v�Ȃ�ʓr�e�N�X�`���p�Ӂj
		}

		int n = c - '0';
		if (n < 0 || n > 9) continue;

		float u = (n % 10) / 10.0f;
		float v = 0.0f;
		float uw = 1.0f / 10.0f;
		float vh = 1.0f;

		SetSpriteLeftTop(
			g_VertexBuffer,
			baseX + i * digitWidth, baseY,
			digitWidth, digitHeight,
			u, v, uw, vh
		);

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
		GetDeviceContext()->Draw(4, 0);
	}
}


//=============================================================================
// �X�R�A�����Z����
// ����:add :�ǉ�����_���B�}�C�i�X���\
//=============================================================================
void AddScore(int add)
{
	g_Score += add;
	if (g_Score > SCORE_MAX)
	{
		g_Score = SCORE_MAX;
	}

}


int GetScore(void)
{
	return g_Score;
}

//=============================================================================
// �w偂̃l�b�g���ʁi��ʂɕ\���j����莞�Ԍ�����֐�
//=============================================================================
void ShowWebEffect(float time)
{
	g_WebEffectTimer = time; // time �b�ԁA��ʂɒw偂̃l�b�g��\��
}
