#pragma once
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;
struct TriangleData;
struct OctreeNode
{
	XMFLOAT3 minBound;
	XMFLOAT3 maxBound;

	std::vector<const TriangleData*> triangles; 

	OctreeNode* children[8] = { nullptr };

	bool IsLeaf() const {
		return children[0] == nullptr;
	}
};
OctreeNode* BuildOctree(const std::vector<TriangleData>& triangleList, const XMFLOAT3& minBound, const XMFLOAT3& maxBound, int depth, int maxDepth, int minTri);
bool RayHitOctree(OctreeNode* node,const XMFLOAT3& origin,const XMFLOAT3& dir,float* closestDist,XMFLOAT3* hitPos, XMFLOAT3* hitNormal);
bool AABBHitOctree(OctreeNode* node, const XMFLOAT3& boxMin, const XMFLOAT3& boxMax);

