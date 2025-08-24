#include "boundingBoxDebug.h"
#include "camera.h"
#include "player.h"
#include "FBXmodel.h"

BoundingBoxDebugRenderer* BoundingBoxDebugRenderer::s_instance = nullptr;

BoundingBoxDebugRenderer& BoundingBoxDebugRenderer::GetInstance() {
    if (!s_instance) {
        s_instance = new BoundingBoxDebugRenderer();
    }
    return *s_instance;
}

void BoundingBoxDebugRenderer::Initialize() {
    CreateLineBoxMesh();
    CreateLineMesh();
}

void BoundingBoxDebugRenderer::Shutdown() {
    if (m_vertexBuffer) {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }

    if (m_indexBuffer) {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }

    if (m_lineVertexBuffer) {
        m_lineVertexBuffer->Release();
        m_lineVertexBuffer = nullptr;
    }
}


void BoundingBoxDebugRenderer::CreateLineBoxMesh() {
    // 立方体
    VERTEX_3D vertices[8] = {
        // 下４頂点
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        // 上４頂点
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
    };

    // 線
    WORD indices[24] = {
        // 下
        0, 1,  1, 2,  2, 3,  3, 0,
        // 上
        4, 5,  5, 6,  6, 7,  7, 4,
        // 縦
        0, 4,  1, 5,  2, 6,  3, 7
    };

	//頂点バッファの作成
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = {};
    vInitData.pSysMem = vertices;

    GetDevice()->CreateBuffer(&vbd, &vInitData, &m_vertexBuffer);

	// インデックスバッファの作成
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData = {};
    iInitData.pSysMem = indices;

    GetDevice()->CreateBuffer(&ibd, &iInitData, &m_indexBuffer);
}


// 線分メッシュの作成
void BoundingBoxDebugRenderer::CreateLineMesh() {
    // 頂点バッファ
    VERTEX_3D lineVertices[2] = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
    };

    D3D11_BUFFER_DESC lbd = {};
    lbd.Usage = D3D11_USAGE_DYNAMIC;
    lbd.ByteWidth = sizeof(lineVertices);
    lbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    lbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA lInitData = {};
    lInitData.pSysMem = lineVertices;

    GetDevice()->CreateBuffer(&lbd, &lInitData, &m_lineVertexBuffer);
}



void BoundingBoxDebugRenderer::Update() {
    if (!m_globalEnable) return;

    m_boxes.clear();

	// プレイヤーのバウンディングボックスを追加
    if (m_showPlayerBox) {
        PLAYER* player = GetPlayer();
        XMFLOAT3 pos = player->GetPosition();
        float size = player->size;
        AddPlayerBox(pos, size);
    }

    //他のボックス
    // if (m_showEnemyBox) {  }
    // if (m_showItemBox) { }


    if (m_showTerrainBox) {
        AddTerrainBoxes();
    }

}


//描画
void BoundingBoxDebugRenderer::Draw() {
    if (!m_globalEnable) return;

	//ボックスを描画
    if (!m_boxes.empty())
    {
        //レンダリング設定
        SetDepthEnable(TRUE);
        SetBlendState(BLEND_MODE_ALPHABLEND);
        SetCullingMode(CULL_MODE_NONE);

        // 全てのボックスを描画
        for (const auto& box : m_boxes) {
            if (box.enabled) {
                DrawWireframeBox(box.min, box.max, box.color);
            }
        }

        // カーリングモードを元に戻す
        SetCullingMode(CULL_MODE_BACK);
    }


	// 線分メッシュの描画
    if (m_showNormalVector) {
        DrawNormalVectors();
    }
	
}


void BoundingBoxDebugRenderer::DrawWireframeBox(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT4& color) {
	// ボックスの中心とサイズを計算
    XMFLOAT3 center = {
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f,
        (min.z + max.z) * 0.5f
    };

    XMFLOAT3 size = {
        max.x - min.x,
        max.y - min.y,
        max.z - min.z
    };

	//ワールド行列を設定
    XMMATRIX mtxScale = XMMatrixScaling(size.x, size.y, size.z);
    XMMATRIX mtxTranslate = XMMatrixTranslation(center.x, center.y, center.z);
    XMMATRIX mtxWorld = mtxScale * mtxTranslate;

    SetWorldMatrix(&mtxWorld);

	// マテリアル設定
    MATERIAL mat = {};
    mat.Diffuse = color;
    mat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Shininess = 1.0f;
    mat.noTexSampling = 1;
    SetMaterial(mat);

    // バッファ
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // 描画
    GetDeviceContext()->DrawIndexed(24, 0, 0);
}


