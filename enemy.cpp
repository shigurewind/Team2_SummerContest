//=============================================================================
//
// 
// 
//
//=============================================================================
#pragma once
#include "enemy.h"
#include "player.h"
#include "bullet.h"
#include "debugproc.h"
#include "camera.h"
#include "main.h"
#include "renderer.h"
#include "sprite.h"
#include "input.h"
#include "collision.h"






//*****************************************************************************
//
//*****************************************************************************

std::vector<BaseEnemy*> g_enemies;
ID3D11Buffer* g_VertexBufferEnemy = nullptr;

#define ENEMY_MAX (1)
static BOOL g_bAlphaTestEnemy;

#define ENEMY_OFFSET_Y  (0.0f)

static INTERPOLATION_DATA g_MoveTbl0[] = {
    { XMFLOAT3(0.0f, ENEMY_OFFSET_Y, 20.0f),    XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },
    { XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },
    { XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, -100.0f),XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },
    { XMFLOAT3(0.0f, ENEMY_OFFSET_Y, -200.0f),XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1), 60 * 5 },

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

SpiderEnemy::SpiderEnemy() :
    texture(nullptr), width(100.0f), height(100.0f)
{
    material = new MATERIAL{};
    XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
SpiderEnemy::~SpiderEnemy() {
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
    delete material;
    material = nullptr;
}

void SpiderEnemy::Init() {
    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/2Dpicture/enemy/spider_move.png",
        NULL, NULL, &texture, NULL);


    *material = {};
    material->Diffuse = XMFLOAT4(1, 1, 1, 1);

    pos = XMFLOAT3(0.0f, 0.0f, 20.0f);
    scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
    use = true;
    speed = 1.0f;

    currentFrame = 0;
    frameCounter = 0;
    frameInterval = 15;//change speed
    maxFrames = 2;

    tblNo = 0;
    tblMax = _countof(g_MoveTbl0);
    time = 0.0f;


}

void SpiderEnemy::Update() {
    if (!use) return;



    PLAYER* player = GetPlayer();

    // エネミーからプレイヤーまでのベクトル
    XMFLOAT3 dir;
    dir.x = player->pos.x - pos.x;
    dir.y = player->pos.y - pos.y;
    dir.z = player->pos.z - pos.z;



    //// プレイヤーの座標までの計算
    XMFLOAT3 toPlayer = { dir.x, dir.y, dir.z };

    float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
    float range = 100.0f; // 発射範囲

    //if (fireTimer > 0.0f)
    //{
    //    fireTimer -= 1.0f / 60.0f;  // 60fps
    //}

    //float angleY = atan2f(pos.x - player->pos.x, pos.z - player->pos.z);
    //XMFLOAT3 bulletRot = { 0.0f, angleY, 0.0f };
    //XMFLOAT3 bulletPos = pos;
    //bulletPos.y += 10.0f; 


    //プレイヤーを追いかける行う範囲
    if (distSq < range * range)
    {
        ChasingPlayer(speed, range);
    }
    else
    {
        NormalMovement();
    }

#ifdef _DEBUG

    float dist = sqrtf(distSq);
    PrintDebugProc("Enemy1 to Player Distance: %f\n", dist);

#endif
}

