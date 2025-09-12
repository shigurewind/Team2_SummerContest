#include "main.h"
#include "renderer.h"
#include "victory.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "GameUI.h"
#include "inputManager.h"
#include "light.h"


#define TEXTURE_WIDTH           (SCREEN_WIDTH)  // 背景サイズ
#define TEXTURE_HEIGHT          (SCREEN_HEIGHT) //
#define TEXTURE_MAX             (2)             // テクスチャの数


static ID3D11Buffer* g_VertexBuffer = NULL;     // 頂点バッファ
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL }; // テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
    "data/TEXTURE/victory_bg.png",      // 背景
    "data/TEXTURE/victory_text.png",    // 文字
};

static BOOL     g_Use;          // TRUE:使用している  FALSE:未使用
static float    g_w, g_h;       // 幅と高さ
static XMFLOAT3 g_Pos;          // ポリゴンの座標
static int      g_TexNo;        // テクスチャ番号


HRESULT InitVictory(void)
{
    ID3D11Device* pDevice = GetDevice();

    
    // テクスチャ生成
    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(GetDevice(),
            g_TexturName[i],
            NULL,
            NULL,
            &g_Texture[i],
            NULL);
    }

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    g_Use = TRUE;
    g_w = TEXTURE_WIDTH;
    g_h = TEXTURE_HEIGHT;
    g_Pos = XMFLOAT3(g_w / 2, g_h / 2, 0.0f);
    g_TexNo = 0;

	// サウンド再生
    PlaySound(SOUND_LABEL_BGM_Victory);

    return S_OK;
}



void UninitVictory(void)
{
    if (g_VertexBuffer)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        if (g_Texture[i])
        {
            g_Texture[i]->Release();
            g_Texture[i] = NULL;
        }
    }
}




void UpdateVictory(void)
{

    if (g_pInputManager->IsActionPressed(ACTION_CONFIRM) || IsMouseLeftTriggered())
    {
        SetFade(FADE_OUT, MODE_TITLE);
    }
}


void DrawVictory(void)
{

    BOOL fogWas = GetFogEnable();
    SetFogEnable(FALSE);
    SetLightEnable(FALSE);
    SetDepthEnable(FALSE);

    SetWorldViewProjection2D();
    SetAlphaTestEnable(FALSE);
    SetBlendState(BLEND_MODE_ALPHABLEND);

    // 頂点バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

    // マトリクス設定
    SetWorldViewProjection2D();

    // プリミティブトポロジ設定
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // マテリアル設定
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    // 背景描画
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
        SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    // 文字描画
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);
        SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, 600.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    SetDepthEnable(TRUE);
    SetFogEnable(fogWas);
    SetLightEnable(TRUE);
}