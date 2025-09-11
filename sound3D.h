//=============================================================================
//
// サウンド処理 [sound.h]
//
//=============================================================================
#pragma once

// sound3D.h
#pragma once
#include <windows.h>
#include <xaudio2.h>
#include <X3DAudio.h>
#include <DirectXMath.h>

using namespace DirectX;

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum
{
    SOUND3D_GUNSHOT,
    SOUND3D_ENEMY_GHOST,
    SOUND3D_MAX
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

BOOL InitSound3D(HWND hWnd);
void UninitSound3D();
void UpdateListener(XMFLOAT3 listenerPos, XMFLOAT3 listenerFront);
void UpdateSound3D();
void PlaySound3D(int label, XMFLOAT3 pos);
