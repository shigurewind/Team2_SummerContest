#define NOMINMAX 

#include "Octree.h"

#include <algorithm>
#include "FBXmodel.h"

using namespace DirectX;

static void CalcTriangleBounds(const TriangleData& tri, XMFLOAT3& minOut, XMFLOAT3& maxOut)
{
	minOut.x = std::min(std::min(tri.v0.x, tri.v1.x), tri.v2.x);
	minOut.y = std::min(std::min(tri.v0.y, tri.v1.y), tri.v2.y);
	minOut.z = std::min(std::min(tri.v0.z, tri.v1.z), tri.v2.z);

	maxOut.x = std::max(std::max(tri.v0.x, tri.v1.x), tri.v2.x); 
	maxOut.y = std::max(std::max(tri.v0.y, tri.v1.y), tri.v2.y);
	maxOut.z = std::max(std::max(tri.v0.z, tri.v1.z), tri.v2.z);
}

static bool TriangleInBox(const TriangleData& tri, const XMFLOAT3& boxMin, const XMFLOAT3& boxMax)
{
	XMFLOAT3 triMin, triMax;
	CalcTriangleBounds(tri, triMin, triMax);

	return !(triMax.x < boxMin.x || triMin.x > boxMax.x ||
		triMax.y < boxMin.y || triMin.y > boxMax.y ||
		triMax.z < boxMin.z || triMin.z > boxMax.z);
}

static bool RayIntersectAABB(XMVECTOR rayOrigin, XMVECTOR rayDir, const XMFLOAT3& boxMin, const XMFLOAT3& boxMax)
{
	XMVECTOR min = XMLoadFloat3(&boxMin);
	XMVECTOR max = XMLoadFloat3(&boxMax);

	XMVECTOR invDir = XMVectorReciprocal(rayDir);
	XMVECTOR t1 = XMVectorMultiply(XMVectorSubtract(min, rayOrigin), invDir);
	XMVECTOR t2 = XMVectorMultiply(XMVectorSubtract(max, rayOrigin), invDir);

	XMVECTOR tmin = XMVectorMin(t1, t2);
	XMVECTOR tmax = XMVectorMax(t1, t2);

	float tNear = std::max({ XMVectorGetX(tmin), XMVectorGetY(tmin), XMVectorGetZ(tmin) });
	float tFar = std::min({ XMVectorGetX(tmax), XMVectorGetY(tmax), XMVectorGetZ(tmax) });

	return tNear <= tFar && tFar > 0.0f;
}

bool TriangleRayIntersect(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* outDist)
{
	const float EPSILON = 1e-6f;
	XMVECTOR edge1 = XMVectorSubtract(v1, v0);
	XMVECTOR edge2 = XMVectorSubtract(v2, v0);

	XMVECTOR h = XMVector3Cross(rayDir, edge2);
	float a = XMVectorGetX(XMVector3Dot(edge1, h));
	if (fabs(a) < EPSILON) return false;

	float f = 1.0f / a;
	XMVECTOR s = XMVectorSubtract(rayOrigin, v0);
	float u = f * XMVectorGetX(XMVector3Dot(s, h));
	if (u < 0.0f || u > 1.0f) return false;

	XMVECTOR q = XMVector3Cross(s, edge1);
	float v = f * XMVectorGetX(XMVector3Dot(rayDir, q));
	if (v < 0.0f || u + v > 1.0f) return false;

	float t = f * XMVectorGetX(XMVector3Dot(edge2, q));
	if (t > EPSILON) {
		*outDist = t;
		return true;
	}

	return false;
}

OctreeNode* BuildOctree(const std::vector<TriangleData>& triangleList, const XMFLOAT3& minBound, const XMFLOAT3& maxBound, int depth, int maxDepth, int minTri)
{
	OctreeNode* node = new OctreeNode;
	node->minBound = minBound;
	node->maxBound = maxBound;

	char buf[256];
	sprintf_s(buf, "Building octree: depth=%d\n", depth);
	OutputDebugStringA(buf);

	for (const TriangleData& tri : triangleList) {
		if (TriangleInBox(tri, minBound, maxBound)) {
			node->triangles.push_back(tri);
		}
	}

	if (depth >= maxDepth || node->triangles.size() <= minTri) return node;

	XMFLOAT3 center = {
		(minBound.x + maxBound.x) * 0.5f,
		(minBound.y + maxBound.y) * 0.5f,
		(minBound.z + maxBound.z) * 0.5f
	};

	for (int i = 0; i < 8; i++) {
		XMFLOAT3 cmin = minBound, cmax = center;
		if (i & 1) { cmin.x = center.x; cmax.x = maxBound.x; }
		if (i & 2) { cmin.y = center.y; cmax.y = maxBound.y; }
		if (i & 4) { cmin.z = center.z; cmax.z = maxBound.z; }
		node->children[i] = BuildOctree(triangleList, cmin, cmax, depth + 1, maxDepth, minTri);
	}

	return node;
}

