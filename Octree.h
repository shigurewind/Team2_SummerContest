#pragma once
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;


enum TriangleType {
	TYPE_UNKNOWN = -1,
	TYPE_FLOOR = 0,
	TYPE_WALL = 1
};


struct TriangleData {
	XMFLOAT3 v0, v1, v2;
	XMFLOAT3 normal;
	TriangleType type;
};

struct OctreeNode
{
	XMFLOAT3 minBound;
	XMFLOAT3 maxBound;


	OctreeNode* children[8] = { nullptr };

	bool IsLeaf() const {
		for (int i = 0; i < 8; i++) {
			if (children[i]) return false;
		}
		return true;
	}
	std::vector<int> triangleIndices; 
	bool isSubdivided = false;

};
OctreeNode* BuildOctree(const std::vector<TriangleData>& triangleList, const XMFLOAT3& minBound, const XMFLOAT3& maxBound, int depth, int maxDepth, int minTri);
bool RayHitOctree(OctreeNode* node, const std::vector<TriangleData>& triangleList,
	const XMFLOAT3& origin, const XMFLOAT3& dir,
	float* closestDist, XMFLOAT3* hitPos, XMFLOAT3* hitNormal,
	int depth, int maxDepth, int minTri);
bool AABBHitOctree(OctreeNode* node, const std::vector<TriangleData>& triangleList,
	const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	int depth, int maxDepth, int minTri);
void Subdivide(OctreeNode* node, const std::vector<TriangleData>& triangleList, int depth, int maxDepth, int minTri);
