#define NOMINMAX 

#include "Octree.h"

#include <algorithm>
#include "FBXmodel.h"

using namespace DirectX;

static thread_local float    g_AABBTriLastDepth = 0.0f;
static thread_local XMFLOAT3 g_AABBTriLastNormal = { 0,0,0 };


float GetAABBvsTriangleLastDepth() { return g_AABBTriLastDepth; }
XMFLOAT3 GetAABBvsTriangleLastNormal() { return g_AABBTriLastNormal; }

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

OctreeNode* BuildOctree(const std::vector<TriangleData>& triangleList, const XMFLOAT3& minBound, const XMFLOAT3& maxBound, int depth, int maxDepth,	int minTri)
{
	OctreeNode* node = new OctreeNode;
	node->minBound = minBound;
	node->maxBound = maxBound;
	node->isSubdivided = false;

	//今のノードに含まれる三角形を調べる
	for (int i = 0; i < triangleList.size(); ++i) {
		if (TriangleInBox(triangleList[i], minBound, maxBound)) {
			node->triangleIndices.push_back(i);
		}
	}

	//今分割する(メモリが多くなるが、スピードが上がる)
	if (depth < maxDepth && node->triangleIndices.size() > minTri) {
		//プレビルド
		Subdivide(node, triangleList, depth, maxDepth, minTri);
	}
	

	return node;
}

