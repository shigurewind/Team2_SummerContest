#pragma once
#include "FbxMeshFile.h"

struct Box
{
	//XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	Vector3			pos;				// モデルの位置
	Vector3			rot;				// モデルの向き(回転)
	Vector3			scl;				// モデルの大きさ(スケール)

	FbxMeshFile* model;


};


HRESULT InitBox(void);
void UninitBox(void);
void UpdateBox(void);
void DrawBox(void);

Box* GetBox(void);