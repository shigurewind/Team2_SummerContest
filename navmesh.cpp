#include "navmesh.h"
#include <algorithm>
#include "debugproc.h"
#include "renderer.h"
#include "model.h"
static std::vector<NavTriangle> g_NavMesh;





void InitNavMeshFromModel(AMODEL* model, const XMMATRIX& worldMatrix)
{
	g_NavMesh.clear();

	if (!model || !model->AiScene) return;

	for (unsigned int meshIndex = 0; meshIndex < model->AiScene->mNumMeshes; ++meshIndex)
	{
		const aiMesh* mesh = model->AiScene->mMeshes[meshIndex];
		if (!mesh || !mesh->mVertices) continue;

		const aiVector3D* vertices = mesh->mVertices;

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices != 3) continue;

			XMFLOAT3 v[3];
			for (int j = 0; j < 3; j++)
			{
				XMVECTOR local = XMVectorSet(
					vertices[face.mIndices[j]].x,
					vertices[face.mIndices[j]].y,
					vertices[face.mIndices[j]].z,
					1.0f);
				XMVECTOR world = XMVector3Transform(local, worldMatrix);
				XMStoreFloat3(&v[j], world);
			}

			XMVECTOR edge1 = XMVectorSubtract(XMLoadFloat3(&v[1]), XMLoadFloat3(&v[0]));
			XMVECTOR edge2 = XMVectorSubtract(XMLoadFloat3(&v[2]), XMLoadFloat3(&v[0]));
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge1, edge2));

			XMFLOAT3 n;
			XMStoreFloat3(&n, normal);

			if (fabs(n.y) >= 0.5f)
			{
				g_NavMesh.push_back({ v[0], v[1], v[2], n });
			}
		}
	}

	char buf[128];
	sprintf_s(buf, "[NavMesh] Triangles: %zu\n", g_NavMesh.size());
	OutputDebugStringA(buf);
}

//-----------------------------------------------------------------------------------
bool PointInTriangleXZ(const XMFLOAT3& p, const NavTriangle& tri)
{
	XMFLOAT2 p0 = { tri.v0.x, tri.v0.z };
	XMFLOAT2 p1 = { tri.v1.x, tri.v1.z };
	XMFLOAT2 p2 = { tri.v2.x, tri.v2.z };
	XMFLOAT2 pt = { p.x, p.z };

	auto edge = [](XMFLOAT2 a, XMFLOAT2 b, XMFLOAT2 p) {
		return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
		};

	float d1 = edge(p0, p1, pt);
	float d2 = edge(p1, p2, pt);
	float d3 = edge(p2, p0, pt);

	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

bool GetNavMeshHeight(float x, float z, float* outY)
{
	XMFLOAT3 p = { x, 0, z };
	for (const auto& tri : g_NavMesh)
	{
		if (PointInTriangleXZ(p, tri))
		{
			XMVECTOR v0 = XMLoadFloat3(&tri.v0);
			XMVECTOR v1 = XMLoadFloat3(&tri.v1);
			XMVECTOR v2 = XMLoadFloat3(&tri.v2);

			XMVECTOR n = XMVector3Cross(XMVectorSubtract(v1, v0), XMVectorSubtract(v2, v0));
			n = XMVector3Normalize(n);

			XMFLOAT3 n3;
			XMStoreFloat3(&n3, n);

			float d = -(n3.x * tri.v0.x + n3.y * tri.v0.y + n3.z * tri.v0.z);
			float y = -(n3.x * x + n3.z * z + d) / n3.y;

			if (outY) *outY = y;
			return true;
		}
	}
	return false;
}

bool IsPointOnNavMesh(float x, float z)
{
	float y;
	return GetNavMeshHeight(x, z, &y);
}