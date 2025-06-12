//=============================================================================
//
// 
// 
//
//=============================================================================
#pragma once
#include "enemy.h"
#include "debugproc.h"
#include "camera.h"
#include "main.h"
#include "renderer.h"
#include "sprite.h"



//*****************************************************************************
//
//*****************************************************************************

std::vector<BaseEnemy*> g_enemies;
ID3D11Buffer* g_VertexBufferEnemy = nullptr;


#define ENEMY_OFFSET_Y 0.0f

static INTERPOLATION_DATA g_MoveTbl0[] = {
    { XMFLOAT3(0.0f, ENEMY_OFFSET_Y, 20.0f),    XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },
    { XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 20.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },
    { XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 200.0f),XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },
};

INTERPOLATION_DATA* g_MoveTblAdr[] = {
    g_MoveTbl0,
};


//*****************************************************************************
// 
//*****************************************************************************
BaseEnemy::BaseEnemy() : pos({ 0,0,0 }), scl({ 1,1,1 }), use(false) {
    XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
BaseEnemy::~BaseEnemy() {}

ScarecrowEnemy::ScarecrowEnemy() :
    texture(nullptr), width(60.0f), height(90.0f)
{
    material = new MATERIAL{};
    XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());

}
ScarecrowEnemy::~ScarecrowEnemy() {
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
    delete material;
    material = nullptr;
}

void ScarecrowEnemy::Init() {
    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/TEXTURE/enemy001.png",
        NULL, NULL, &texture, NULL);

    *material = {};
    material->Diffuse = XMFLOAT4(1, 1, 1, 1);

    pos = XMFLOAT3(0.0f, 0.0f, 20.0f);
    scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
    use = true;

    currentFrame = 0;
    frameCounter = 0;
    frameInterval = 30;//change speed
    maxFrames = 2;

    tblNo = 0;
    tblMax = _countof(g_MoveTbl0);
    time = 0.0f;

}

void ScarecrowEnemy::Update() {
    if (!use) return;

    frameCounter++;
    if (frameCounter >= frameInterval) {
        frameCounter = 0;
        currentFrame = (currentFrame + 1) % maxFrames;
    }

    if (tblMax <= 0) return;

    int nowNo = (int)time;
    int nextNo = (nowNo + 1) % tblMax;

    INTERPOLATION_DATA* tbl = g_MoveTblAdr[tblNo];

    XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);
    XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);
    XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);

    XMVECTOR diffPos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;
    XMVECTOR diffRot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;
    XMVECTOR diffScl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;

    float localTime = time - nowNo;

    XMStoreFloat3(&pos, nowPos + diffPos * localTime);
    XMStoreFloat3(&scl, nowScl + diffScl * localTime);

    time += 1.0f / tbl[nowNo].frame;
    if ((int)time >= tblMax) {
        time -= tblMax;
    }

}

void ScarecrowEnemy::Draw() {
    if (!use || !texture || !g_VertexBufferEnemy) return;

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferEnemy, &stride, &offset);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CAMERA* cam = GetCamera();
    XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);

    XMMATRIX mtxWorld = XMMatrixIdentity();
    mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
    mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
    mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

    mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
    mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
    mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

    mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
    mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
    mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

    XMMATRIX mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
    XMMATRIX mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

    ////�H�H�H
    //SetAlphaTestEnable(TRUE);
    //SetBlendState(BLEND_MODE_ALPHABLEND);
    //SetWorldMatrix(&mtxWorld);
    //SetMaterial(*material);
    //GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    //float px = pos.x;	// プレイヤーの表示位置X
    //float py = pos.y;	// プレイヤーの表示位置Y
    //float pw = width;	// プレイヤーの表示幅
    //float ph = height;	// プレイヤーの表示高さ
    //py += 50.0f;		// 足元に表示

    //float tw = 1.0f;	// テクスチャの幅
    //float th = 1.0f;	// テクスチャの高さ
    //float tx = 0.0f;	// テクスチャの左上X座標
    //float ty = 0.0f;	// テクスチャの左上Y座標

    //// １枚のポリゴンの頂点とテクスチャ座標を設定
    //SetSpriteColor(g_VertexBufferEnemy, px, py, pw, ph, tx, ty, tw, th,
    //    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));



    GetDeviceContext()->Draw(4, 0);

    SetAlphaTestEnable(FALSE);
}
//*****************************************************************************
// 
//*****************************************************************************
void InitEnemy() {
    MakeVertexEnemy();
    g_enemies.clear();
    for (int i = 0; i < 5; ++i) {
        ScarecrowEnemy* e = new ScarecrowEnemy();
        e->Init();
        e->SetUsed(true);
        XMFLOAT3 pos = XMFLOAT3(-50.0f + i * 30.0f, 0.0f, 20.0f);
        e->SetPosition(pos);
        g_enemies.push_back(e);
    }
}

void UpdateEnemy() {
    for (auto enemy : g_enemies) {
        if (enemy->IsUsed()) enemy->Update();
    }
}

void DrawEnemy() {
    for (auto enemy : g_enemies) {
        if (enemy->IsUsed()) enemy->Draw();
    }


#ifdef _DEBUG
    for (auto enemy : g_enemies)
    {
        if (enemy->IsUsed())
        {
            XMFLOAT3 pos = enemy->GetPosition();
            PrintDebugProc("Enemy Pos: X:%.2f Y:%.2f Z:%.2f\n", pos.x, pos.y, pos.z);
            break;
        }
    }
#endif
}

void UninitEnemy() {
    for (auto enemy : g_enemies) {
        delete enemy;
    }
    g_enemies.clear();

    if (g_VertexBufferEnemy) {
        g_VertexBufferEnemy->Release();
        g_VertexBufferEnemy = nullptr;
    }
}

std::vector<BaseEnemy*>& GetEnemies() {
    return g_enemies;
}

HRESULT MakeVertexEnemy() {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = GetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBufferEnemy);
    if (FAILED(hr)) return hr;

    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    VERTEX_3D* v = (VERTEX_3D*)msr.pData;

    float w = 60.0f, h = 90.0f;
    v[0].Position = XMFLOAT3(-w / 2, h, 0);
    v[1].Position = XMFLOAT3(w / 2, h, 0);
    v[2].Position = XMFLOAT3(-w / 2, 0, 0);
    v[3].Position = XMFLOAT3(w / 2, 0, 0);

    for (int i = 0; i < 4; ++i) {
        v[i].Normal = XMFLOAT3(0, 0, -1);
        v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
    }

    v[0].TexCoord = XMFLOAT2(0, 0);
    v[1].TexCoord = XMFLOAT2(1, 0);
    v[2].TexCoord = XMFLOAT2(0, 1);
    v[3].TexCoord = XMFLOAT2(1, 1);

    GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);
    return S_OK;
}
void BaseEnemy::SetPosition(const XMFLOAT3& p) {
    pos = p;
}

XMFLOAT3 BaseEnemy::GetPosition() const {
    return pos;
}

void BaseEnemy::SetScale(const XMFLOAT3& s) {
    scl = s;
}

XMFLOAT3 BaseEnemy::GetScale() const {
    return scl;
}