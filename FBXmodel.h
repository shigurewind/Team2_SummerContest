#pragma once

#include <fbxsdk.h>






void Destroy(FbxManager** manager, FbxIOSettings** iosetting, FbxScene** scene, FbxImporter** importer);
const char* GetNodeAttributeName(FbxNodeAttribute::EType attribute);
void PrintNode(FbxNode* node, int hierarchy);


