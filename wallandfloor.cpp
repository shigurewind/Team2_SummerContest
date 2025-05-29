//=============================================================================
////0522kabe
//shinnki
// エネミーモデル処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "game.h"
#include "renderer.h"
#include "input.h"
#include "debugproc.h"
#include "wallandfloor.h"
#include "shadow.h"
#include "collision.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
//0526floor
#define	MODEL_WALLANDFLOOR_01			"data/MODEL/rockroom.obj"		// 読み込むモデル名
#define	MODEL_WALLANDFLOOR_02			"data/MODEL/flor.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define WALLANDFLOOR_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define WALLANDFLOOR_OFFSET_Y		(0.2f)						// エネミーの足元をあわせる


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static WALLANDFLOOR			g_WallAndFloor[MAX_WALLANDFLOOR];				// エネミー

int g_WallAndFloor_load = 0;


static INTERPOLATION_DATA g_MoveTbl0[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, WALLANDFLOOR_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 2 },
	{ XMFLOAT3(-200.0f, WALLANDFLOOR_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 6.28f, 0.0f), XMFLOAT3(3.0f, 3.0f, 3.0f), 60 * 1 },
	{ XMFLOAT3(-200.0f, WALLANDFLOOR_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 0.5f },

};


static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	g_MoveTbl0,

};



