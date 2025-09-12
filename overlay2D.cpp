//=============================================================================
//
// スコア処理 [overlay2D.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "overlay2D.h"
#include "sprite.h"
#include "player.h"
#include"camera.h"
#include"light.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

#define MAX_EXPLOSION (64)

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static bool g_IsMeleePlaying = false;
static bool g_IsTutorialShowing = false;

static int g_MeleeFrame = 0;
static int g_MeleeTimer = 0;

const int MELEE_FRAME_COUNT = 3;
const int MELEE_FRAME_DURATION = 8;

static ID3D11ShaderResourceView* g_TexMelee = nullptr;
static ID3D11ShaderResourceView* g_TexTutorial = nullptr;


struct ExplosionEntry {
    bool     use = false;
    XMFLOAT3 pos{};
    float    size = 160.0f;

    int frame = 0;
    int frameTimer = 0;
};

static const int EXP_COLS = 7;
static const int EXP_ROWS = 3;
static const int EXP_FRAME_COUNT = EXP_COLS * EXP_ROWS; 
static const int EXP_FRAME_DURATION = 3; 

HandState g_HandState = HAND_IDLE;
float g_HandOffsetY = 0.0f;

static ID3D11Buffer* g_VertexBufferOverlay = NULL;
static ID3D11ShaderResourceView* g_TexExplosion = nullptr;
static ExplosionEntry g_Explosions[MAX_EXPLOSION];

static inline void GetExplosionUV_7x3(int frame, float& u, float& v, float& tw, float& th);
static inline DirectX::XMMATRIX MakeBillboardWorld_EnemyStyle(const DirectX::XMFLOAT3& pos, float size);
static inline void WriteQuadVB_EnemyLayout(ID3D11Buffer* vb, float u, float v, float tw, float th);

void InitOverlay2D()
{
    //melee
    D3DX11CreateShaderResourceViewFromFile(GetDevice(),
        "data/TEXTURE/atk.png",
        NULL,
        NULL,
        &g_TexMelee,
        NULL);
    //TutorialShowing
    D3DX11CreateShaderResourceViewFromFile(GetDevice(),
        "data/TEXTURE/bg002.jpg", 
        NULL,
        NULL,
        &g_TexTutorial,
        NULL);
    D3D11_BUFFER_DESC bd = {};
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBufferOverlay);

    D3DX11CreateShaderResourceViewFromFile(GetDevice(),
        "data/TEXTURE/rocketeffect.png",
        NULL, NULL, &g_TexExplosion, NULL);
    for (auto& e : g_Explosions) e.use = false;

}

void UninitOverlay2D()
{
    if (g_VertexBufferOverlay)
    {
        g_VertexBufferOverlay->Release();
        g_VertexBufferOverlay = NULL;
    }
    if (g_TexExplosion) {
        g_TexExplosion->Release(); g_TexExplosion = nullptr;
    }
}

void UpdateOverlay2D()
{
    if (g_IsMeleePlaying)
    {
        g_MeleeTimer++;
        if (g_MeleeTimer >= MELEE_FRAME_DURATION)
        {
            g_MeleeTimer = 0;
            g_MeleeFrame++;
            if (g_MeleeFrame >= MELEE_FRAME_COUNT)
            {
                g_IsMeleePlaying = false;
                g_HandState = HAND_SHOWING;
            }
        }
    }


    // ====== Hand animation ======
    switch (g_HandState)
    {
    case HAND_HIDING:
        g_HandOffsetY += 40.0f;
        if (g_HandOffsetY >= 300.0f)
        {
            g_HandOffsetY = 300.0f;
            g_HandState = HAND_HIDDEN;
        }
        break;
    

    case HAND_HIDDEN:
        break;

    case HAND_SHOWING:
        g_HandOffsetY -= 40.0f;
        if (g_HandOffsetY <= 0.0f)
        {
            g_HandOffsetY = 0.0f;
            g_HandState = HAND_IDLE;
        }
        break;
    }


    for (auto& e : g_Explosions) if (e.use) {
        if (++e.frameTimer >= EXP_FRAME_DURATION) {
            e.frameTimer = 0;
            if (++e.frame >= EXP_FRAME_COUNT) {
                e.use = false;
            }
        }
    }

}

