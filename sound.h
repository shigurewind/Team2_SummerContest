//=============================================================================
//
// �T�E���h���� [sound.h]
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// �T�E���h�����ŕK�v

//*****************************************************************************
// �T�E���h�t�@�C��
//*****************************************************************************
enum
{
	//BGM
	SOUND_LABEL_BGM_sample000,	// �^�C�g����BGM
	SOUND_LABEL_BGM_sample001,	// in-game��BGM
	SOUND_LABEL_BGM_sample002,	// �Q�[���I�[�o�[��BGM
	//�v���C���[
	SOUND_LABEL_SE_bomb000,		// �e���ˉ�
	SOUND_LABEL_SE_defend000,	// �e���ˉ�
	SOUND_LABEL_SE_defend001,	// �e���ˉ�
	SOUND_LABEL_SE_hit000,		// �e���ˉ�
	SOUND_LABEL_SE_laser000,	// ����g�ݗ��Ď��̉�
	SOUND_LABEL_SE_lockon000,	// ����g�ݗ��Ď��̉�
	SOUND_LABEL_SE_shot000,		// 
	SOUND_LABEL_SE_shot001,		// �q�b�g��
	//�G�l�~�[
	SOUND_LABEL_SE_spiderEnemyMoving,	//�N���̃^�C�v�̃G�l�~�[�̓����̉�
	SOUND_LABEL_SE_spiderEnemyBite,		//�N���̃^�C�v�̃G�l�~�[�̊��މ�
	SOUND_LABEL_SE_spiderEnemySpit		//�N���̃^�C�v�̃G�l�~�[�̔��˂̉�
	,
	SOUND_LABEL_MAX,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
BOOL InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