// 法線ベクトルの描画
void BoundingBoxDebugRenderer::DrawNormalVectors() {
    PLAYER* player = GetPlayer();
    XMFLOAT3 playerPos = player->GetPosition();

	//レンダリング設定
    SetDepthEnable(TRUE);
    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetCullingMode(CULL_MODE_NONE);

	// 床の法線を描画
    if (m_showFloorNormal) {
        const auto& floorTriangles = GetFloorTriangles();
        for (const auto& tri : floorTriangles) {
			// 三角形の中心を計算
            XMFLOAT3 center = {
                (tri.v0.x + tri.v1.x + tri.v2.x) / 3.0f,
                (tri.v0.y + tri.v1.y + tri.v2.y) / 3.0f,
                (tri.v0.z + tri.v1.z + tri.v2.z) / 3.0f
            };

			// 距離チェック
            float dx = center.x - playerPos.x;
            float dy = center.y - playerPos.y;
            float dz = center.z - playerPos.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);

            if (distance <= m_normalDisplayRange) {
                XMFLOAT3 normalEnd = {
                    center.x + tri.normal.x * m_normalLength,
                    center.y + tri.normal.y * m_normalLength,
                    center.z + tri.normal.z * m_normalLength
                };
				DrawLine(center, normalEnd, { 0.0f, 1.0f, 0.0f, 1.0f }); // 床は緑色
            }
        }
    }

	// 壁の法線を描画
    if (m_showWallNormal) {
        const auto& wallTriangles = GetWallTriangles();
        for (const auto& tri : wallTriangles) {
            // 三角形の中心を計算
            XMFLOAT3 center = {
                (tri.v0.x + tri.v1.x + tri.v2.x) / 3.0f,
                (tri.v0.y + tri.v1.y + tri.v2.y) / 3.0f,
                (tri.v0.z + tri.v1.z + tri.v2.z) / 3.0f
            };

            // 距離チェック
            float dx = center.x - playerPos.x;
            float dy = center.y - playerPos.y;
            float dz = center.z - playerPos.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);

            if (distance <= m_normalDisplayRange) {
                XMFLOAT3 normalEnd = {
                    center.x + tri.normal.x * m_normalLength,
                    center.y + tri.normal.y * m_normalLength,
                    center.z + tri.normal.z * m_normalLength
                };
				DrawLine(center, normalEnd, { 1.0f, 0.0f, 0.0f, 1.0f }); //壁は赤色
            }
        }
    }

	// カーリングモードを元に戻す
    SetCullingMode(CULL_MODE_BACK);
}

// 線分の描画
void BoundingBoxDebugRenderer::DrawLine(const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color) {
	// 線の頂点データを更新
    D3D11_MAPPED_SUBRESOURCE mapped;
    GetDeviceContext()->Map(m_lineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    VERTEX_3D* vertices = reinterpret_cast<VERTEX_3D*>(mapped.pData);

    vertices[0].Position = start;
    vertices[0].Diffuse = color;
    vertices[1].Position = end;
    vertices[1].Diffuse = color;

    GetDeviceContext()->Unmap(m_lineVertexBuffer, 0);

	// 行列を単位行列に設定
    XMMATRIX identity = XMMatrixIdentity();
    SetWorldMatrix(&identity);

	// マテリアル設定
    MATERIAL mat = {};
    mat.Diffuse = color;
    mat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Shininess = 1.0f;
    mat.noTexSampling = 1;
    SetMaterial(mat);

	// 頂点バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &m_lineVertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// 線の描画
    GetDeviceContext()->Draw(2, 0);
}


void BoundingBoxDebugRenderer::AddBox(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT4& color) {
    DebugBoundingBox box;
    box.min = min;
    box.max = max;
    box.color = color;
    box.enabled = true;
    m_boxes.push_back(box);
}

void BoundingBoxDebugRenderer::AddPlayerBox(const XMFLOAT3& center, float size) {
    XMFLOAT3 min = { center.x - size, center.y - size, center.z - size };
    XMFLOAT3 max = { center.x + size, center.y + size, center.z + size };
    AddBox(min, max, { 0.0f, 1.0f, 0.0f, 1.0f }); // 緑
}

void BoundingBoxDebugRenderer::AddEnemyBox(const XMFLOAT3& center, float size) {
    XMFLOAT3 min = { center.x - size, center.y - size, center.z - size };
    XMFLOAT3 max = { center.x + size, center.y + size, center.z + size };
    AddBox(min, max, { 1.0f, 0.0f, 0.0f, 1.0f }); // 赤
}

void BoundingBoxDebugRenderer::AddItemBox(const XMFLOAT3& center, float size) {
    XMFLOAT3 min = { center.x - size, center.y - size, center.z - size };
    XMFLOAT3 max = { center.x + size, center.y + size, center.z + size };
	AddBox(min, max, { 0.0f, 0.0f, 1.0f, 1.0f }); // 青
}


void BoundingBoxDebugRenderer::AddTerrainBoxes() {
	// 八分木から壁と床のボックスを取得
    OctreeNode* wallTree = GetWallTree();
    OctreeNode* floorTree = GetFloorTree();

	// 壁八分木を走査してボックスを追加
    if (wallTree) {
        TraverseOctreeNode(wallTree, 0);
    }

	// 床八分木を走査してボックスを追加
    if (floorTree) {
        TraverseOctreeNode(floorTree, 0);
    }
}


void BoundingBoxDebugRenderer::TraverseOctreeNode(OctreeNode* node, int currentDepth) {
    if (!node || currentDepth > m_octreeDepthLimit) return;

	// 葉ノードまたは深度制限に達した場合、ボックスを追加
    bool shouldDraw = (node->IsLeaf() || currentDepth == m_octreeDepthLimit)
        && !node->triangleIndices.empty();

    if (shouldDraw) {
		// 深度に応じて色を変更
        XMFLOAT4 color;
        switch (currentDepth % 4) {
        case 0: color = { 1.0f, 0.0f, 0.0f, 0.7f }; break;  //赤
        case 1: color = { 0.0f, 1.0f, 0.0f, 0.7f }; break;  //緑
        case 2: color = { 0.0f, 0.0f, 1.0f, 0.7f }; break;  //青
        case 3: color = { 1.0f, 1.0f, 0.0f, 0.7f }; break;  //黄
        default: color = { 1.0f, 0.0f, 1.0f, 0.7f }; break; //紫
        }

        AddBox(node->minBound, node->maxBound, color);
    }

	// 再帰的に子ノードを走査
    if (node->isSubdivided) {
        for (int i = 0; i < 8; i++) {
            if (node->children[i]) {
                TraverseOctreeNode(node->children[i], currentDepth + 1);
            }
        }
    }
}





