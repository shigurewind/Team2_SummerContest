#include "FBXmodel.h"


//FBX�̔j��
void Destroy(FbxManager** manager, FbxIOSettings** iosetting, FbxScene** scene, FbxImporter** importer)
{
	if ((*importer) != nullptr)
	{
		(*importer)->Destroy();
		(*importer) = nullptr;
	}

	if ((*iosetting) != nullptr)
	{
		(*iosetting)->Destroy();
		(*iosetting) = nullptr;
	}

	if (*scene != nullptr)
	{
		(*scene)->Destroy();
		(*scene) = nullptr;
	}

	if ((*manager) != nullptr)
	{
		(*manager)->Destroy();
		(*manager) = nullptr;
	}
}

//
const char* GetNodeAttributeName(FbxNodeAttribute::EType attribute)
{
	static int skeleton_counter = 0;
	switch (attribute)
	{
	case fbxsdk::FbxNodeAttribute::eUnknown:
		return "eUnknown";
		break;
	case fbxsdk::FbxNodeAttribute::eNull:
		return "eNull";
		break;
	case fbxsdk::FbxNodeAttribute::eMarker:
		return "eMarker";
		break;
	case fbxsdk::FbxNodeAttribute::eSkeleton:
		skeleton_counter++;
		printf("%d\n", skeleton_counter);
		return "eSkeleton";
		break;
	case fbxsdk::FbxNodeAttribute::eMesh:
		return "eMesh";
		break;
	case fbxsdk::FbxNodeAttribute::eNurbs:
		return "eNurbs";
		break;
	case fbxsdk::FbxNodeAttribute::ePatch:
		return "ePatch";
		break;
	case fbxsdk::FbxNodeAttribute::eCamera:
		return "eCamera";
		break;
	case fbxsdk::FbxNodeAttribute::eCameraStereo:
		return "eCameraStereo";
		break;
	case fbxsdk::FbxNodeAttribute::eCameraSwitcher:
		return "eCameraSwitcher";
		break;
	case fbxsdk::FbxNodeAttribute::eLight:
		return "eLight";
		break;
	case fbxsdk::FbxNodeAttribute::eOpticalReference:
		return "eOpticalReference";
		break;
	case fbxsdk::FbxNodeAttribute::eOpticalMarker:
		return "eOpticalMarker";
		break;
	case fbxsdk::FbxNodeAttribute::eNurbsCurve:
		return "eNurbsCurve";
		break;
	case fbxsdk::FbxNodeAttribute::eTrimNurbsSurface:
		return "eTrimNurbsSurface";
		break;
	case fbxsdk::FbxNodeAttribute::eBoundary:
		return "eBoundary";
		break;
	case fbxsdk::FbxNodeAttribute::eNurbsSurface:
		return "eNurbsSurface";
		break;
	case fbxsdk::FbxNodeAttribute::eShape:
		return "eShape";
		break;
	case fbxsdk::FbxNodeAttribute::eLODGroup:
		return "eLODGroup";
		break;
	case fbxsdk::FbxNodeAttribute::eSubDiv:
		return "eSubDiv";
		break;
	case fbxsdk::FbxNodeAttribute::eCachedEffect:
		return "eCachedEffect";
		break;
	case fbxsdk::FbxNodeAttribute::eLine:
		return "eLine";
		break;
	}

	return "";
}


void PrintNode(FbxNode* node, int hierarchy)
{
	if (node == nullptr)
	{
		return;
	}
	const char* name = node->GetName();

	for (int i = 0; i < hierarchy; i++)
	{
		printf("\t");
	}
	printf("name�F%s\n", name);
	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		for (int j = 0; j < hierarchy; j++)
		{
			printf("\t");
		}
		printf("\tAttribute %s\n", GetNodeAttributeName(node->GetNodeAttributeByIndex(i)->GetAttributeType()));
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		PrintNode(node->GetChild(i), hierarchy + 1);
	}
}

