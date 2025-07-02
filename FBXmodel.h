#pragma once

#include <fbxsdk.h>
#include "model.h"

struct Triangle {
	XMFLOAT3 v0;
	XMFLOAT3 v1;
	XMFLOAT3 v2;
};


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

	SHADER*				shader;				//使うShader

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

	std::vector<Triangle> wallTriangles;
	std::vector<Triangle> floorTriangles;
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitFBXTestModel(void);
void UninitFBXTestModel(void);
void UpdateFBXTestModel(void);
void DrawFBXTestModel(void);

FBXTESTMODEL* GetFBXTestModel(void);

BOOL RayHitFBXModel(AMODEL* model, XMFLOAT3 start, XMFLOAT3 end, XMFLOAT3* hitPos, XMFLOAT3* normal);


void ExtractWallTrianglesFromFBX(FBXTESTMODEL* fbxModel);