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
	SOUND_LABEL_SE_shot001,		// 弾発射音
	SOUND_LABEL_SE_shot002,	// 弾発射音
	SOUND_LABEL_SE_shot003,	// 弾発射音
	SOUND_LABEL_SE_shot004,		// 弾発射音
	SOUND_LABEL_SE_laser000,	// 武器組み立て時の音
	SOUND_LABEL_SE_lockon000,	// 武器組み立て時の音
	SOUND_LABEL_SE_shot0,		// 
	SOUND_LABEL_SE_shot1,		// ヒット音
	SOUND_LABEL_SE_punch,		// パンチ音
	SOUND_LABEL_SE_walk,		// 歩く音
	SOUND_LABEL_SE_walk2,		// 歩く音
	//システム
	SOUND_LABEL_SE_changeGun,	//銃変わり
	SOUND_LABEL_SE_pickItem,	//アイテムを拾う
	//エネミー
	SOUND_LABEL_SE_spiderEnemyMoving,	//クモのタイプのエネミーの動きの音
	SOUND_LABEL_SE_spiderEnemyBite,		//クモのタイプのエネミーの噛む音
	SOUND_LABEL_SE_spiderEnemySpit,		//クモのタイプのエネミーの発射の音
	SOUND_LABEL_SE_spiderEnemyDead		//クモのタイプのエネミーのの音
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