bool RayHitOctree(OctreeNode* node, const XMFLOAT3& origin, const XMFLOAT3& dir, float* closestDist, XMFLOAT3* hitPos, XMFLOAT3* hitNormal)
{
	XMVECTOR rayOrigin = XMLoadFloat3(&origin);
	XMVECTOR rayDir = XMVector3Normalize(XMLoadFloat3(&dir)); 

	if (!RayIntersectAABB(rayOrigin, rayDir, node->minBound, node->maxBound))
		return false;

	bool hit = false;
	float minDist = *closestDist;

	for (const TriangleData& tri : node->triangles) {
		float dist;
		if (TriangleRayIntersect(
			rayOrigin, rayDir,
			XMLoadFloat3(&tri.v0),
			XMLoadFloat3(&tri.v1),
			XMLoadFloat3(&tri.v2),
			&dist)) {

			if (dist < minDist && dist > 0.0f) {
				minDist = dist;

				XMVECTOR hitPoint = XMVectorAdd(rayOrigin, XMVectorScale(rayDir, dist));
				XMStoreFloat3(hitPos, hitPoint);

				XMVECTOR v0 = XMLoadFloat3(&tri.v0);
				XMVECTOR v1 = XMLoadFloat3(&tri.v1);
				XMVECTOR v2 = XMLoadFloat3(&tri.v2);
				XMVECTOR edge1 = XMVectorSubtract(v1, v0);
				XMVECTOR edge2 = XMVectorSubtract(v2, v0);
				XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge1, edge2));

				if (XMVectorGetX(XMVector3Dot(rayDir, normal)) > 0) {
					normal = XMVectorNegate(normal);
				}

				XMStoreFloat3(hitNormal, normal);
				hit = true;
			}
		}
	}

	if (!node->IsLeaf()) {
		for (int i = 0; i < 8; ++i) {
			if (!node->children[i]) continue;

			XMFLOAT3 childHitPos;
			XMFLOAT3 childHitNormal;
			float childMinDist = minDist; 

			if (RayHitOctree(node->children[i], origin, dir, &childMinDist, &childHitPos, &childHitNormal)) {
				if (childMinDist < minDist) {
					minDist = childMinDist;
					*hitPos = childHitPos;
					*hitNormal = childHitNormal;
					hit = true;
				}
			}
		}
	}

	if (hit) {
		*closestDist = minDist;
	}

	return hit;
}
void DeleteOctree(OctreeNode* node)
{
	if (!node) return;

	
	node->triangles.clear();

	for (int i = 0; i < 8; i++) {
		DeleteOctree(node->children[i]);
		node->children[i] = nullptr;
	}

	delete node;
}

bool AABBvsTriangle(const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	XMFLOAT3 triMin, triMax;
	triMin.x = std::min({ v0.x, v1.x, v2.x });
	triMin.y = std::min({ v0.y, v1.y, v2.y });
	triMin.z = std::min({ v0.z, v1.z, v2.z });

	triMax.x = std::max({ v0.x, v1.x, v2.x });
	triMax.y = std::max({ v0.y, v1.y, v2.y });
	triMax.z = std::max({ v0.z, v1.z, v2.z });

	bool overlap =
		!(triMax.x < boxMin.x || triMin.x > boxMax.x ||
			triMax.y < boxMin.y || triMin.y > boxMax.y ||
			triMax.z < boxMin.z || triMin.z > boxMax.z);

	return overlap;
}

bool AABBHitOctree(OctreeNode* node, const XMFLOAT3& boxMin, const XMFLOAT3& boxMax)
{
	XMFLOAT3 nmin = node->minBound;
	XMFLOAT3 nmax = node->maxBound;

	bool overlap =
		!(nmax.x < boxMin.x || nmin.x > boxMax.x ||
			nmax.y < boxMin.y || nmin.y > boxMax.y ||
			nmax.z < boxMin.z || nmin.z > boxMax.z);

	if (!overlap) return false;

	for (const TriangleData& tri : node->triangles) {
		if (AABBvsTriangle(boxMin, boxMax, tri.v0, tri.v1, tri.v2)) {
			return true;
		}
	}

	if (!node->IsLeaf()) {
		for (int i = 0; i < 8; i++) {
			if (AABBHitOctree(node->children[i], boxMin, boxMax))
				return true;
		}
	}

	return false;
}