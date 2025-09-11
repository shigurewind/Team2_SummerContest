//=============================================================================
//
// ゲーム画面処理 [game.cpp]
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

#include "blood.h"

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


#include "boundingBoxDebug.h"
#include "map.h"
#include "inputManager.h"



//*****************************************************************************
// マクロ定義
//*****************************************************************************



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void CheckHit(void);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

BOOL	g_bPause1 = FALSE;	// ポーズON/OFF


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGame1(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// フィールドの初期化
	//InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 100, 100, 13.0f, 13.0f);

	// ライトを有効化	// 影の初期化処理
	InitShadow();

	// プレイヤーの初期化
	InitPlayer();

	//// エネミーの初期化
	InitEnemy();

	InitBlood();




	// 弾の初期化
	InitBullet();

	// UIの初期化
	InitGameUI();

	InitOverlay2D();
	// パーティクルの初期化
	InitParticle();

	//InitFBXTestModel();

	InitItem();

	// デバッグ用のバウンディングボックスの初期化
	BoundingBoxDebugRenderer::GetInstance().Initialize();


	// BGM再生
	PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGame1(void)
{
	// パーティクルの終了処理
	UninitParticle();

	// UIの終了処理
	UnInitGameUI();

	UninitOverlay2D();

	// 弾の終了処理
	UninitBullet();

	UninitBlood();


	// 地面の終了処理
	//UninitMeshField();

	//// エネミーの終了処理
	UninitEnemy();

	// プレイヤーの終了処理
	UninitPlayer();

	// 影の終了処理
	UninitShadow();

	UninitFBXMapModel();

	UninitItem();

	BoundingBoxDebugRenderer::GetInstance().Shutdown();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGame1(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	// 時間を止める
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
	// 地面処理の更新
	//UpdateMeshField();
	UpdateFBXMapModel();

	// プレイヤーの更新処理
	UpdatePlayer();

	// リセット処理
	if (g_pInputManager->IsResetTriggered())
	{
		ResetPlayerPosition();
	}

	//// エネミーの更新処理
	UpdateEnemy();


	// 弾の更新処理
	UpdateBullet();

	// パーティクルの更新処理
	//UpdateParticle();

	// 影の更新処理
	UpdateShadow();
	// 当たり判定処理
	CheckHit();

	UpdateBlood();

	// 当たり判定処理
	UpdateOverlay2D();
	// UIの更新処理
	UpdateGameUI();

	//UpdateFBXTestModel();

	UpdateItem();


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGame01(void)
{
	if (GetFade() == FADE_OUT) {
		return;
	}

	// 3Dの物を描画する処理
	// 地面の描画処理
	//DrawMeshField();

	// 影の描画処理
	DrawShadow();

	//// エネミーの描画処理
	DrawEnemy();

	// プレイヤーの描画処理
	DrawPlayer();

	// 弾の描画処理
	DrawBullet();


	// パーティクルの描画処理
	DrawParticle();

	DrawFBXMapModel();

	DrawItem();

	//デバッグ用のバウンディングボックスの描画処理（全ての物描画した後）
	BoundingBoxDebugRenderer::GetInstance().Draw();

	DrawBlood();


	// 2Dの物を描画する処理
	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);

	// UIの描画処理
	DrawGameUI();
	DrawOverlay2D();



	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}


void DrawGame1(void)
{
	XMFLOAT3 pos;

	if (GetFade() == FADE_OUT) {
		return;
	}

#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif

	// プレイヤー視点
	//pos = GetPlayer()->pos;
	//pos.y = 0.0f;			// カメラ酔いを防ぐためにクリアしている
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

		// エネミー視点
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

		// エネミー視点
		//pos = GetEnemy()->pos;
		//pos.y = 0.0f;
		//SetCameraAT(pos);
		//SetCamera();
		//SetViewPort(TYPE_DOWN_HALF_SCREEN);
		//DrawGame0();
		break;

	}

}


