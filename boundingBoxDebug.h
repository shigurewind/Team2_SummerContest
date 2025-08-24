#pragma once
#include "main.h"
#include "renderer.h"
#include "octree.h"


struct DebugBoundingBox {
    XMFLOAT3 min;
    XMFLOAT3 max;
    XMFLOAT4 color;
    bool enabled;
};

//バウンディングボックスクラス
class BoundingBoxDebugRenderer {
public:
    static BoundingBoxDebugRenderer& GetInstance();

    void Initialize();
    void Shutdown();
    void Update();
    void Draw();

    // ボックス追加
    void AddBox(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT4& color = { 1.0f, 0.0f, 0.0f, 1.0f });
    void AddPlayerBox(const XMFLOAT3& center, float size);
    void AddEnemyBox(const XMFLOAT3& center, float size);
    void AddItemBox(const XMFLOAT3& center, float size);


    void AddTerrainBoxes();
    void SetOctreeDepthLimit(int depth) { m_octreeDepthLimit = depth; }
    int GetOctreeDepthLimit() const { return m_octreeDepthLimit; }


    // スイッチ
    void SetGlobalEnable(bool enable) { m_globalEnable = enable; }
    void SetPlayerBoxEnable(bool enable) { m_showPlayerBox = enable; }
    void SetEnemyBoxEnable(bool enable) { m_showEnemyBox = enable; }
    void SetItemBoxEnable(bool enable) { m_showItemBox = enable; }
    void SetTerrainBoxEnable(bool enable) { m_showTerrainBox = enable; }

    bool GetGlobalEnable() const { return m_globalEnable; }
    bool GetPlayerBoxEnable() const { return m_showPlayerBox; }
    bool GetEnemyBoxEnable() const { return m_showEnemyBox; }
    bool GetItemBoxEnable() const { return m_showItemBox; }
    bool GetTerrainBoxEnable() const { return m_showTerrainBox; }


    //法線用
    void SetNormalVectorEnable(bool enable) { m_showNormalVector = enable; }
    bool GetNormalVectorEnable() const { return m_showNormalVector; }
    void SetFloorNormalEnable(bool enable) { m_showFloorNormal = enable; }
    void SetWallNormalEnable(bool enable) { m_showWallNormal = enable; }
    bool GetFloorNormalEnable() const { return m_showFloorNormal; }
    bool GetWallNormalEnable() const { return m_showWallNormal; }
    void SetNormalLength(float length) { m_normalLength = length; }
    float GetNormalLength() const { return m_normalLength; }
    void SetNormalDisplayRange(float range) { m_normalDisplayRange = range; }
    float GetNormalDisplayRange() const { return m_normalDisplayRange; }

private:
    BoundingBoxDebugRenderer() = default;
    ~BoundingBoxDebugRenderer() = default;

    void CreateLineBoxMesh();
    void DrawWireframeBox(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT4& color);


    void TraverseOctreeNode(OctreeNode* node, int currentDepth);
    int m_octreeDepthLimit = 3;//デフォルトは3階まで


    ID3D11Buffer* m_vertexBuffer = nullptr;
    ID3D11Buffer* m_indexBuffer = nullptr;

    std::vector<DebugBoundingBox> m_boxes;

	// デバッグ用のフラグ
    bool m_globalEnable = false;
    bool m_showPlayerBox = false;
    bool m_showEnemyBox = false;
    bool m_showItemBox = false;
    bool m_showTerrainBox = false;


	// 法線表示用
    bool m_showNormalVector = false;
    bool m_showFloorNormal = true;
    bool m_showWallNormal = true;
    float m_normalLength = 5.0f;
	float m_normalDisplayRange = 50.0f;//プレイヤーからの距離

    ID3D11Buffer* m_lineVertexBuffer = nullptr;

    void CreateLineMesh();
    void DrawNormalVectors();
    void DrawLine(const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color);


    static BoundingBoxDebugRenderer* s_instance;
};