//FBX�m�[�h�̏���
void ProcessNode(FbxNode* node) {
	FbxMesh* mesh = node->GetMesh();
	if (mesh) {
		ProcessMesh(mesh);
	}
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i) {
		ProcessNode(node->GetChild(i));
	}
}

//FBX���b�V���̏���
void ProcessMesh(FbxMesh* mesh) {
	int vertexCount = mesh->GetControlPointsCount();
	FbxVector4* ctrlPoints = mesh->GetControlPoints();

	// ���_�ʒu
	for (int i = 0; i < vertexCount; ++i) {
		float x = (float)ctrlPoints[i][0];
		float y = (float)ctrlPoints[i][1];
		float z = (float)ctrlPoints[i][2];
		
	}

	// ���_�C���f�b�N�X
	for (int i = 0; i < mesh->GetPolygonCount(); ++i) {
		for (int j = 0; j < 3; ++j) {
			int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
			//index array�ɓ����
		}
	}
}


int InitFBXModelTest() 
{
	FbxManager* fbx_manager = nullptr;
	FbxIOSettings* fbx_iosetting = nullptr;
	FbxScene* fbx_scene = nullptr;
	FbxImporter* fbx_importer = nullptr;

	// FBX�̃}�l�[�W���[
	fbx_manager = FbxManager::Create();
	if (fbx_manager == nullptr)
	{
		return 0;
	}
	FbxIOSettings* iosetting = FbxIOSettings::Create(fbx_manager, IOSROOT);
	fbx_manager->SetIOSettings(iosetting);


	// FBX�̃}�l�[�W���[�ƃC���|�[�^�[�̐���
	fbx_scene = FbxScene::Create(fbx_manager, "");
	if (fbx_scene == nullptr)
	{
		Destroy(&fbx_manager, &fbx_iosetting, &fbx_scene, &fbx_importer);
		return 0;
	}

	// �t�@�C���̃C���|�[�g
	fbx_importer = FbxImporter::Create(fbx_manager, "");
	if (fbx_importer == nullptr)
	{
		Destroy(&fbx_manager, &fbx_iosetting, &fbx_scene, &fbx_importer);
		return 0;
	}

	fbx_importer->Initialize("data\MODEL\model.fbx");
	//fbx_importer->Initialize("unitychan.fbx");
	fbx_importer->Import(fbx_scene);

	FbxNode* root_node = fbx_scene->GetRootNode();

	int count = fbx_scene->GetNodeCount();

	if (root_node == nullptr)
	{
		Destroy(&fbx_manager, &fbx_iosetting, &fbx_scene, &fbx_importer);
		return 0;
	}

	PrintNode(root_node, 0);

	Destroy(&fbx_manager, &fbx_iosetting, &fbx_scene, &fbx_importer);

	return 0;
}

//-------------------------------------------------------------------------

static FBXTESTMODEL g_FBXTestModel;	// FBX���f���̃f�[�^

HRESULT InitFBXTestModel(void)
{
	g_FBXTestModel.load = TRUE;
	LoadFBXModel("data/MODEL/model.fbx", &g_FBXTestModel.model);

	//FBXTEST
	//LoadFBXModel("data/MODEL/model.fbx", &g_FBXTestModel.model);


	g_FBXTestModel.pos = XMFLOAT3(-10.0f,  10.0f, -50.0f);
	g_FBXTestModel.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_FBXTestModel.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_FBXTestModel.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_FBXTestModel.alive = TRUE;			// TRUE:�����Ă�
	

	return S_OK;
}

void UninitFBXTestModel(void)
{
	// ���f���̉������
	if (g_FBXTestModel.load == TRUE)
	{
		UnloadModel(&g_FBXTestModel.model);
		g_FBXTestModel.load = FALSE;
	}

}

void UpdateFBXTestModel(void)
{

}

void DrawFBXTestModel(void)
{
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


	// �����̐ݒ�
	SetFuchi(1);

	// ���f���`��
	DrawModel(&g_FBXTestModel.model);



	SetFuchi(0);

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






