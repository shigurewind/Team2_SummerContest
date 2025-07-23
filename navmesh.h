#pragma once
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

struct NavNode {
    XMFLOAT3 pos;
    std::vector<int> neighbors;
};

std::vector<NavNode> GenerateNavMeshFromFloorTriangles(float maxNeighborDistance = 100.0f);

bool FindPathAStar(
	const XMFLOAT3& start,
	const XMFLOAT3& goal,
	const std::vector<NavNode>& navNodes,
	std::vector<XMFLOAT3>& outPath
);