//=============================================================================
//
// スコア処理 [score.h]Update
// Author : 
//
//=============================================================================
#pragma once
#include "enemy.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************

extern BOOL g_BugEffectActive;
extern float bugEffectTimer;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGameUI(void);
void UnInitGameUI(void);
void UpdateGameUI(void);
void DrawGameUI(void);


void DrawHPBar();
void DrawHP();
void DrawShootingHand();


void DrawAmmoUI(void);

void ShowWebEffect(float time);
void ShowBugEffect();
void HideBugEffect();


void DrawItemSlot(void);
void AddUIRecoil();

void DrawPaused(void);