//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitWallAndFloor(void)
{/*
	for (int i = 0; i < MAX_WALLANDFLOOR; i++)
	{
		LoadModel(MODEL_WALLANDFLOOR_02, &g_WallAndFloor[i].model);
		g_WallAndFloor[i].load = TRUE;

		g_WallAndFloor[i].pos = XMFLOAT3(-50.0f + i * 160.0f, 0.0f, 20.0f);
		g_WallAndFloor[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_WallAndFloor[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_WallAndFloor[i].size = WALLANDFLOOR_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_WallAndFloor[i].model, &g_WallAndFloor[i].diffuse[0]);

		XMFLOAT3 pos = g_WallAndFloor[i].pos;
		pos.y -= (WALLANDFLOOR_OFFSET_Y - 0.1f);
		g_WallAndFloor[i].shadowIdx = CreateShadow(pos, WALLANDFLOOR_SHADOW_SIZE, WALLANDFLOOR_SHADOW_SIZE);


		g_WallAndFloor[i].use = TRUE;			// TRUE:生きてる

	}*/
	
	//0526floor//for more floor or rock models.
	LoadModel(MODEL_WALLANDFLOOR_01, &g_WallAndFloor[0].model);
	g_WallAndFloor[0].load = TRUE;
	g_WallAndFloor[0].pos = XMFLOAT3(-50.0f, 0.0f, 20.0f);
	g_WallAndFloor[0].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_WallAndFloor[0].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_WallAndFloor[0].size = WALLANDFLOOR_SIZE;
	GetModelDiffuse(&g_WallAndFloor[0].model, &g_WallAndFloor[0].diffuse[0]);
	XMFLOAT3 pos0 = g_WallAndFloor[0].pos;
	pos0.y -= (WALLANDFLOOR_OFFSET_Y - 0.1f);
	g_WallAndFloor[0].shadowIdx = CreateShadow(pos0, WALLANDFLOOR_SHADOW_SIZE, WALLANDFLOOR_SHADOW_SIZE);
	g_WallAndFloor[0].use = TRUE;


	LoadModel(MODEL_WALLANDFLOOR_02, &g_WallAndFloor[1].model);
	g_WallAndFloor[1].load = TRUE;
	g_WallAndFloor[1].pos = XMFLOAT3(50.0f, 1.0f, -50.0f);
	g_WallAndFloor[1].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_WallAndFloor[1].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_WallAndFloor[1].size = WALLANDFLOOR_SIZE;
	GetModelDiffuse(&g_WallAndFloor[1].model, &g_WallAndFloor[1].diffuse[0]);
	XMFLOAT3 pos1 = g_WallAndFloor[1].pos;
	pos1.y -= (WALLANDFLOOR_OFFSET_Y - 0.1f);
	g_WallAndFloor[1].shadowIdx = CreateShadow(pos1, WALLANDFLOOR_SHADOW_SIZE, WALLANDFLOOR_SHADOW_SIZE);
	g_WallAndFloor[1].use = TRUE;

	

	// 0番だけ線形補間で動かしてみる

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitWallAndFloor(void)
{

	for (int i = 0; i < MAX_WALLANDFLOOR; i++)
	{
		if (g_WallAndFloor[i].load)
		{
			UnloadModel(&g_WallAndFloor[i].model);
			g_WallAndFloor[i].load = FALSE;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateWallAndFloor(void)
{
	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_WALLANDFLOOR; i++)
	{
		if (g_WallAndFloor[i].use == TRUE)		// このエネミーが使われている？
		{								// Yes



			// 影もプレイヤーの位置に合わせる
			XMFLOAT3 pos = g_WallAndFloor[i].pos;
			pos.y -= (WALLANDFLOOR_OFFSET_Y - 0.1f);
			SetPositionShadow(g_WallAndFloor[i].shadowIdx, pos);
		}
	}




#ifdef _DEBUG

	if (GetKeyboardTrigger(DIK_P))
	{
		// モデルの色を変更できるよ！半透明にもできるよ。
		for (int j = 0; j < g_WallAndFloor[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_WallAndFloor[0].model, j, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
		}
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		// モデルの色を元に戻している
		for (int j = 0; j < g_WallAndFloor[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_WallAndFloor[0].model, j, g_WallAndFloor[0].diffuse[j]);
		}
	}
#endif


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawWallAndFloor(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_BACK);

	for (int i = 0; i < MAX_WALLANDFLOOR; i++)
	{
		if (g_WallAndFloor[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_WallAndFloor[i].scl.x, g_WallAndFloor[i].scl.y, g_WallAndFloor[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_WallAndFloor[i].rot.x, g_WallAndFloor[i].rot.y + XM_PI, g_WallAndFloor[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_WallAndFloor[i].pos.x, g_WallAndFloor[i].pos.y, g_WallAndFloor[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_WallAndFloor[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_WallAndFloor[i].model);
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
WALLANDFLOOR* GetWallAndFloor()
{
	return &g_WallAndFloor[0];
}
BOOL RayHitWallAndFloor(XMFLOAT3 pos, XMFLOAT3* HitPosition, XMFLOAT3* Normal)
{
	XMFLOAT3 start = pos;
	XMFLOAT3 end = pos;
	start.y += 100.0f;
	end.y -= 1000.0f;

	BOOL hit = FALSE;
	float closestDist = FLT_MAX;

	for (int i = 0; i < MAX_WALLANDFLOOR; i++)
	{
		if (!g_WallAndFloor[i].use) continue;

		DX11_MODEL* model = &g_WallAndFloor[i].model;
		XMMATRIX world = XMLoadFloat4x4(&g_WallAndFloor[i].mtxWorld);

		for (UINT j = 0; j < model->IndexNumCPU; j += 3)
		{
			int i0 = model->IndexArrayCPU[j];
			int i1 = model->IndexArrayCPU[j + 1];
			int i2 = model->IndexArrayCPU[j + 2];

			XMFLOAT3 p0, p1, p2;
			XMStoreFloat3(&p0, XMVector3TransformCoord(XMLoadFloat3(&model->VertexArrayCPU[i0].Position), world));
			XMStoreFloat3(&p1, XMVector3TransformCoord(XMLoadFloat3(&model->VertexArrayCPU[i1].Position), world));
			XMStoreFloat3(&p2, XMVector3TransformCoord(XMLoadFloat3(&model->VertexArrayCPU[i2].Position), world));

			XMFLOAT3 tempHit, tempNormal;
			if (RayCast(p0, p1, p2, start, end, &tempHit, &tempNormal))
			{
				float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&tempHit) - XMLoadFloat3(&start)));
				if (dist < closestDist)
				{
					closestDist = dist;
					*HitPosition = tempHit;
					*Normal = tempNormal;
					hit = TRUE;
				}
			}
		}
	}

	return hit;
}