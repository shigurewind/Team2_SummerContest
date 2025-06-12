//=============================================================================
//
// サウンド処理 [sound.h]
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// サウンド処理で必要

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum
{
	//BGM
	SOUND_LABEL_BGM_sample000,	// タイトルのBGM
	SOUND_LABEL_BGM_sample001,	// in-gameのBGM
	SOUND_LABEL_BGM_sample002,	// ゲームオーバーのBGM
	//プレイヤー
	SOUND_LABEL_SE_bomb000,		// 弾発射音
	SOUND_LABEL_SE_defend000,	// 弾発射音
	SOUND_LABEL_SE_defend001,	// 弾発射音
	SOUND_LABEL_SE_hit000,		// 弾発射音
	SOUND_LABEL_SE_laser000,	// 武器組み立て時の音
	SOUND_LABEL_SE_lockon000,	// 武器組み立て時の音
	SOUND_LABEL_SE_shot000,		// 
	SOUND_LABEL_SE_shot001,		// ヒット音
	//エネミー
	SOUND_LABEL_SE_spiderEnemyMoving,	//クモのタイプのエネミーの動きの音
	SOUND_LABEL_SE_spiderEnemyBite,		//クモのタイプのエネミーの噛む音
	SOUND_LABEL_SE_spiderEnemySpit		//クモのタイプのエネミーの発射の音
	,
	SOUND_LABEL_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
BOOL InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

