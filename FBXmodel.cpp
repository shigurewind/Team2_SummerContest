#include "FBXmodel.h"
#include "Octree.h"


//-------------------------------------------------------------------------
static std::vector<TriangleData> g_TriangleList;
static FBXTESTMODEL g_FBXTestModel;	// FBXモデルのデータ

static SHADER g_shaderCustom;

static OctreeNode* g_WallTree = nullptr;
static OctreeNode* g_FloorTree = nullptr;

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;

	//g_FBXTestModel.model = ModelLoad("data/MODEL/model.fbx");	// FBXモデルの読み込み
	g_FBXTestModel.model = ModelLoad("data/MODEL/stage1.fbx");	// FBXモデルの読み込み
	if (!g_FBXTestModel.model) {
		MessageBoxA(NULL, "Failed to load stage1.fbx", "Error", MB_OK);
		return E_FAIL;
	}
	LoadShaderFromFile("Shader/testShader.hlsl", "VertexShaderPolygon", "PixelShaderPolygon", &g_shaderCustom);
	g_FBXTestModel.shader = &g_shaderCustom;


	g_FBXTestModel.pos = XMFLOAT3(-10.0f, 0.0f, -50.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(20.0f, 20.0f, 20.0f);

	g_FBXTestModel.spd = 0.0f;			// 移動スピードクリア

	g_FBXTestModel.Quaternion = XMFLOAT4(0, 0, 0, 1);

	g_FBXTestModel.alive = TRUE;			// TRUE:生きてる
	XMMATRIX mtxScl = XMMatrixScaling(g_FBXTestModel.scl.x, g_FBXTestModel.scl.y, g_FBXTestModel.scl.z);
	XMMATRIX mtxRot = XMMatrixRotationRollPitchYaw(g_FBXTestModel.rot.x, g_FBXTestModel.rot.y + XM_PI, g_FBXTestModel.rot.z);
	XMMATRIX mtxQuat = XMMatrixRotationQuaternion(XMLoadFloat4(&g_FBXTestModel.Quaternion));
	XMMATRIX mtxTrans = XMMatrixTranslation(g_FBXTestModel.pos.x, g_FBXTestModel.pos.y, g_FBXTestModel.pos.z);

	XMMATRIX world = mtxScl * mtxRot * mtxQuat * mtxTrans;

	ExtractTriangleData(g_FBXTestModel.model, world);
	std::vector<TriangleData> floorTris;
	std::vector<TriangleData> wallTris;

	for (const auto& tri : GetTriangleList()) {
		if (tri.type == TYPE_FLOOR) floorTris.push_back(tri);
		else if (tri.type == TYPE_WALL) wallTris.push_back(tri);
	}

	XMFLOAT3 minBound = { FLT_MAX, FLT_MAX, FLT_MAX };
	XMFLOAT3 maxBound = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	auto updateBounds = [&](const TriangleData& tri) {
		for (const XMFLOAT3& v : { tri.v0, tri.v1, tri.v2 }) {
			minBound.x = min(minBound.x, v.x);
			minBound.y = min(minBound.y, v.y);
			minBound.z = min(minBound.z, v.z);
			maxBound.x = max(maxBound.x, v.x);
			maxBound.y = max(maxBound.y, v.y);
			maxBound.z = max(maxBound.z, v.z);
		}
		};

	for (const auto& tri : floorTris) updateBounds(tri);
	g_FloorTree = BuildOctree(floorTris, minBound, maxBound, 0, 5, 10);

	minBound = { FLT_MAX, FLT_MAX, FLT_MAX };
	maxBound = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (const auto& tri : wallTris) updateBounds(tri);
	g_WallTree = BuildOctree(wallTris, minBound, maxBound, 0, 5, 10);

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



void ExtractTriangleData(AMODEL* model, const XMMATRIX& worldMatrix)
{
	g_TriangleList.clear();

	if (!model || !model->AiScene) {
		OutputDebugStringA("Error: Invalid model or AiScene\n");
		return;
	}

	for (unsigned int meshIndex = 0; meshIndex < model->AiScene->mNumMeshes; meshIndex++) {
		const aiMesh* mesh = model->AiScene->mMeshes[meshIndex];
		if (!mesh || !mesh->mVertices) {
			continue;
		}

		const aiVector3D* vertices = mesh->mVertices;

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices != 3) continue;

			XMFLOAT3 v[3];
			for (int j = 0; j < 3; j++) {
				aiVector3D vRaw = vertices[face.mIndices[j]];
				//XMVECTOR vLocal = XMVectorSet(vRaw.x, vRaw.y, vRaw.z, 1.0f);
				XMVECTOR vLocal = XMVectorSet(vRaw.x, -vRaw.z, vRaw.y, 1.0f);//右手座標を左手に
				XMVECTOR vWorld = XMVector3Transform(vLocal, worldMatrix);
				XMStoreFloat3(&v[j], vWorld);
			}

			// 法線計算
			XMVECTOR edge1 = XMVectorSubtract(XMLoadFloat3(&v[1]), XMLoadFloat3(&v[0]));
			XMVECTOR edge2 = XMVectorSubtract(XMLoadFloat3(&v[2]), XMLoadFloat3(&v[0]));
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge1, edge2));

			XMFLOAT3 normal3;
			XMStoreFloat3(&normal3, normal);

			// 三角形の分類を改善
			TriangleType type = TYPE_UNKNOWN;
			float normalY = fabs(normal3.y);

			if (normalY >= 0.5f) {  // Y成分が0.5以上なら床
				type = TYPE_FLOOR;
			}
			else if (normalY <= 0.3f) {  // Y成分が0.3以下なら壁
				type = TYPE_WALL;
			}
			else {
				type = TYPE_FLOOR; // 中間的な角度も床として扱う
			}

			g_TriangleList.push_back({ v[0], v[1], v[2], normal3, type });
			char buf[128];
			sprintf_s(buf, "Normal Y: %.2f Type: %d\n", normal3.y, type);
			OutputDebugStringA(buf);
		}
	}

	char debugMsg[256];
	sprintf_s(debugMsg, "ExtractTriangleData: %zu triangles extracted\n", g_TriangleList.size());
	OutputDebugStringA(debugMsg);
	
	
}

const std::vector<TriangleData>& GetTriangleList()
{
	return g_TriangleList;
}

OctreeNode* GetWallTree() { return g_WallTree; }
OctreeNode* GetFloorTree() { return g_FloorTree; }