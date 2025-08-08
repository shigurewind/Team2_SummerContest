#include "FBXmodel.h"
#include "Octree.h"
#include "shaderManager.h"


//-------------------------------------------------------------------------
static std::vector<TriangleData> g_TriangleList;
static FBXTESTMODEL g_FBXTestModel;	// FBX���f���̃f�[�^




static OctreeNode* g_WallTree = nullptr;
static OctreeNode* g_FloorTree = nullptr;

static std::vector<TriangleData> g_FloorTris;
static std::vector<TriangleData> g_WallTris;

const std::vector<TriangleData>& GetFloorTriangles() { return g_FloorTris; }
const std::vector<TriangleData>& GetWallTriangles() { return g_WallTris; }

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;

	//g_FBXTestModel.model = ModelLoad("data/MODEL/model.fbx");	// FBX���f���̓ǂݍ���
	g_FBXTestModel.model = ModelLoad("data/MODEL/stage111.fbx");	// FBX���f���̓ǂݍ���
	if (!g_FBXTestModel.model) {
		MessageBoxA(NULL, "Failed to load stage1.fbx", "Error", MB_OK);
		return E_FAIL;
	}

	char debugPos[128];
	sprintf_s(debugPos, "FBX pos.y = %.2f\n", g_FBXTestModel.pos.y);
	OutputDebugStringA(debugPos);

	


	g_FBXTestModel.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_FBXTestModel.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_FBXTestModel.alive = TRUE;			// TRUE:�����Ă�
	XMMATRIX mtxScl = XMMatrixScaling(g_FBXTestModel.scl.x, g_FBXTestModel.scl.y, g_FBXTestModel.scl.z);
	XMMATRIX mtxRot = XMMatrixRotationRollPitchYaw(g_FBXTestModel.rot.x, g_FBXTestModel.rot.y + XM_PI, g_FBXTestModel.rot.z);
	XMMATRIX mtxQuat = XMMatrixRotationQuaternion(XMLoadFloat4(&g_FBXTestModel.Quaternion));
	XMMATRIX mtxTrans = XMMatrixTranslation(g_FBXTestModel.pos.x, g_FBXTestModel.pos.y, g_FBXTestModel.pos.z);

	XMMATRIX world = mtxScl * mtxRot * mtxQuat * mtxTrans;

	ExtractTriangleData(g_FBXTestModel.model, world);



	g_FloorTris.clear();
	g_WallTris.clear();

	for (const auto& tri : GetTriangleList()) {
		if (tri.type == TYPE_FLOOR) g_FloorTris.push_back(tri);
		else if (tri.type == TYPE_WALL) g_WallTris.push_back(tri);
	}
	float floorMinY = FLT_MAX, floorMaxY = -FLT_MAX;
	for (const auto& tri : g_FloorTris) {
		for (auto v : { tri.v0, tri.v1, tri.v2 }) {
			floorMinY = min(floorMinY, v.y);
			floorMaxY = max(floorMaxY, v.y);
		}
	}
	char debugFloor[128];
	sprintf_s(debugFloor, "Floor Y Range: min = %.2f, max = %.2f\n", floorMinY, floorMaxY);
	OutputDebugStringA(debugFloor);

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

	for (const auto& tri : g_FloorTris) updateBounds(tri);
	g_FloorTree = BuildOctree(g_FloorTris, minBound, maxBound, 0, 6,1);
			
	minBound = { FLT_MAX, FLT_MAX, FLT_MAX };
	maxBound = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (const auto& tri : g_WallTris) updateBounds(tri);
	g_WallTree = BuildOctree(g_WallTris, minBound, maxBound, 0, 6, 1);


	

	return S_OK;
}

void UninitFBXTestModel(void)
{
	// ���f���̉������
	if (g_FBXTestModel.load == TRUE)
	{
		ModelRelease(g_FBXTestModel.model);	// FBX���f���̉��
		g_FBXTestModel.load = FALSE;
	}

}

void UpdateFBXTestModel(void)
{
	//g_FBXTestModel.rot.y += 0.01f;	// ��]�����Ă݂�
	//g_FBXTestModel.rot.x += 0.01f;
	//g_FBXTestModel.pos.x +=  0.1f;	// X�������Ɉړ�
}

void DrawFBXTestModel(void)
{
	SHADER_SCOPE(SHADER_TERRAIN);//����Shader�؂�ւ�

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_FBXTestModel.scl.x, g_FBXTestModel.scl.y, g_FBXTestModel.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_FBXTestModel.rot.x, g_FBXTestModel.rot.y + XM_PI, g_FBXTestModel.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �N�H�[�^�j�I���𔽉f
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_FBXTestModel.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_FBXTestModel.pos.x, g_FBXTestModel.pos.y, g_FBXTestModel.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_FBXTestModel.mtxWorld, mtxWorld);

	MATERIAL mat = {};
	mat.Diffuse = XMFLOAT4(0.6f, 0.0f, 0.0f, 1.0f);
	mat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mat.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mat.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mat.Shininess = 16.0f;
	mat.noTexSampling = 0;//�d�v�FFBX�S��0�ɂ���

	SetMaterial(mat);

	// �����̐ݒ�
	//SetFuchi(1);


	// ���f���`��
	ModelDraw(g_FBXTestModel.model);

	
	//SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
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
				XMVECTOR vLocal = XMVectorSet(vRaw.x, -vRaw.z, vRaw.y, 1.0f);
				XMVECTOR vWorld = XMVector3Transform(vLocal, worldMatrix);
				XMStoreFloat3(&v[j], vWorld);
			}

			// �@���v�Z
			XMVECTOR edge1 = XMVectorSubtract(XMLoadFloat3(&v[1]), XMLoadFloat3(&v[0]));
			XMVECTOR edge2 = XMVectorSubtract(XMLoadFloat3(&v[2]), XMLoadFloat3(&v[0]));
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge1, edge2));

			XMFLOAT3 normal3;
			XMStoreFloat3(&normal3, normal);

			// �O�p�`�̕��ނ����P
			TriangleType type = TYPE_UNKNOWN;
			float normalY = fabs(normal3.y);

			if (normalY >= 0.5f) {
				type = TYPE_FLOOR;
			}
			else {
				type = TYPE_WALL;
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