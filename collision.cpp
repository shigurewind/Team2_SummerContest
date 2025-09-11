//=============================================================================
//
// 当たり判定処理 [collision.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "collision.h"
#include "FBXmodel.h"
#include "Octree.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// 構造体定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static inline bool OverlapAABB(const XMFLOAT3& aMin, const XMFLOAT3& aMax,
	const XMFLOAT3& bMin, const XMFLOAT3& bMax)
{
	return !(aMax.x < bMin.x || aMin.x > bMax.x ||
		aMax.y < bMin.y || aMin.y > bMax.y ||
		aMax.z < bMin.z || aMin.z > bMax.z);
}


static void CollectTriIndicesLOD(OctreeNode* node,
	const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	std::vector<int>& out, int lodLevel)
{
	if (!node) return;
	if (!OverlapAABB(node->minBound, node->maxBound, boxMin, boxMax)) return;

	if (lodLevel <= 1) {
		out.insert(out.end(), node->triangleIndices.begin(), node->triangleIndices.end());
	}
	else {
		for (int i = 0; i < (int)node->triangleIndices.size(); i += lodLevel) {
			out.push_back(node->triangleIndices[i]);
		}
	}

	if (node->isSubdivided) {
		for (int i = 0; i < 8; ++i) {
			if (node->children[i]) {
				CollectTriIndicesLOD(node->children[i], boxMin, boxMax, out, lodLevel);
			}
		}
	}
}

static constexpr float kContactSkin = 2.0f;  //
static constexpr float kEpsPush = 0.01f; 

//=============================================================================
// BBによる当たり判定処理
// 回転は考慮しない
// 戻り値：当たってたらTRUE
//=============================================================================
BOOL CollisionBB(XMFLOAT3 mpos, float mw, float mh,
	XMFLOAT3 ypos, float yw, float yh)
{
	BOOL ans = FALSE;	// 外れをセットしておく

	// 座標が中心点なので計算しやすく半分にしている
	mw /= 2;
	mh /= 2;
	yw /= 2;
	yh /= 2;

	// バウンディングボックス(BB)の処理
	if ((mpos.x + mw > ypos.x - yw) &&
		(mpos.x - mw < ypos.x + yw) &&
		(mpos.y + mh > ypos.y - yh) &&
		(mpos.y - mh < ypos.y + yh))
	{
		// 当たった時の処理
		ans = TRUE;
	}

	return ans;
}

//=============================================================================
// BCによる当たり判定処理
// サイズは半径
// 戻り値：当たってたらTRUE
//=============================================================================
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2)
{
	BOOL ans = FALSE;						// 外れをセットしておく

	float len = (r1 + r2) * (r1 + r2);		// 半径を2乗した物
	XMVECTOR temp = XMLoadFloat3(&pos1) - XMLoadFloat3(&pos2);
	temp = XMVector3LengthSq(temp);			// 2点間の距離（2乗した物）
	float lenSq = 0.0f;
	XMStoreFloat(&lenSq, temp);

	// 半径を2乗した物より距離が短い？
	if (len > lenSq)
	{
		ans = TRUE;	// 当たっている
	}

	return ans;
}


//=============================================================================
// 内積(dot)
//=============================================================================
float dotProduct(XMVECTOR* v1, XMVECTOR* v2)
{
#if 0
	float ans = v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
#else
	// ダイレクトＸでは、、、
	XMVECTOR temp = XMVector3Dot(*v1, *v2);
	float ans = 0.0f;
	XMStoreFloat(&ans, temp);
#endif

	return(ans);
}


//=============================================================================
// 外積(cross)
//=============================================================================
void crossProduct(XMVECTOR* ret, XMVECTOR* v1, XMVECTOR* v2)
{
#if 0
	ret->x = v1->y * v2->z - v1->z * v2->y;
	ret->y = v1->z * v2->x - v1->x * v2->z;
	ret->z = v1->x * v2->y - v1->y * v2->x;
#else
	// ダイレクトＸでは、、、
	* ret = XMVector3Cross(*v1, *v2);
#endif

}


