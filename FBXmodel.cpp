#include "FBXmodel.h"



//-------------------------------------------------------------------------

static FBXTESTMODEL g_FBXTestModel;	// FBXモデルのデータ

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;
	
	//g_FBXTestModel.model = ModelLoad("data/MODEL/model.fbx");	// FBXモデルの読み込み
	g_FBXTestModel.model = ModelLoad("data/MODEL/rockkk.fbx");	// FBXモデルの読み込み

	


	g_FBXTestModel.pos = XMFLOAT3(-10.0f, 20.0f, -50.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(10.0f, 10.0f, 10.0f);

	g_FBXTestModel.spd = 0.0f;			// 移動スピードクリア

	g_FBXTestModel.alive = TRUE;			// TRUE:生きてる


	return S_OK;
}

void UninitFBXTestModel(void)
{
	// モデルの解放処理
	if (g_FBXTestModel.load == TRUE)
	{
		ModelRelease(g_FBXTestModel.model);	// FBXモデルの解放
		g_FBXTestModel.load = FALSE;
	}

}

void UpdateFBXTestModel(void)
{
	//g_FBXTestModel.rot.y += 0.01f;	// 回転させてみる
	//g_FBXTestModel.rot.x += 0.01f;
	//g_FBXTestModel.pos.x +=  0.1f;	// X軸方向に移動
}

void DrawFBXTestModel(void)
{
	

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_FBXTestModel.scl.x, g_FBXTestModel.scl.y, g_FBXTestModel.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_FBXTestModel.rot.x, g_FBXTestModel.rot.y + XM_PI, g_FBXTestModel.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// クォータニオンを反映
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_FBXTestModel.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_FBXTestModel.pos.x, g_FBXTestModel.pos.y, g_FBXTestModel.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_FBXTestModel.mtxWorld, mtxWorld);


	// 縁取りの設定
	//SetFuchi(1);

	// モデル描画
	ModelDraw(g_FBXTestModel.model);


	//SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
FBXTESTMODEL* GetFBXTestModel(void)
{
	return &g_FBXTestModel;
}






