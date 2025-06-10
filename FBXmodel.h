#pragma once

#include <fbxsdk.h>
#include "model.h"



//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct FBXTESTMODEL
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	float				spd;				// 移動スピード

	BOOL				load;
	AMODEL*				model;				// FBXモデル情報

	int					shadowIdx;			// 影のインデックス番号

	BOOL				alive;

	float				size;

	// 階層アニメーション用のメンバー変数
	float				time;				// 線形補間用
	int					tblNo;				// 行動データのテーブル番号
	int					tblMax;				// そのテーブルのデータ数


	// クォータニオン
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// 自分が立っている所


};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitFBXTestModel(void);
void UninitFBXTestModel(void);
void UpdateFBXTestModel(void);
void DrawFBXTestModel(void);

FBXTESTMODEL* GetFBXTestModel(void);