void SpiderEnemy::Draw() {

    if (!use || !texture || !g_VertexBufferEnemy) return;


    SetLightEnable(FALSE);

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


    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    VERTEX_3D* v = (VERTEX_3D*)msr.pData;

    float w = width, h = height;
    v[0].Position = XMFLOAT3(-w / 2, h, 0);
    v[1].Position = XMFLOAT3(w / 2, h, 0);
    v[2].Position = XMFLOAT3(-w / 2, 0, 0);
    v[3].Position = XMFLOAT3(w / 2, 0, 0);

    for (int i = 0; i < 4; ++i) {
        v[i].Normal = XMFLOAT3(0, 0, -1);
        v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
    }

    float tw = 1.0f / 2; // 2フレーム
    float th = 1.0f;
    float tx = currentFrame * tw;
    float ty = 0.0f;

    v[0].TexCoord = XMFLOAT2(tx, ty);
    v[1].TexCoord = XMFLOAT2(tx + tw, ty);
    v[2].TexCoord = XMFLOAT2(tx, ty + th);
    v[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

    GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);

    //�H�H�H
    SetAlphaTestEnable(FALSE);
    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetWorldMatrix(&mtxWorld);
    SetMaterial(*material);
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);


    GetDeviceContext()->Draw(4, 0);

}
void SpiderEnemy::NormalMovement()
{
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
//*****************************************************************************
// 
//*****************************************************************************
void InitEnemy() {
    MakeVertexEnemy();
    g_enemies.clear();
    for (int i = 0; i < ENEMY_MAX; ++i) {
        SpiderEnemy* e1 = new SpiderEnemy();
        e1->Init();
        e1->SetUsed(true);
        XMFLOAT3 pos1 = XMFLOAT3(-50.0f + i * 30.0f, 0.0f, 20.0f);
        e1->SetPosition(pos1);
        g_enemies.push_back(e1);

        GhostEnemy* e = new GhostEnemy();
        e->Init();
        e->SetUsed(true);
        XMFLOAT3 pos = XMFLOAT3(50.0f + i * 30.0f, 0.0f, 20.0f);
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
            PrintDebugProc("Enemy Pos: X:%f Y:%f Z:%f\n", pos.x, pos.y, pos.z);
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

void BaseEnemy::ChasingPlayer(float speed, float chaseRange)
{
    PLAYER* player = GetPlayer();

    // エネミーからプレイヤーまでのベクトル
    XMFLOAT3 dir;
    dir.x = player->pos.x - pos.x;
    dir.y = player->pos.y - pos.y;
    dir.z = player->pos.z - pos.z;

    float distSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;

    if (distSq < chaseRange * chaseRange) {
        // 正規化ベクトル
        XMVECTOR vec = XMVector3Normalize(XMLoadFloat3(&dir));
        XMStoreFloat3(&dir, vec);

        // 位置アップデート
        pos.x += dir.x * speed;
        pos.y += dir.y * speed;
        pos.z += dir.z * speed;
    }
}

GhostEnemy::GhostEnemy() :
    texture(nullptr), width(100.0f), height(100.0f)
{
    material = new MATERIAL{};
    XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
GhostEnemy::~GhostEnemy() {
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
    delete material;
    material = nullptr;
}

void GhostEnemy::Init()
{
    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/2Dpicture/enemy/ghost.png",
        NULL, NULL, &texture, NULL);


    *material = {};
    material->Diffuse = XMFLOAT4(1, 1, 1, 1);

    pos = XMFLOAT3(0.0f, 0.0f, 20.0f);
    scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
    use = true;
    moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f);       // 現在の動き方向
    moveChangeTimer = 2.0f;  // 向き変わるタイマー
    speed = 0.5f;			//エネミーのスピード
    currentFrame = 0;
    frameCounter = 0;
    frameInterval = 15;//change speed
    maxFrames = 1;

}

void GhostEnemy::Update()
{
    if (!use) return;

    NormalMovement();


    PLAYER* player = GetPlayer();

    // エネミーからプレイヤーまでのベクトル
    XMFLOAT3 dir;
    dir.x = player->pos.x - pos.x;
    dir.y = player->pos.y - pos.y;
    dir.z = player->pos.z - pos.z;



    //// プレイヤーの座標までの計算
    XMFLOAT3 toPlayer = { dir.x, dir.y, dir.z };

    float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
    float range = 100.0f; // 発射範囲



    //攻撃行う範囲
    if (distSq < range * range)
    {
        //ChasingPlayer(speed, range);
    }
    else
    {
    }

#ifdef _DEBUG

    float dist = sqrtf(distSq);
    PrintDebugProc("Enemy2 to Player Distance: %f\n", dist);

#endif

}

void GhostEnemy::Draw()
{
    if (!use || !texture || !g_VertexBufferEnemy) return;


    SetLightEnable(FALSE);

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


    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    VERTEX_3D* v = (VERTEX_3D*)msr.pData;

    float w = width, h = height;
    v[0].Position = XMFLOAT3(-w / 2, h, 0);
    v[1].Position = XMFLOAT3(w / 2, h, 0);
    v[2].Position = XMFLOAT3(-w / 2, 0, 0);
    v[3].Position = XMFLOAT3(w / 2, 0, 0);

    for (int i = 0; i < 4; ++i) {
        v[i].Normal = XMFLOAT3(0, 0, -1);
        v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
    }

    float tw = 1.0f ; // 2フレーム
    float th = 1.0f;
    float tx = currentFrame * tw;
    float ty = 0.0f;

    v[0].TexCoord = XMFLOAT2(tx, ty);
    v[1].TexCoord = XMFLOAT2(tx + tw, ty);
    v[2].TexCoord = XMFLOAT2(tx, ty + th);
    v[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

    GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);

    //�H�H�H
    SetAlphaTestEnable(FALSE);
    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetWorldMatrix(&mtxWorld);
    SetMaterial(*material);
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);


    GetDeviceContext()->Draw(4, 0);

}

void GhostEnemy::NormalMovement()
{
    frameCounter++;
    if (frameCounter >= frameInterval) {
        frameCounter = 0;
        currentFrame = (currentFrame + 1) % maxFrames;
    }

    // 動き方向変わりタイマー
    moveChangeTimer -= 1.0f / 60.0f; // 60fpsことに動き方向変わり
    if (moveChangeTimer <= 0.0f) {
        moveChangeTimer = 2.0f; // reset timer

        // 動き方向変わることはランダム設定
        int dir = rand() % 4;
        switch (dir) {
        case 0: moveDir = XMFLOAT3(1.0f, 0.0f, 0.0f); break;  // 右
        case 1: moveDir = XMFLOAT3(-1.0f, 0.0f, 0.0f); break; // 左
        case 2: moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f); break;  // 前（+ｚ）
        case 3: moveDir = XMFLOAT3(0.0f, 0.0f, -1.0f); break; // 後ろ（-ｚ）
        }
    }

    pos.x += moveDir.x * speed;
    pos.y += moveDir.y * speed;
    pos.z += moveDir.z * speed;
}

