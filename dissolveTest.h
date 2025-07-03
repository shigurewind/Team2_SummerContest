#pragma once
#include "renderer.h"

typedef struct {
	XMFLOAT3	pos;			// 位置
	XMFLOAT3	scl;			// スケール
	MATERIAL	material;		// マテリアル

	float		dissolve;		// 溶解度
	float		dissolveSpeed;	// 溶解速度
	float		dissolveTime;	// 溶解時間

	bool		isDissolving;	// 溶解中フラグ

}DissolveTest;





DissolveTest* GetDissolveTest();

HRESULT InitDissolveTest();
void UninitDissolveTest();
void UpdateDissolveTest();
void DrawDissolveTest();

void StartDissolve();