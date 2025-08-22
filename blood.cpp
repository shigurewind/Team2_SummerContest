#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "shadow.h"
#include "light.h"
#include "blood.h"
#include "player.h"



//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_BLOOD   (256)       // 血液パーティクル最大数
#define GRAVITY     (0.3f)      // 重力加速度
#define BLOOD_TEX   "data/TEXTURE/blood2.png"   // 血液用テクスチャ

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct BloodParticle
{
    XMFLOAT3 pos;
    XMFLOAT3 vel;
    float    size;
    float    life;
    float    alpha;
    bool     use;
};

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static BloodParticle g_blood[MAX_BLOOD];
static ID3D11Buffer* g_VertexBuffer = nullptr;
static ID3D11ShaderResourceView* g_TexBlood = nullptr;

//*****************************************************************************
// 初期化
//*****************************************************************************
void InitBlood()
{
    for (int i = 0; i < MAX_BLOOD; i++)
    {
        g_blood[i].use = false;
    }

    // 頂点バッファ作成（四角形）
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    // テクスチャ読み込み
    D3DX11CreateShaderResourceViewFromFile(GetDevice(),
        BLOOD_TEX, NULL, NULL, &g_TexBlood, NULL);

    // 頂点バッファに四角形セット
    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    VERTEX_3D* v = (VERTEX_3D*)msr.pData;

    v[0].Position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
    v[1].Position = XMFLOAT3(0.5f, 0.5f, 0.0f);
    v[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
    v[3].Position = XMFLOAT3(0.5f, -0.5f, 0.0f);

    v[0].TexCoord = { 0.0f, 0.0f };
    v[1].TexCoord = { 1.0f, 0.0f };
    v[2].TexCoord = { 0.0f, 1.0f };
    v[3].TexCoord = { 1.0f, 1.0f };

    for (int i = 0; i < 4; i++) {
        v[i].Normal = { 0,0,-1 };
        v[i].Diffuse = { 1,1,1,1 };
    }

    GetDeviceContext()->Unmap(g_VertexBuffer, 0);
}

//*****************************************************************************
// 終了処理
//*****************************************************************************
void UninitBlood()
{
    if (g_TexBlood) { g_TexBlood->Release(); g_TexBlood = nullptr; }
    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = nullptr; }
}

//*****************************************************************************
// 血液パーティクル発生
//*****************************************************************************
void SpawnBlood(XMFLOAT3 hitPos, int count, XMFLOAT3 hitNormal)
{
    XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&hitNormal));
    XMFLOAT3 N; XMStoreFloat3(&N, n);

    for (int i = 0; i < MAX_BLOOD && count > 0; ++i)
    {
        if (!g_blood[i].use)
        {
            g_blood[i].use = true;
            g_blood[i].pos = hitPos;

            float u = (float)rand() / RAND_MAX;        // 0..1
            float v = (float)rand() / RAND_MAX;        // 0..1
            float phi = 2.0f * 3.1415926f * v;         // 0..2pi
            float cosTheta = u;                         // 0..1
            float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

            XMFLOAT3 dirLocal = { sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta };

            XMVECTOR Nz = XMLoadFloat3(&N);
            XMVECTOR Tx = XMVector3Normalize(XMVector3Cross(Nz, XMVectorSet(0, 1, 0, 0)));
            if (XMVector3LengthSq(Tx).m128_f32[0] < 1e-5f)
                Tx = XMVector3Normalize(XMVector3Cross(Nz, XMVectorSet(1, 0, 0, 0)));
            XMVECTOR By = XMVector3Normalize(XMVector3Cross(Nz, Tx));

            XMVECTOR d = dirLocal.x * Tx + dirLocal.y * By + dirLocal.z * Nz;
            XMFLOAT3 dir; XMStoreFloat3(&dir, d);

            float speed = (float)(rand() % 80) / 10.0f + 6.0f; // 6-14
            g_blood[i].vel = { dir.x * speed, dir.y * speed, dir.z * speed };

            g_blood[i].size = (float)(rand() % 16 + 12);     // 12~27
            g_blood[i].life = 50 + rand() % 25;              // 50~74 
            g_blood[i].alpha = 1.0f;

            --count;
        }
    }
}

//*****************************************************************************
// 更新処理
//*****************************************************************************
void UpdateBlood()
{
    for (int i = 0; i < MAX_BLOOD; i++)
    {
        if (!g_blood[i].use) continue;

        g_blood[i].vel.x *= 0.75f;
        g_blood[i].vel.y *= 0.75f;
        g_blood[i].vel.z *= 0.75f;

        g_blood[i].pos.x += g_blood[i].vel.x * 0.2f;
        g_blood[i].pos.y += g_blood[i].vel.y * 0.2f;
        g_blood[i].pos.z += g_blood[i].vel.z * 0.2f;

        g_blood[i].size *= 1.07f;

        g_blood[i].life -= 2.5f;
        if (g_blood[i].life < 20)
        {
            g_blood[i].alpha -= 0.25f;
            if (g_blood[i].alpha < 0.0f) g_blood[i].alpha = 0.0f;
        }

        if (g_blood[i].life <= 0 || g_blood[i].alpha <= 0.0f)
        {
            g_blood[i].use = false;
        }
    }
}

//*****************************************************************************
// 描画処理
//*****************************************************************************
void DrawBlood()
{
    CAMERA* cam = GetCamera();

    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetDepthEnable(TRUE);
    SetFogEnable(FALSE);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_TexBlood);

    for (int i = 0; i < MAX_BLOOD; i++)
    {
        if (!g_blood[i].use) continue;

        XMVECTOR v = XMLoadFloat3(&g_blood[i].vel);
        float vlen2 = XMVectorGetX(XMVector3LengthSq(v));

        XMMATRIX mtxWorld = XMMatrixIdentity();

        if (vlen2 > 1e-6f)
        {
            XMVECTOR F = XMVector3Normalize(v);

            XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
            XMVECTOR T = XMVector3Cross(F, worldUp);
            if (XMVectorGetX(XMVector3LengthSq(T)) < 1e-6f)
            {
                worldUp = XMVectorSet(1, 0, 0, 0);
                T = XMVector3Cross(F, worldUp);
            }
            T = XMVector3Normalize(T);

            XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, F));

            mtxWorld.r[0] = XMVectorSet(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T), 0.0f); 
            mtxWorld.r[1] = XMVectorSet(XMVectorGetX(F), XMVectorGetY(F), XMVectorGetZ(F), 0.0f); 
            mtxWorld.r[2] = XMVectorSet(XMVectorGetX(N), XMVectorGetY(N), XMVectorGetZ(N), 0.0f); 
        }
        else
        {
            XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);
            mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
            mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
            mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

            mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
            mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
            mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

            mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
            mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
            mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];
        }

        float base = g_blood[i].size;
        float stretch = 1.0f + min(3.0f, sqrtf(vlen2) * 0.05f); 
        XMMATRIX mtxScl = XMMatrixScaling(base, base * stretch, base);
        XMMATRIX mtxTran = XMMatrixTranslation(g_blood[i].pos.x, g_blood[i].pos.y, g_blood[i].pos.z);

        mtxWorld = mtxScl * mtxWorld * mtxTran;
        SetWorldMatrix(&mtxWorld);

        MATERIAL m = {};
        m.Diffuse = { 1.0f, 0.0f, 0.0f, g_blood[i].alpha };
        SetMaterial(m);

        GetDeviceContext()->Draw(4, 0);
    }

    SetFogEnable(TRUE);
}