//=============================================================================
// レイキャスト
// p0, p1, p2　ポリゴンの３頂点
// pos0 始点
// pos1 終点
// hit　交点の返却用
// normal 法線ベクトルの返却用
// 当たっている場合、TRUEを返す
//=============================================================================
BOOL RayCast(XMFLOAT3 xp0, XMFLOAT3 xp1, XMFLOAT3 xp2, XMFLOAT3 xpos0, XMFLOAT3 xpos1, XMFLOAT3* hit, XMFLOAT3* normal)
{
	XMVECTOR	p0 = XMLoadFloat3(&xp0);
	XMVECTOR	p1 = XMLoadFloat3(&xp1);
	XMVECTOR	p2 = XMLoadFloat3(&xp2);
	XMVECTOR	pos0 = XMLoadFloat3(&xpos0);
	XMVECTOR	pos1 = XMLoadFloat3(&xpos1);

	XMVECTOR	nor;	// ポリゴンの法線
	XMVECTOR	vec1;
	XMVECTOR	vec2;
	float		d1, d2;

	{	// ポリゴンの外積をとって法線を求める(この処理は固定物なら予めInit()で行っておくと良い)
		vec1 = p1 - p0;
		vec2 = p2 - p0;
		crossProduct(&nor, &vec2, &vec1);
		nor = XMVector3Normalize(nor);		// 計算しやすいように法線をノーマライズしておく(このベクトルの長さを１にしている)
		XMStoreFloat3(normal, nor);			// 求めた法線を入れておく
	}

	// ポリゴン平面と線分の内積とって衝突している可能性を調べる（鋭角なら＋、鈍角ならー、直角なら０）
	vec1 = pos0 - p0;
	vec2 = pos1 - p0;
	{	// 求めたポリゴンの法線と２つのベクトル（線分の両端とポリゴン上の任意の点）の内積とって衝突している可能性を調べる
		d1 = dotProduct(&vec1, &nor);
		d2 = dotProduct(&vec2, &nor);
		if (((d1 * d2) > 0.0f) || (d1 == 0 && d2 == 0))
		{
			// 当たっている可能性は無い
			return(FALSE);
		}
	}


	{	// ポリゴンと線分の交点を求める
		d1 = (float)fabs(d1);	// 絶対値を求めている
		d2 = (float)fabs(d2);	// 絶対値を求めている
		float a = d1 / (d1 + d2);							// 内分比

		XMVECTOR	vec3 = (1 - a) * vec1 + a * vec2;		// p0から交点へのベクトル
		XMVECTOR	p3 = p0 + vec3;							// 交点
		XMStoreFloat3(hit, p3);								// 求めた交点を入れておく

		{	// 求めた交点がポリゴンの中にあるか調べる

			// ポリゴンの各辺のベクトル
			XMVECTOR	v1 = p1 - p0;
			XMVECTOR	v2 = p2 - p1;
			XMVECTOR	v3 = p0 - p2;

			// 各頂点と交点とのベクトル
			XMVECTOR	v4 = p3 - p1;
			XMVECTOR	v5 = p3 - p2;
			XMVECTOR	v6 = p3 - p0;

			// 外積で各辺の法線を求めて、ポリゴンの法線との内積をとって符号をチェックする
			XMVECTOR	n1, n2, n3;

			crossProduct(&n1, &v4, &v1);
			if (dotProduct(&n1, &nor) < 0.0f) return(FALSE);	// 当たっていない

			crossProduct(&n2, &v5, &v2);
			if (dotProduct(&n2, &nor) < 0.0f) return(FALSE);	// 当たっていない

			crossProduct(&n3, &v6, &v3);
			if (dotProduct(&n3, &nor) < 0.0f) return(FALSE);	// 当たっていない
		}
	}

	return(TRUE);	// 当たっている！(hitには当たっている交点が入っている。normalには法線が入っている)
}


