#pragma once

#include "model.h"
//0522kabe
//shinnki

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_WALLANDFLOOR		(2)					// エネミーの数//0526floor

#define	WALLANDFLOOR_SIZE		(5.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct WALLANDFLOOR
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号


};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitWallAndFloor(void);
void UninitWallAndFloor(void);
void UpdateWallAndFloor(void);
void DrawWallAndFloor(void);

WALLANDFLOOR* GetWallAndFloor(void);

