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



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGameUI(void);
void UnInitGameUI(void);
void UpdateGameUI(void);
void DrawGameUI(void);


void DrawHPBar();
void DrawHP();


void DrawAmmoUI(void);

void ShowWebEffect(float time);
void ShowBugEffect(BugEnemy* enemy);
void HideBugEffect(BugEnemy* enemy);


void DrawItemSlot(void);