//=============================================================================
// BB+BC　の当たり判定
//=============================================================================
BOOL CheckSphereAABBCollision(XMFLOAT3 spherePos, float sphereRadius,
	XMFLOAT3 boxPos, XMFLOAT3 boxHalfSize)
{
	float x = max(boxPos.x - boxHalfSize.x, min(spherePos.x, boxPos.x + boxHalfSize.x));
	float y = max(boxPos.y - boxHalfSize.y, min(spherePos.y, boxPos.y + boxHalfSize.y));
	float z = max(boxPos.z - boxHalfSize.z, min(spherePos.z, boxPos.z + boxHalfSize.z));

	float dx = x - spherePos.x;
	float dy = y - spherePos.y;
	float dz = z - spherePos.z;

	return (dx * dx + dy * dy + dz * dz) <= (sphereRadius * sphereRadius);
}

bool CheckWallCollisionLODEx(const XMFLOAT3& boxMin, const XMFLOAT3& boxMax,
	WallHitInfo* outInfo, Object* obj)
{
	if (outInfo) {
		outInfo->hit = false;
		outInfo->normal = { 0,0,0 };
		outInfo->penetration = 0.0f;
	}

	int lodLevel = 1;
	if (obj) {
		XMFLOAT3 vel = obj->GetVelocity();
		float speed = sqrtf(vel.x * vel.x + vel.z * vel.z);
		if (speed > 4.0f)  lodLevel = 2;
		if (speed > 7.5f)  lodLevel = 3;
	}

	OctreeNode* root = GetWallTree(); 
	const auto& tris = GetWallTriangles(); 
	if (!root) return false;

	XMFLOAT3 qMin = { boxMin.x - kContactSkin, boxMin.y - kContactSkin, boxMin.z - kContactSkin };
	XMFLOAT3 qMax = { boxMax.x + kContactSkin, boxMax.y + kContactSkin, boxMax.z + kContactSkin };

	std::vector<int> candidates;
	candidates.reserve(128);
	CollectTriIndicesLOD(root, qMin, qMax, candidates, lodLevel);
	if (candidates.empty()) return false;

	bool anyHit = false;

	float bestScore = -1e9f;
	float bestDepth = 0.0f;
	XMFLOAT3 bestN = { 0,0,0 };

	XMFLOAT3 vel = obj ? obj->GetVelocity() : XMFLOAT3{ 0,0,0 };
	XMFLOAT2 velXZ = { vel.x, vel.z };
	float velLen = sqrtf(velXZ.x * velXZ.x + velXZ.y * velXZ.y);
	XMFLOAT2 velDir = { 0,0 };
	if (velLen > 1e-6f) { velDir.x = velXZ.x / velLen; velDir.y = velXZ.y / velLen; }

	for (int idx : candidates) {
		if (idx < 0 || idx >= (int)tris.size()) continue;
		const TriangleData& tri = tris[idx];

		if (tri.type != TYPE_WALL && tri.type != TYPE_UNKNOWN) {
			continue;
		}

		if (AABBvsTriangle(qMin, qMax, tri.v0, tri.v1, tri.v2)) {
			anyHit = true;

			float penN = GetAABBvsTriangleLastDepth();
			XMFLOAT3 triN = GetAABBvsTriangleLastNormal();

			XMFLOAT2 nXZ = { triN.x, triN.z };
			float nLen = sqrtf(nXZ.x * nXZ.x + nXZ.y * nXZ.y);
			if (nLen < 1e-6f) {
				continue;
			}
			nXZ.x /= nLen; nXZ.y /= nLen;

			if (velLen > 1e-6f) {
				float d = nXZ.x * velDir.x + nXZ.y * velDir.y;  
				if (d > 0.0f) { nXZ.x = -nXZ.x; nXZ.y = -nXZ.y; }
			}
			float faceScore = 0.0f;
			if (velLen > 1e-6f) {
				faceScore = -(velDir.x * nXZ.x + velDir.y * nXZ.y);
			}
			float score = faceScore * 1000.0f + penN;   

			if (score > bestScore) {
				bestScore = score;
				bestDepth = penN;
				bestN = XMFLOAT3{ nXZ.x, 0.0f, nXZ.y };
			}
		}
	}

	if (!anyHit) return false;

	if (outInfo) {
		outInfo->hit = true;
		outInfo->normal = bestN;         
		outInfo->penetration = bestDepth;
	}

	return true;
}