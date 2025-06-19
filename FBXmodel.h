#pragma once

#include <fbxsdk.h>
#include "model.h"
#include "DirectXMath.h"
#include "Octree.h"

enum TriangleType {
	TYPE_UNKNOWN = -1,
	TYPE_FLOOR = 0,
	TYPE_WALL = 1
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


};

struct TriangleData {
	XMFLOAT3 v0, v1, v2;
	XMFLOAT3 normal; 
	TriangleType type; 
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitFBXTestModel(void);
void UninitFBXTestModel(void);
void UpdateFBXTestModel(void);
void DrawFBXTestModel(void);

FBXTESTMODEL* GetFBXTestModel(void);

void ExtractTriangleData(AMODEL* model, const XMMATRIX& worldMatrix);
const std::vector<TriangleData>& GetTriangleList();

OctreeNode* GetWallTree();
OctreeNode* GetFloorTree();