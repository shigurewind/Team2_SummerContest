//=============================================================================
//
// �Q�[����ʏ��� [game.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "game1.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"
#include "overlay2D.h"

#include "player.h"
#include "enemy.h"
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "tree.h"
#include "bullet.h"
#include "GameUI.h"
#include "particle.h"
#include "collision.h"
#include "debugproc.h"

#include "FBXmodel.h"
#include "item.h"
#include "dissolveTest.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

BOOL	g_bPause1 = FALSE;	// �|�[�YON/OFF


//=============================================================================
// ����������
//=============================================================================
HRESULT InitGame1(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// �t�B�[���h�̏�����
	//InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 100, 100, 13.0f, 13.0f);

	// ���C�g��L����	// �e�̏���������
	InitShadow();

	// �v���C���[�̏�����
	InitPlayer();

	//// �G�l�~�[�̏�����
	InitEnemy();

	

	

	// �e�̏�����
	InitBullet();

	// �X�R�A�̏�����
	InitScore();

	InitOverlay2D();
	// �p�[�e�B�N���̏�����
	InitParticle();

	InitFBXTestModel();

	InitItem();

	InitDissolveTest();

	// BGM�Đ�
	PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitGame1(void)
{
	// �p�[�e�B�N���̏I������
	UninitParticle();

	// �X�R�A�̏I������
	UninitScore();
	UninitOverlay2D();
	// �e�̏I������
	UninitBullet();

	

	// �n�ʂ̏I������
	//UninitMeshField();

	//// �G�l�~�[�̏I������
	UninitEnemy();

	// �v���C���[�̏I������
	UninitPlayer();

	// �e�̏I������
	UninitShadow();

	UninitFBXTestModel();

	UninitItem();

	UninitDissolveTest();

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateGame1(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	// ���Ԃ��~�߂�
	if (GetKeyboardTrigger(DIK_P))
	{
		g_bPause1 = g_bPause1 ? FALSE : TRUE;
	}


#endif

	if (GetFade() == FADE_OUT) {
		return;
	}

	if (g_bPause1 == TRUE)
		return;
	
	if (IsTutorialShowing())
	{
		if (IsMouseLeftTriggered())
		{
			SetTutorialShowing(false); 
		}
		return;  
	}
	// �n�ʏ����̍X�V
	//UpdateMeshField();
	UpdateFBXTestModel();
	// �v���C���[�̍X�V����
	UpdatePlayer();

	//// �G�l�~�[�̍X�V����
	UpdateEnemy();


	// �e�̍X�V����
	UpdateBullet();

	// �p�[�e�B�N���̍X�V����
	//UpdateParticle();

	// �e�̍X�V����
	UpdateShadow();

	// �����蔻�菈��
	UpdateOverlay2D();
	// �X�R�A�̍X�V����
	UpdateScore();

	//UpdateFBXTestModel();

	UpdateItem();

	UpdateDissolveTest();
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawGame01(void)
{
	if (GetFade() == FADE_OUT) {
		return;
	}

	// 3D�̕���`�悷�鏈��
	// �n�ʂ̕`�揈��
	//DrawMeshField();

	// �e�̕`�揈��
	DrawShadow();

	//// �G�l�~�[�̕`�揈��
	DrawEnemy();

	// �v���C���[�̕`�揈��
	DrawPlayer();

	// �e�̕`�揈��
	DrawBullet();

	
	// �p�[�e�B�N���̕`�揈��
	DrawParticle();

	DrawFBXTestModel();

	DrawItem();

	DrawDissolveTest();


	// 2D�̕���`�悷�鏈��
	// Z��r�Ȃ�
	SetDepthEnable(FALSE);

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);

	// �X�R�A�̕`�揈��
	DrawScore();
	DrawOverlay2D();
	


	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// Z��r����
	SetDepthEnable(TRUE);
}


void DrawGame1(void)
{
	XMFLOAT3 pos;

	if (GetFade() == FADE_OUT) {
		return;
	}

#ifdef _DEBUG
	// �f�o�b�O�\��
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif

	// �v���C���[���_
	//pos = GetPlayer()->pos;
	//pos.y = 0.0f;			// �J����������h�����߂ɃN���A���Ă���
	//SetCameraAT(pos);
	SetCamera();

	switch (g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame01();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame01();

		// �G�l�~�[���_
		//pos = GetEnemy()->pos;
		//pos.y = 0.0f;
		//SetCameraAT(pos);
		//SetCamera();
		//SetViewPort(TYPE_RIGHT_HALF_SCREEN);
		//DrawGame0();
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawGame01();

		// �G�l�~�[���_
		//pos = GetEnemy()->pos;
		//pos.y = 0.0f;
		//SetCameraAT(pos);
		//SetCamera();
		//SetViewPort(TYPE_DOWN_HALF_SCREEN);
		//DrawGame0();
		break;

	}

}


