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
    // ������
    VERTEX_3D vertices[8] = {
        // ���S���_
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        // ��S���_
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
    };

    // ��
    WORD indices[24] = {
        // ��
        0, 1,  1, 2,  2, 3,  3, 0,
        // ��
        4, 5,  5, 6,  6, 7,  7, 4,
        // �c
        0, 4,  1, 5,  2, 6,  3, 7
    };

	//���_�o�b�t�@�̍쐬
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = {};
    vInitData.pSysMem = vertices;

    GetDevice()->CreateBuffer(&vbd, &vInitData, &m_vertexBuffer);

	// �C���f�b�N�X�o�b�t�@�̍쐬
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

	// �v���C���[�̃o�E���f�B���O�{�b�N�X��ǉ�
    if (m_showPlayerBox) {
        PLAYER* player = GetPlayer();
        XMFLOAT3 pos = player->GetPosition();
        float size = player->size;
        AddPlayerBox(pos, size);
    }

    //���̃{�b�N�X
    // if (m_showEnemyBox) {  }
    // if (m_showItemBox) { }

}


//�`��
void BoundingBoxDebugRenderer::Draw() {
    if (!m_globalEnable || m_boxes.empty()) return;

	//�����_�����O�ݒ�
    SetDepthEnable(TRUE);
    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetCullingMode(CULL_MODE_NONE);

	// �S�Ẵ{�b�N�X��`��
    for (const auto& box : m_boxes) {
        if (box.enabled) {
            DrawWireframeBox(box.min, box.max, box.color);
        }
    }

	// �J�[�����O���[�h�����ɖ߂�
    SetCullingMode(CULL_MODE_BACK);
}


void BoundingBoxDebugRenderer::DrawWireframeBox(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT4& color) {
	// �{�b�N�X�̒��S�ƃT�C�Y���v�Z
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

	//���[���h�s���ݒ�
    XMMATRIX mtxScale = XMMatrixScaling(size.x, size.y, size.z);
    XMMATRIX mtxTranslate = XMMatrixTranslation(center.x, center.y, center.z);
    XMMATRIX mtxWorld = mtxScale * mtxTranslate;

    SetWorldMatrix(&mtxWorld);

	// �}�e���A���ݒ�
    MATERIAL mat = {};
    mat.Diffuse = color;
    mat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mat.Shininess = 1.0f;
    mat.noTexSampling = 1;
    SetMaterial(mat);

    // �o�b�t�@
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // �`��
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
    AddBox(min, max, { 0.0f, 1.0f, 0.0f, 1.0f }); // ��
}

void BoundingBoxDebugRenderer::AddEnemyBox(const XMFLOAT3& center, float size) {
    XMFLOAT3 min = { center.x - size, center.y - size, center.z - size };
    XMFLOAT3 max = { center.x + size, center.y + size, center.z + size };
    AddBox(min, max, { 1.0f, 0.0f, 0.0f, 1.0f }); // ��
}

void BoundingBoxDebugRenderer::AddItemBox(const XMFLOAT3& center, float size) {
    XMFLOAT3 min = { center.x - size, center.y - size, center.z - size };
    XMFLOAT3 max = { center.x + size, center.y + size, center.z + size };
	AddBox(min, max, { 0.0f, 0.0f, 1.0f, 1.0f }); // ��
}

