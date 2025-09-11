//=============================================================================
//
// ライト処理 [light.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void InitLight(void);
void UpdateLight(void);

void SetLightData(int index, LIGHT *light);
void SetFogData(FOG *fog);
BOOL GetFogEnable(void);

LIGHT *GetLightData(int index);

// スポットライト用
void UpdateSpotlight(void);
BOOL GetSpotlightEnabled(void);
void SetSpotlightEnabled(BOOL enable);
void SetGlobalFogXZ_Y(float startXZ, float endXZ,
    float startY, float endY,
    const XMFLOAT4& color,
    BOOL enable = TRUE);
