#pragma once

#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

struct NavTriangle {
	XMFLOAT3 v0, v1, v2;
	XMFLOAT3 normal;
};

// 模型からNavMeshを初期化（worldMatrixはモデルのワールド変換行列）
void InitNavMeshFromModel(struct AMODEL* model, const XMMATRIX& worldMatrix);

// 指定したXZ座標におけるNavMeshの高さを取得（Y座標）
// 戻り値: 成功時 true、失敗時 false。outYにY値を格納
bool GetNavMeshHeight(float x, float z, float* outY = nullptr);

// 指定したXZ座標がNavMesh内にあるかどうか
bool IsPointOnNavMesh(float x, float z);