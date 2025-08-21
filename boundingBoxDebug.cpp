#include "boundingBoxDebug.h"
#include "camera.h"
#include "player.h"

BoundingBoxDebugRenderer* BoundingBoxDebugRenderer::s_instance = nullptr;

BoundingBoxDebugRenderer& BoundingBoxDebugRenderer::GetInstance() {
    if (!s_instance) {
        s_instance = new BoundingBoxDebugRenderer();
    }
    return *s_instance;
}

void BoundingBoxDebugRenderer::Initialize() {
    CreateLineBoxMesh();
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

}


//描画
void BoundingBoxDebugRenderer::Draw() {
    if (!m_globalEnable || m_boxes.empty()) return;

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

