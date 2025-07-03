//=============================================================================
//
// ゲーム画面処理 [game.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "game.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"
#include "overlay2D.h"

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
#include "dissolveTest.h"


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

BOOL	g_bPause = FALSE;	// ポーズON/OFF


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGame(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// フィールドの初期化
	//InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 100, 100, 13.0f, 13.0f);

	// ライトを有効化	// 影の初期化処理
	InitShadow();

	// プレイヤーの初期化
	InitPlayer();

	// エネミーの初期化
	InitEnemy();

	

	

	// 弾の初期化
	InitBullet();

	// スコアの初期化
	InitScore();

	InitOverlay2D();
	// パーティクルの初期化
	InitParticle();

	InitFBXTestModel();

	InitItem();

	InitDissolveTest();

	// BGM再生
	PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGame(void)
{
	// パーティクルの終了処理
	UninitParticle();

	// スコアの終了処理
	UninitScore();
	UninitOverlay2D();
	// 弾の終了処理
	UninitBullet();

	

	// 地面の終了処理
	//UninitMeshField();

	// エネミーの終了処理
	UninitEnemy();

	// プレイヤーの終了処理
	UninitPlayer();

	// 影の終了処理
	UninitShadow();

	UninitFBXTestModel();

	UninitItem();

	UninitDissolveTest();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGame(void)
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
		g_bPause = g_bPause ? FALSE : TRUE;
	}


#endif

	if (g_bPause == TRUE)
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

	// プレイヤーの更新処理
	UpdatePlayer();

	// エネミーの更新処理
	UpdateEnemy();


	// 弾の更新処理
	UpdateBullet();

	// パーティクルの更新処理
	//UpdateParticle();

	// 影の更新処理
	UpdateShadow();

	// 当たり判定処理
	CheckHit();
	UpdateOverlay2D();
	// スコアの更新処理
	UpdateScore();

	UpdateFBXTestModel();

	UpdateItem();

	UpdateDissolveTest();
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGame0(void)
{


	// 3Dの物を描画する処理
	// 地面の描画処理
	//DrawMeshField();

	// 影の描画処理
	DrawShadow();

	// エネミーの描画処理
	DrawEnemy();

	// プレイヤーの描画処理
	DrawPlayer();

	// 弾の描画処理
	DrawBullet();

	
	// パーティクルの描画処理
	DrawParticle();

	DrawFBXTestModel();

	DrawItem();

	DrawDissolveTest();


	// 2Dの物を描画する処理
	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);

	// スコアの描画処理
	DrawScore();
	DrawOverlay2D();
	


	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}


void DrawGame(void)
{
	XMFLOAT3 pos;


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
		DrawGame0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame0();

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
		DrawGame0();

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


//=============================================================================
// 当たり判定処理
//=============================================================================
void CheckHit(void)
{
	//ENEMY *enemy = GetEnemy();		// エネミーのポインターを初期化
	//PLAYER *player = GetPlayer();	// プレイヤーのポインターを初期化
	//BULLET *bullet = GetBullet();	// 弾のポインターを初期化

	//// 敵とプレイヤーキャラ
	//for (int i = 0; i < MAX_ENEMY; i++)
	//{
	//	//敵の有効フラグをチェックする
	//	if (enemy[i].use == FALSE)
	//		continue;

	//	//BCの当たり判定
	//	if (CollisionBC(player->pos, enemy[i].pos, player->size, enemy[i].size))
	//	{
	//		// 敵キャラクターは倒される
	//		enemy[i].use = FALSE;
	//		ReleaseShadow(enemy[i].shadowIdx);

	//		// スコアを足す
	//		AddScore(100);
	//	}
	//}


	//// プレイヤーの弾と敵
	////for (int i = 0; i < MAX_BULLET; i++)
	////{
	////	//弾の有効フラグをチェックする
	////	if (bullet[i].use == FALSE)
	////		continue;

	////	// 敵と当たってるか調べる
	////	for (int j = 0; j < MAX_ENEMY; j++)
	////	{
	////		//敵の有効フラグをチェックする
	////		if (enemy[j].use == FALSE)
	////			continue;

			////BCの当たり判定
			//if (CollisionBC(bullet[i].pos, enemy[j].pos, bullet[i].fWidth, enemy[j].size))
			//{
			//	// 当たったから未使用に戻す
			//	bullet[i].use = FALSE;


	////			// 敵キャラクターは倒される
	////			enemy[j].use = FALSE;
	////			ReleaseShadow(enemy[j].shadowIdx);

	////			// スコアを足す
	////			AddScore(10);
	////		}
	////	}

	////}


	//// エネミーが全部死亡したら状態遷移
	//int enemy_count = 0;
	//for (int i = 0; i < MAX_ENEMY; i++)
	//{
	//	if (enemy[i].use == FALSE) continue;
	//	enemy_count++;
	//}

	//// エネミーが０匹？
	//if (enemy_count == 0)
	//{
	//	SetFade(FADE_OUT, MODE_RESULT);
	//}

}