void DrawOverlay2D()
{
    if (g_TexExplosion) {

        CAMERA* cam = GetCamera();
        XMMATRIX viewM = XMLoadFloat4x4(&cam->mtxView);
        XMMATRIX projM = XMLoadFloat4x4(&cam->mtxProjection);
        SetViewMatrix(&viewM);
        SetProjectionMatrix(&projM);

        SetDepthEnable(TRUE);
        SetFogEnable(TRUE);

        UINT stride = sizeof(VERTEX_3D);
        UINT offset = 0;
        GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferOverlay, &stride, &offset);
        GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        SetLightEnable(FALSE);
        SetBlendState(BLEND_MODE_ADD);
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_TexExplosion);

        for (auto& e : g_Explosions) if (e.use) {
            float u, v, tw, th; GetExplosionUV_7x3(e.frame, u, v, tw, th);
            WriteQuadVB_EnemyLayout(g_VertexBufferOverlay, u, v, tw, th);

            XMMATRIX world = MakeBillboardWorld_EnemyStyle(e.pos, e.size);
            SetWorldMatrix(&world);


            GetDeviceContext()->Draw(4, 0);
        }
        SetBlendState(BLEND_MODE_ALPHABLEND);
    }

    SetFogEnable(FALSE);
    SetDepthEnable(FALSE);
    SetWorldViewProjection2D();

    if (g_IsMeleePlaying)
    {
        UINT stride = sizeof(VERTEX_3D);
        UINT offset = 0;
        GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferOverlay, &stride, &offset);
        SetWorldViewProjection2D();
        GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        float u = g_MeleeFrame / (float)MELEE_FRAME_COUNT;
        float v = 0.0f;
        float tw = 1.0f / MELEE_FRAME_COUNT;
        float th = 1.0f;

        float px = SCREEN_WIDTH / 2.0f-30.0f;
        float py = SCREEN_HEIGHT - 290;
        float pw = 1200;
        float ph = 900;

        SetSpriteColor(g_VertexBufferOverlay, px, py, pw, ph, u, v, tw, th, XMFLOAT4(1, 1, 1, 1));

        GetDeviceContext()->PSSetShaderResources(0, 1, &g_TexMelee);
        GetDeviceContext()->Draw(4, 0);
    }

    if (g_IsTutorialShowing)
    {
        UINT stride = sizeof(VERTEX_3D);
        UINT offset = 0;
        GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferOverlay, &stride, &offset);
        SetWorldViewProjection2D();
        GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        float px = SCREEN_WIDTH / 2.0f;
        float py = SCREEN_HEIGHT / 2.0f;
        float pw = 800;
        float ph = 600;

        SetSpriteColor(g_VertexBufferOverlay, px, py, pw, ph, 0, 0, 1, 1, XMFLOAT4(1, 1, 1, 1));
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_TexTutorial);
        GetDeviceContext()->Draw(4, 0);

        SetDepthEnable(TRUE);
        SetFogEnable(TRUE);
        return; 
    }

    //if (!GetPlayer()->alive) {
    //    UINT stride = sizeof(VERTEX_3D);
    //    UINT offset = 0;
    //    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferOverlay, &stride, &offset);
    //    SetWorldViewProjection2D();
    //    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //    float px = SCREEN_WIDTH / 2.0f;
    //    float py = SCREEN_HEIGHT / 2.0f;
    //    float pw = SCREEN_WIDTH;
    //    float ph = SCREEN_HEIGHT;

    //    SetSpriteColor(g_VertexBufferOverlay, px, py, pw, ph, 0, 0, 1, 1, XMFLOAT4(1, 1, 1, 1));

    //    GetDeviceContext()->PSSetShaderResources(0, 1, &g_TexTutorial);
    //    GetDeviceContext()->Draw(4, 0);
    //}

   

   
}

void PlayMeleeAnimation()
{
    g_IsMeleePlaying = true;
    g_MeleeFrame = 0;
    g_MeleeTimer = 0;

    g_HandState = HAND_HIDING;
}

bool IsTutorialShowing()
{
    return g_IsTutorialShowing;
}

void SetTutorialShowing(bool flag)
{
    g_IsTutorialShowing = flag;
}

static inline void GetExplosionUV_7x3(int frame, float& u, float& v, float& tw, float& th) {
    const int cols = 7, rows = 3;
    int r = frame / cols;
    int c = frame % cols;
    tw = 1.0f / cols;
    th = 1.0f / rows;
    u = c * tw;
    v = r * th;
}

static inline XMMATRIX MakeBillboardWorld_EnemyStyle(const XMFLOAT3& pos, float size) {
    CAMERA* cam = GetCamera();
    XMMATRIX view = XMLoadFloat4x4(&cam->mtxView);

    XMMATRIX world = XMMatrixIdentity();
    world.r[0].m128_f32[0] = view.r[0].m128_f32[0];
    world.r[0].m128_f32[1] = view.r[1].m128_f32[0];
    world.r[0].m128_f32[2] = view.r[2].m128_f32[0];

    world.r[1].m128_f32[0] = view.r[0].m128_f32[1];
    world.r[1].m128_f32[1] = view.r[1].m128_f32[1];
    world.r[1].m128_f32[2] = view.r[2].m128_f32[1];

    world.r[2].m128_f32[0] = view.r[0].m128_f32[2];
    world.r[2].m128_f32[1] = view.r[1].m128_f32[2];
    world.r[2].m128_f32[2] = view.r[2].m128_f32[2];

    XMMATRIX scl = XMMatrixScaling(size, size, size);
    XMMATRIX t = XMMatrixTranslation(pos.x, pos.y, pos.z);
    return world * scl * t;
}
static inline void WriteQuadVB_EnemyLayout(ID3D11Buffer* vb, float u, float v, float tw, float th) {
    D3D11_MAPPED_SUBRESOURCE msr{};
    if (SUCCEEDED(GetDeviceContext()->Map(vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr))) {
        VERTEX_3D* vt = (VERTEX_3D*)msr.pData;

        float w = 1.0f, h = 1.0f;
        vt[0].Position = XMFLOAT3(-w * 0.5f, h, 0);
        vt[1].Position = XMFLOAT3(w * 0.5f, h, 0);
        vt[2].Position = XMFLOAT3(-w * 0.5f, 0, 0);
        vt[3].Position = XMFLOAT3(w * 0.5f, 0, 0);

        for (int i = 0; i < 4; ++i) {
            vt[i].Normal = XMFLOAT3(0, 0, -1);
            vt[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
        }

        vt[0].TexCoord = XMFLOAT2(u, v);
        vt[1].TexCoord = XMFLOAT2(u + tw, v);
        vt[2].TexCoord = XMFLOAT2(u, v + th);
        vt[3].TexCoord = XMFLOAT2(u + tw, v + th);

        GetDeviceContext()->Unmap(vb, 0);
    }
}

void SpawnRocketExplosion(const XMFLOAT3& pos, float size) {
    if (!g_TexExplosion) return;
    for (auto& e : g_Explosions) {
        if (!e.use) { e.use = true; e.pos = pos; e.size = size; e.frame = 0; e.frameTimer = 0; return; }
    }
}


float GetHandOffsetY()
{
    return g_HandOffsetY;
}
