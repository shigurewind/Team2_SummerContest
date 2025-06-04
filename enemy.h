//=============================================================================
//
// エネミーモデル処理 [enemy.h]
// Author : 
//
//=============================================================================
#pragma once
#include "shadow.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ENEMY		(5)					// エネミーの数

#define	ENEMY_SIZE		(5.0f)				// 当たり判定の大きさ

#define	MODEL_ENEMY			"data/MODEL/enemy.obj"		// 読み込むモデル名
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる
#define ENEMY_SHADOW_SIZE	(0.4f)						// 影の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct ENEMY 
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)
	XMFLOAT3			dir;				// 動きの向き
	int					type;


	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号

	float				time;				// 線形補間用
	int					tblNo;				// 行動データのテーブル番号
	int					tblMax;				// そのテーブルのデータ数

	int					moveCounter;		// 向き変わるタイマー
};

enum ENEMY_TYPE
{
	GHOST = 0,
	SKELETON,
	SPIDER,
	NONE
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY *GetEnemy(void);

void ChangeEnemyDirection(int i);
void ChasingPlayer(int i);
void GhostMovement(int i);
void SkeletonMovement(int i);