bool RayHitOctree(OctreeNode* node, const std::vector<TriangleData>& triangleList,
	const XMFLOAT3& origin, const XMFLOAT3& dir,
	float* closestDist, XMFLOAT3* hitPos, XMFLOAT3* hitNormal,
	int depth, int maxDepth, int minTri) 
{
	XMVECTOR rayOrigin = XMLoadFloat3(&origin);
	XMVECTOR rayDir = XMVector3Normalize(XMLoadFloat3(&dir));

	if (!RayIntersectAABB(rayOrigin, rayDir, node->minBound, node->maxBound))
		return false;

	

	bool hit = false;
	float minDist = *closestDist;

	for (int idx : node->triangleIndices) {
		const TriangleData& tri = triangleList[idx];
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

	if (node->isSubdivided) {
		for (int i = 0; i < 8; ++i) {
			if (!node->children[i]) continue;

			XMFLOAT3 childHitPos;
			XMFLOAT3 childHitNormal;
			float childMinDist = minDist;

			if (RayHitOctree(node->children[i], triangleList, origin, dir,
				&childMinDist, &childHitPos, &childHitNormal,
				depth + 1, maxDepth, minTri)) {

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



	for (int i = 0; i < 8; i++) {
		DeleteOctree(node->children[i]);
		node->children[i] = nullptr;
	}

	delete node;
}

static inline bool TriBoxOverlap(const XMFLOAT3& boxCenter, const XMFLOAT3& boxHalf,
	const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	XMFLOAT3 tv0 = { v0.x - boxCenter.x, v0.y - boxCenter.y, v0.z - boxCenter.z };
	XMFLOAT3 tv1 = { v1.x - boxCenter.x, v1.y - boxCenter.y, v1.z - boxCenter.z };
	XMFLOAT3 tv2 = { v2.x - boxCenter.x, v2.y - boxCenter.y, v2.z - boxCenter.z };

	XMFLOAT3 e0 = { tv1.x - tv0.x, tv1.y - tv0.y, tv1.z - tv0.z };
	XMFLOAT3 e1 = { tv2.x - tv1.x, tv2.y - tv1.y, tv2.z - tv1.z };
	XMFLOAT3 e2 = { tv0.x - tv2.x, tv0.y - tv2.y, tv0.z - tv2.z };

	auto AXISTEST = [&](float a, float b, float fa, float fb, float v0a, float v0b, float v1a, float v1b, float v2a, float v2b, float ha, float hb)->bool {
		float p0 = a * v0a - b * v0b;
		float p1 = a * v1a - b * v1b;
		float p2 = a * v2a - b * v2b;
		float min = std::min(p0, std::min(p1, p2));
		float max = std::max(p0, std::max(p1, p2));
		float rad = fa * ha + fb * hb;
		return !(min > rad || max < -rad);
		};

	float fe0x = fabsf(e0.x), fe0y = fabsf(e0.y), fe0z = fabsf(e0.z);
	float fe1x = fabsf(e1.x), fe1y = fabsf(e1.y), fe1z = fabsf(e1.z);
	float fe2x = fabsf(e2.x), fe2y = fabsf(e2.y), fe2z = fabsf(e2.z);

	if (!AXISTEST(e0.z, e0.y, fe0z, fe0y, tv0.y, tv0.z, tv1.y, tv1.z, tv2.y, tv2.z, boxHalf.y, boxHalf.z)) return false; // X
	if (!AXISTEST(e0.z, e0.x, fe0z, fe0x, tv0.x, tv0.z, tv1.x, tv1.z, tv2.x, tv2.z, boxHalf.x, boxHalf.z)) return false; // Y
	if (!AXISTEST(e0.y, e0.x, fe0y, fe0x, tv0.x, tv0.y, tv1.x, tv1.y, tv2.x, tv2.y, boxHalf.x, boxHalf.y)) return false; // Z

	if (!AXISTEST(e1.z, e1.y, fe1z, fe1y, tv0.y, tv0.z, tv1.y, tv1.z, tv2.y, tv2.z, boxHalf.y, boxHalf.z)) return false;
	if (!AXISTEST(e1.z, e1.x, fe1z, fe1x, tv0.x, tv0.z, tv1.x, tv1.z, tv2.x, tv2.z, boxHalf.x, boxHalf.z)) return false;
	if (!AXISTEST(e1.y, e1.x, fe1y, fe1x, tv0.x, tv0.y, tv1.x, tv1.y, tv2.x, tv2.y, boxHalf.x, boxHalf.y)) return false;

	if (!AXISTEST(e2.z, e2.y, fe2z, fe2y, tv0.y, tv0.z, tv1.y, tv1.z, tv2.y, tv2.z, boxHalf.y, boxHalf.z)) return false;
	if (!AXISTEST(e2.z, e2.x, fe2z, fe2x, tv0.x, tv0.z, tv1.x, tv1.z, tv2.x, tv2.z, boxHalf.x, boxHalf.z)) return false;
	if (!AXISTEST(e2.y, e2.x, fe2y, fe2x, tv0.x, tv0.y, tv1.x, tv1.y, tv2.x, tv2.y, boxHalf.x, boxHalf.y)) return false;

	auto FINDMINMAX = [](float a, float b, float c, float& minv, float& maxv) {
		minv = std::min(a, std::min(b, c));
		maxv = std::max(a, std::max(b, c));
		};
	float minv, maxv;
	FINDMINMAX(tv0.x, tv1.x, tv2.x, minv, maxv); if (minv > boxHalf.x || maxv < -boxHalf.x) return false;
	FINDMINMAX(tv0.y, tv1.y, tv2.y, minv, maxv); if (minv > boxHalf.y || maxv < -boxHalf.y) return false;
	FINDMINMAX(tv0.z, tv1.z, tv2.z, minv, maxv); if (minv > boxHalf.z || maxv < -boxHalf.z) return false;

	XMFLOAT3 n = {
		e0.y * e1.z - e0.z * e1.y,
		e0.z * e1.x - e0.x * e1.z,
		e0.x * e1.y - e0.y * e1.x
	};
	float r = boxHalf.x * fabsf(n.x) + boxHalf.y * fabsf(n.y) + boxHalf.z * fabsf(n.z);
	float s = tv0.x * n.x + tv0.y * n.y + tv0.z * n.z; 
	return !(s > r || s < -r);
}

bool AABBvsTriangle(const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	XMFLOAT3 center = { (boxMin.x + boxMax.x) * 0.5f,
						(boxMin.y + boxMax.y) * 0.5f,
						(boxMin.z + boxMax.z) * 0.5f };
	XMFLOAT3 half = { (boxMax.x - boxMin.x) * 0.5f,
						(boxMax.y - boxMin.y) * 0.5f,
						(boxMax.z - boxMin.z) * 0.5f };

	bool overlap = TriBoxOverlap(center, half, v0, v1, v2);

	using namespace DirectX;
	XMVECTOR V0 = XMLoadFloat3(&v0);
	XMVECTOR V1 = XMLoadFloat3(&v1);
	XMVECTOR V2 = XMLoadFloat3(&v2);

	XMVECTOR N = XMVector3Normalize(
		XMVector3Cross(XMVectorSubtract(V1, V0),
			XMVectorSubtract(V2, V1)));
	XMStoreFloat3(&g_AABBTriLastNormal, N);

	XMFLOAT3 n; XMStoreFloat3(&n, N);
	XMFLOAT3 vc = { v0.x - center.x, v0.y - center.y, v0.z - center.z };
	float r = half.x * fabsf(n.x) + half.y * fabsf(n.y) + half.z * fabsf(n.z);
	float s = vc.x * n.x + vc.y * n.y + vc.z * n.z;
	float pen = r - fabsf(s);

	g_AABBTriLastDepth = (overlap && pen > 0.0f) ? pen : 0.0f;

	return overlap;
}


bool AABBHitOctree(OctreeNode* node, const std::vector<TriangleData>& triangleList,
	const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	int depth, int maxDepth, int minTri)
{
	XMFLOAT3 nmin = node->minBound;
	XMFLOAT3 nmax = node->maxBound;

	bool overlap =
		!(nmax.x < boxMin.x || nmin.x > boxMax.x ||
			nmax.y < boxMin.y || nmin.y > boxMax.y ||
			nmax.z < boxMin.z || nmin.z > boxMax.z);

	if (!overlap) return false;

	

	for (int idx : node->triangleIndices) {
		const TriangleData& tri = triangleList[idx];
		if (AABBvsTriangle(boxMin, boxMax, tri.v0, tri.v1, tri.v2)) {
			return true;
		}
	}

	if (node->isSubdivided) {
		for (int i = 0; i < 8; i++) {
			if (!node->children[i]) continue;
			if (AABBHitOctree(node->children[i], triangleList, boxMin, boxMax,
				depth + 1, maxDepth, minTri)) {
				return true;
			}
		}
	}

	return false;
}

void Subdivide(OctreeNode* node, const std::vector<TriangleData>& triangleList, int depth, int maxDepth, int minTri)
{
	if (node->isSubdivided || depth >= maxDepth || node->triangleIndices.size() <= minTri)
		return;

	node->isSubdivided = true;

	XMFLOAT3 center = {
		(node->minBound.x + node->maxBound.x) * 0.5f,
		(node->minBound.y + node->maxBound.y) * 0.5f,
		(node->minBound.z + node->maxBound.z) * 0.5f
	};

	for (int i = 0; i < 8; i++) {
		XMFLOAT3 cmin = node->minBound, cmax = center;
		if (i & 1) { cmin.x = center.x; cmax.x = node->maxBound.x; }
		if (i & 2) { cmin.y = center.y; cmax.y = node->maxBound.y; }
		if (i & 4) { cmin.z = center.z; cmax.z = node->maxBound.z; }

		OctreeNode* child = new OctreeNode;
		child->minBound = cmin;
		child->maxBound = cmax;

		for (int idx : node->triangleIndices) {
			if (TriangleInBox(triangleList[idx], cmin, cmax)) {
				child->triangleIndices.push_back(idx);
			}
		}

		node->children[i] = child;

		// 再帰的に分割
		Subdivide(child, triangleList, depth + 1, maxDepth, minTri);
	}

	//node->triangleIndices.clear(); 
}



bool RayHitOctreeLOD(OctreeNode* node, const std::vector<TriangleData>& triangleList,
	const XMFLOAT3& origin, const XMFLOAT3& dir,
	float* closestDist, XMFLOAT3* hitPos, XMFLOAT3* hitNormal,
	int depth, int maxDepth, int minTri, int lodLevel)
{
	XMVECTOR rayOrigin = XMLoadFloat3(&origin);
	XMVECTOR rayDir = XMVector3Normalize(XMLoadFloat3(&dir));

	if (!RayIntersectAABB(rayOrigin, rayDir, node->minBound, node->maxBound))
		return false;

	bool hit = false;
	float minDist = *closestDist;

	// 距離によってLODレベルを調整
	for (int i = 0; i < node->triangleIndices.size(); i += lodLevel) {
		int idx = node->triangleIndices[i];
		const TriangleData& tri = triangleList[idx];
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

	// 子ノードも再帰チェック
	if (node->isSubdivided) {
		for (int i = 0; i < 8; ++i) {
			if (!node->children[i]) continue;

			XMFLOAT3 childHitPos;
			XMFLOAT3 childHitNormal;
			float childMinDist = minDist;

			if (RayHitOctreeLOD(node->children[i], triangleList, origin, dir,
				&childMinDist, &childHitPos, &childHitNormal,
				depth + 1, maxDepth, minTri, lodLevel)) {

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


bool AABBHitOctreeLOD(OctreeNode* node, const std::vector<TriangleData>& triangleList,
	const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	int depth, int maxDepth, int minTri, int lodLevel)
{
	XMFLOAT3 nmin = node->minBound;
	XMFLOAT3 nmax = node->maxBound;

	bool overlap =
		!(nmax.x < boxMin.x || nmin.x > boxMax.x ||
			nmax.y < boxMin.y || nmin.y > boxMax.y ||
			nmax.z < boxMin.z || nmin.z > boxMax.z);

	if (!overlap) return false;

	// 距離によってLODレベルを調整
	for (int i = 0; i < node->triangleIndices.size(); i += lodLevel) {
		int idx = node->triangleIndices[i];
		const TriangleData& tri = triangleList[idx];
		if (AABBvsTriangle(boxMin, boxMax, tri.v0, tri.v1, tri.v2)) {
			return true;
		}
	}

	if (node->isSubdivided) {
		for (int i = 0; i < 8; i++) {
			if (!node->children[i]) continue;
			if (AABBHitOctreeLOD(node->children[i], triangleList, boxMin, boxMax,
				depth + 1, maxDepth, minTri, lodLevel)) {
				return true;
			}
		}
	}

	return false;
}
