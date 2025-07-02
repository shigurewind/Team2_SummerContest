//=============================================================================
//
// スコア処理 [score.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "overlay2D.h"
#include "sprite.h"
#include "player.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


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



static ID3D11Buffer* g_VertexBufferOverlay = NULL;




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



}

void UninitOverlay2D()
{
    if (g_VertexBufferOverlay)
    {
        g_VertexBufferOverlay->Release();
        g_VertexBufferOverlay = NULL;
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
            }
        }
    }




}

void DrawOverlay2D()
{

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

        return; 
    }

}

void PlayMeleeAnimation()
{
    g_IsMeleePlaying = true;
    g_MeleeFrame = 0;
    g_MeleeTimer = 0;
}

bool IsTutorialShowing()
{
    return g_IsTutorialShowing;
}

void SetTutorialShowing(bool flag)
{
    g_IsTutorialShowing = flag;
}