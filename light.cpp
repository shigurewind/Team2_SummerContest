//=============================================================================
//
// ライト処理 [light.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "camera.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

static BOOL		g_FogEnable = TRUE;

static BOOL g_SpotlightEnabled = TRUE;


//=============================================================================
// 初期化処理
//=============================================================================
void InitLight(void)
{

	//ライト初期化
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Light[i].Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		g_Light[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_Light[i].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		g_Light[i].Attenuation = 100.0f;	// 減衰距離
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ライトのタイプ
		g_Light[i].Enable = FALSE;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// 並行光源の設定（世界を照らす光）
	g_Light[0].Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);		// 光の向き
	g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 光の色
	g_Light[0].Ambient = XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);	// 環境光の色
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
	g_Light[0].Enable = TRUE;									// このライトをON
	SetLight(0, &g_Light[0]);									// これで設定している



	// フォグの初期化（霧の効果）
	g_Fog.FogStart = 1000.0f;									// 視点からこの距離離れるとフォグがかかり始める
	g_Fog.FogEnd = 1000.0f;									// ここまで離れるとフォグの色で見えなくなる
	g_Fog.FogColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);		// フォグの色
	SetFog(&g_Fog);
	SetFogEnable(g_FogEnable);				// 他の場所もチェックする shadow

	SetLightEnable(TRUE);
}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateLight(void)
{



}


//=============================================================================
// ライトの設定
// Typeによってセットするメンバー変数が変わってくる
//=============================================================================
void SetLightData(int index, LIGHT* light)
{
	SetLight(index, light);
}


LIGHT* GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// フォグの設定
//=============================================================================
void SetFogData(FOG* fog)
{
	SetFog(fog);
}


BOOL	GetFogEnable(void)
{
	return(g_FogEnable);
}


void UpdateSpotlight(void)
{
	if (!g_SpotlightEnabled) return;

	CAMERA* cam = GetCamera();
	LIGHT* light = GetLightData(1);

	// 位置：プレイヤーの頭付近（カメラ位置）
	XMFLOAT3 pos = cam->pos;

	// 向き：カメラが向いている方向（at - pos）を正規化
	XMFLOAT3 dir = { cam->at.x - cam->pos.x, cam->at.y - cam->pos.y, cam->at.z - cam->pos.z };
	float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (len > 0.0001f) { dir.x /= len; dir.y /= len; dir.z /= len; }
	else { dir = { 0,0,1 }; }

	light->Position = pos;
	light->Direction = dir;
	light->Diffuse = XMFLOAT4(1, 1, 1, 1);     // 色はお好みで
	light->Ambient = XMFLOAT4(0, 0, 0, 0);     // 懐中電灯なので環境光は0でOK
	light->Attenuation = 500.0f;                 // 距離（到達範囲）
	light->SpotInnerCos = cosf(XMConvertToRadians(20.0f)); // 内側（明るい）コーン
	light->SpotOuterCos = cosf(XMConvertToRadians(25.0f)); // 外側（薄暗くなる）コーン
	light->SpotExponent = 5.0f;                  // 縁の落ち方の鋭さ
	light->Type = LIGHT_TYPE_SPOT;       // ★ スポットに変更
	light->Enable = g_SpotlightEnabled;

	SetLightData(1, light);
}

BOOL GetSpotlightEnabled(void)
{
	return g_SpotlightEnabled;
}

void SetSpotlightEnabled(BOOL enable)
{
	g_SpotlightEnabled = enable;

	// 光源をON/OFF
	LIGHT* light = GetLightData(1);
	light->Enable = enable;
	SetLightData(1, light);
}
