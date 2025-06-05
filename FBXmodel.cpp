#include "FBXmodel.h"


//FBXの破棄
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
	printf("name：%s\n", name);
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


int InitFBXModelTest() 
{
	FbxManager* fbx_manager = nullptr;
	FbxIOSettings* fbx_iosetting = nullptr;
	FbxScene* fbx_scene = nullptr;
	FbxImporter* fbx_importer = nullptr;

	// FBXのマネージャー
	fbx_manager = FbxManager::Create();
	if (fbx_manager == nullptr)
	{
		return 0;
	}

	// FBXのマネージャーとインポーターの生成
	fbx_scene = FbxScene::Create(fbx_manager, "");
	if (fbx_scene == nullptr)
	{
		Destroy(&fbx_manager, &fbx_iosetting, &fbx_scene, &fbx_importer);
		return 0;
	}

	// ファイルのインポート
	fbx_importer = FbxImporter::Create(fbx_manager, "");
	if (fbx_importer == nullptr)
	{
		Destroy(&fbx_manager, &fbx_iosetting, &fbx_scene, &fbx_importer);
		return 0;
	}

	fbx_importer->Initialize("hand.fbx");
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