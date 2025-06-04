//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include <cstdlib>  // rand()
#include <ctime>    // time()
#include <cmath>    // cos, sin

#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "enemy.h"
#include "player.h"
#include "shadow.h"
#include "collision.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量


//エネミーの動き範囲
#define ENEMY_AREA_MIN_X	(-80.0f)
#define ENEMY_AREA_MAX_X	(80.0f)
#define ENEMY_AREA_MIN_Z	(-80.0f)
#define ENEMY_AREA_MAX_Z	(80.0f)
#define ENEMY_AREA_MIN_Y	(7.0f)
#define ENEMY_AREA_MAX_Y	(80.0f)

#define MOVECOUNTER			(300)		// 向き変わるタイマー



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー

int g_Enemy_load = 0;

static INTERPOLATION_DATA g_MoveTbl0[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 5.0f },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 5.0f },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 5.0f },

};

static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	g_MoveTbl0,

};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, 7.0f, 20.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd  = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ
		g_Enemy[i].type = NONE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[i].model, &g_Enemy[i].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);
		

		g_Enemy[i].time = 0.0f;			// 線形補間用のタイマーをクリア
		g_Enemy[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Enemy[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット
	

		g_Enemy[i].use = TRUE;			// TRUE:生きてる
	}


	g_Enemy[0].type = GHOST;
	ChangeEnemyDirection(0);
	g_Enemy[0].spd = 0.2f;

	g_Enemy[1].type = SKELETON;
	// 1番だけ線形補間で動かしてみる
	g_Enemy[1].time = 0.0f;		// 線形補間用のタイマーをクリア
	g_Enemy[1].tblNo = 0;		// 再生するアニメデータの先頭アドレスをセット
	g_Enemy[1].tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{

	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		switch (g_Enemy[i].type)
		{
		case GHOST:
			GhostMovement(i);
			break;

		case SKELETON:
			SkeletonMovement(i);
			break;

		default:
			break;
		}
	}





#ifdef _DEBUG

	if (GetKeyboardTrigger(DIK_P))
	{
		// モデルの色を変更できるよ！半透明にもできるよ。
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
		}
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		// モデルの色を元に戻している
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, g_Enemy[0].diffuse[j]);
		}
	}
#endif

}



//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Enemy[i].model);
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}

//=============================================================================
// エネミーの動き
//=============================================================================

void ChangeEnemyDirection(int i) {
	float theta = (rand() % 360) * XM_PI / 180.0f;
	float phi = ((rand() % 90) + 45) * XM_PI / 180.0f;

	XMFLOAT3 dir;
	dir.x = sinf(phi) * cosf(theta);
	dir.y = cosf(phi);
	dir.z = sinf(phi) * sinf(theta);

	XMVECTOR v = XMLoadFloat3(&dir);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&g_Enemy[i].dir, v);

	g_Enemy[i].moveCounter = MOVECOUNTER + rand() % 60;
}

void ChasingPlayer(int i)
{
	PLAYER* player = GetPlayer();	// プレイヤーのポインターを初期化

	//BCの当たり判定
	//追いかけらえる範囲
	if (CollisionBC(player->pos, g_Enemy[i].pos, player->size + 40.0f, g_Enemy[i].size + 40.0f))
	{
		PLAYER* player = GetPlayer();

		// エネミーからプレイヤーまでのベクトル
		XMFLOAT3 dir;
		dir.x = player->pos.x - g_Enemy[i].pos.x;
		dir.y = player->pos.y - g_Enemy[i].pos.y;
		dir.z = player->pos.z - g_Enemy[i].pos.z;

		// ベクトル正規化
		XMVECTOR v = XMLoadFloat3(&dir);
		v = XMVector3Normalize(v);
		XMStoreFloat3(&dir, v);

		// スピードの応用
		float speed = g_Enemy[i].spd;

		g_Enemy[i].pos.x += dir.x * speed;
		g_Enemy[i].pos.y += dir.y * speed;
		g_Enemy[i].pos.z += dir.z * speed;

	}

}

void GhostMovement(int i)
{
	ChasingPlayer(i);

	if (g_Enemy[i].use == TRUE)
	{
		float nextX = g_Enemy[i].pos.x + g_Enemy[i].dir.x * g_Enemy[i].spd;
		float nextZ = g_Enemy[i].pos.z + g_Enemy[i].dir.z * g_Enemy[i].spd;
		float nextY = g_Enemy[i].pos.y + g_Enemy[i].dir.y * g_Enemy[i].spd;

		if (nextX < ENEMY_AREA_MIN_X || nextX > ENEMY_AREA_MAX_X ||
			nextZ < ENEMY_AREA_MIN_Z || nextZ > ENEMY_AREA_MAX_Z ||
			nextY < ENEMY_AREA_MIN_Y || nextY > ENEMY_AREA_MAX_Y)
		{
			ChangeEnemyDirection(i);
		}
		else
		{
			g_Enemy[i].pos.x = nextX;
			g_Enemy[i].pos.z = nextZ;
			g_Enemy[i].pos.y = nextY;
		}

		g_Enemy[i].moveCounter--;
		if (g_Enemy[i].moveCounter <= 0)
		{
			ChangeEnemyDirection(i);
		}

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		SetPositionShadow(g_Enemy[i].shadowIdx, pos);
	}
}

void SkeletonMovement(int i)
{
	if (g_Enemy[i].use == FALSE || g_Enemy[i].tblMax <= 0)
		return;

	int nowNo = (int)g_Enemy[i].time;
	int maxNo = g_Enemy[i].tblMax;
	int nextNo = (nowNo + 1) % maxNo;
	INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Enemy[i].tblNo];

	XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);
	XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);
	XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);

	XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;
	XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;
	XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;

	float nowTime = g_Enemy[i].time - nowNo;

	Pos *= nowTime;
	Rot *= nowTime;
	Scl *= nowTime;

	XMStoreFloat3(&g_Enemy[i].pos, nowPos + Pos);
	XMStoreFloat3(&g_Enemy[i].rot, nowRot + Rot);
	XMStoreFloat3(&g_Enemy[i].scl, nowScl + Scl);

	g_Enemy[i].time += 1.0f / tbl[nowNo].frame;
	if ((int)g_Enemy[i].time >= maxNo)
	{
		g_Enemy[i].time -= maxNo;
	}

	XMFLOAT3 pos = g_Enemy[i].pos;
	pos.y -= (ENEMY_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Enemy[i].shadowIdx, pos);
}
