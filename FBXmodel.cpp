#include "FBXmodel.h"
#include "collision.h"



//-------------------------------------------------------------------------

static FBXTESTMODEL g_FBXTestModel;	// FBXモデルのデータ

static SHADER g_shaderCustom;

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;

	//g_FBXTestModel.model = ModelLoad("data/MODEL/model.fbx");	// FBXモデルの読み込み
	g_FBXTestModel.model = ModelLoad("data/MODEL/stage07013.fbx");	// FBXモデルの読み込み

	LoadShaderFromFile("Shader/testShader.hlsl", "VertexShaderPolygon", "PixelShaderPolygon", &g_shaderCustom);
	g_FBXTestModel.shader = &g_shaderCustom;


	g_FBXTestModel.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_FBXTestModel.spd = 0.0f;			// 移動スピードクリア

	g_FBXTestModel.alive = TRUE;			// TRUE:生きてる

	ExtractWallTrianglesFromFBX(&g_FBXTestModel);
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

	// シェーダーの設定
	ID3D11DeviceContext* context = GetDeviceContext();
	if (g_FBXTestModel.shader)
	{
		context->IASetInputLayout(g_FBXTestModel.shader->inputLayout);
		context->VSSetShader(g_FBXTestModel.shader->vertexShader, nullptr, 0);
		context->PSSetShader(g_FBXTestModel.shader->pixelShader, nullptr, 0);
	}

	// モデル描画
	ModelDraw(g_FBXTestModel.model);

	//デフォルトShaderに戻す
	SetDefaultShader();


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






BOOL RayHitFBXModel(AMODEL* model, XMFLOAT3 start, XMFLOAT3 end, XMFLOAT3* hitPos, XMFLOAT3* normal)
{
	if (!model || !model->AiScene) return FALSE;

	const aiScene* scene = model->AiScene;
	if (scene->mNumMeshes == 0) return FALSE;
	BOOL hit = FALSE;
	float nearestT = FLT_MAX;
	XMMATRIX mtxWorld = XMLoadFloat4x4(&GetFBXTestModel()->mtxWorld);

	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];
		if (!mesh || mesh->mNumFaces == 0 || !mesh->HasPositions()) continue;
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices != 3) continue;  

			aiVector3D v0 = mesh->mVertices[face.mIndices[0]];
			aiVector3D v1 = mesh->mVertices[face.mIndices[1]];
			aiVector3D v2 = mesh->mVertices[face.mIndices[2]];

			XMFLOAT3 p0 = { v0.x, v0.y, v0.z };
			XMFLOAT3 p1 = { v1.x, v1.y, v1.z };
			XMFLOAT3 p2 = { v2.x, v2.y, v2.z };

			XMVECTOR w0 = XMVector3TransformCoord(XMLoadFloat3(&p0), mtxWorld);
			XMVECTOR w1 = XMVector3TransformCoord(XMLoadFloat3(&p1), mtxWorld);
			XMVECTOR w2 = XMVector3TransformCoord(XMLoadFloat3(&p2), mtxWorld);
			XMStoreFloat3(&p0, w0);
			XMStoreFloat3(&p1, w1);
			XMStoreFloat3(&p2, w2);

			float len1 = XMVectorGetX(XMVector3Length(w1 - w0));
			float len2 = XMVectorGetX(XMVector3Length(w2 - w0));
			if (len1 < 0.001f || len2 < 0.001f) continue;


			XMFLOAT3 tempHitPos, tempNormal;
			if (RayCast(p0, p1, p2, start, end, &tempHitPos, &tempNormal))
			{
				float dx = tempHitPos.x - start.x;
				float dz = tempHitPos.z - start.z;
				float distXZ = dx * dx + dz * dz;
				
				if (distXZ > 40000.0f) continue;

				float dy = tempHitPos.y - start.y;
				float dist3d = dx * dx + dy * dy + dz * dz;
				if (dist3d < nearestT)
				{
					nearestT = dist3d;
					*hitPos = tempHitPos;
					*normal = tempNormal;
					hit = TRUE;
				}
			}
		}
	}

	return hit;
}

void ExtractWallTrianglesFromFBX(FBXTESTMODEL* fbxModel)
{
	if (!fbxModel || !fbxModel->model || !fbxModel->model->AiScene) return;

	const aiScene* scene = fbxModel->model->AiScene;
	XMMATRIX mtxWorld = XMLoadFloat4x4(&fbxModel->mtxWorld);

	fbxModel->wallTriangles.clear();
	fbxModel->floorTriangles.clear();
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];
		if (!mesh || mesh->mNumFaces == 0 || !mesh->HasPositions()) continue;

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices != 3) continue;

			aiVector3D v0 = mesh->mVertices[face.mIndices[0]];
			aiVector3D v1 = mesh->mVertices[face.mIndices[1]];
			aiVector3D v2 = mesh->mVertices[face.mIndices[2]];

			XMFLOAT3 p0 = { v0.x, v0.y, v0.z };
			XMFLOAT3 p1 = { v1.x, v1.y, v1.z };
			XMFLOAT3 p2 = { v2.x, v2.y, v2.z };

			XMVECTOR w0 = XMVector3TransformCoord(XMLoadFloat3(&p0), mtxWorld);
			XMVECTOR w1 = XMVector3TransformCoord(XMLoadFloat3(&p1), mtxWorld);
			XMVECTOR w2 = XMVector3TransformCoord(XMLoadFloat3(&p2), mtxWorld);
			XMStoreFloat3(&p0, w0);
			XMStoreFloat3(&p1, w1);
			XMStoreFloat3(&p2, w2);

			XMVECTOR edge1 = w1 - w0;
			XMVECTOR edge2 = w2 - w0;
			XMVECTOR normalVec = XMVector3Normalize(XMVector3Cross(edge1, edge2));
			XMFLOAT3 normal;
			XMStoreFloat3(&normal, normalVec);

			if (fabs(normal.y) > 0.6f) {
				fbxModel->floorTriangles.push_back({ p0, p1, p2 }); 
			}
			else {
				fbxModel->wallTriangles.push_back({ p0, p1, p2 });  
			}
		}
	}
}