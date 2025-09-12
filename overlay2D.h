//=============================================================================
//
// スコア処理 [score.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************

enum HandState
{
    HAND_IDLE,
    HAND_HIDING,
    HAND_HIDDEN,
    HAND_SHOWING
};

extern HandState g_HandState;
extern float g_HandOffsetY;


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void InitOverlay2D();
void UninitOverlay2D();
void UpdateOverlay2D();
void DrawOverlay2D();


void PlayMeleeAnimation();


bool IsTutorialShowing();
void SetTutorialShowing(bool flag);
float GetHandOffsetY();
void SpawnRocketExplosion(const DirectX::XMFLOAT3& pos, float size);