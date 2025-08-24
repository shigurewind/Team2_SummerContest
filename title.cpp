//=============================================================================
//
// タイトル画面処理 [title.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(4)				// テクスチャの数

#define TEXTURE_WIDTH_LOGO			(800)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(400)			// 

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/bg02.png",
	"data/TEXTURE/TITLE.png",
	"data/TEXTURE/effect000.jpg",
	"data/TEXTURE/GAME_START.png",

};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号
static XMFLOAT3					g_TitlePos;
static XMFLOAT3					g_GameStartPos;
float	alpha;
BOOL	flag_alpha;

// --- 当たり判定の手動調整用パラメータ ---
static float g_HitOffsetX  = +60.0f;   // +で右へ、-で左へ
static float g_HitOffsetY  = +50.0f;   // +で下へ、-で上へ
static float g_HitInflateW = -60.0f;   // +で幅を広げる（全体）。-で狭める
static float g_HitInflateH = -100.0f;   // +で高さを広げる（全体）。-で狭める

// 「ゲームスタート」ボタン用の基本サイズと状態
static float g_GameStartBaseW = 600.0f;   // 既存描画と同じ幅
static float g_GameStartBaseH = 300.0f;   // 既存描画と同じ高さ
static float g_GameStartScale = 1.0f;     // 拡大率
static bool  g_GameStartHover = false;    // ホバー中か

static BOOL						g_Load = FALSE;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
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


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// 変数の初期化
	g_Use   = TRUE;
	g_w     = TEXTURE_WIDTH;
	g_h     = TEXTURE_HEIGHT;
	g_Pos          = XMFLOAT3(g_w/2, g_h/2, 0.0f);
	g_TitlePos     = XMFLOAT3(SCREEN_WIDTH / 2, 150.0f, 0.0f); // タイトルだけ上へ
	g_GameStartPos = XMFLOAT3(SCREEN_WIDTH / 2, 500.0f, 0.0f);
	g_TexNo = 0;

	alpha = 1.0f;
	flag_alpha = TRUE;

	// BGM再生
	PlaySound(SOUND_LABEL_BGM_sample000);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTitle(void)
{
	if (g_Load == FALSE) return;

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

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTitle(void)
{

	if (GetKeyboardTrigger(DIK_RETURN))
	{// Enter押したら、ステージを切り替える
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}
	// ゲームパッドで入力処理
	else if (IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}
	else if (IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}


	if (GetKeyboardTrigger(DIK_RETURN)) { SetFade(FADE_OUT, MODE_TUTORIAL); }
	else if (IsButtonTriggered(0, BUTTON_START)) { SetFade(FADE_OUT, MODE_TUTORIAL); }
	else if (IsButtonTriggered(0, BUTTON_B)) { SetFade(FADE_OUT, MODE_TUTORIAL); }

	POINT mp;
	GetCursorPos(&mp);


	// ボタンの描画サイズ（拡大率を反映）
	float drawW = g_GameStartBaseW * g_GameStartScale;
	float drawH = g_GameStartBaseH * g_GameStartScale;

	// ★ 手動調整を反映した“判定用”サイズと中心
	float testW = drawW + g_HitInflateW;
	float testH = drawH + g_HitInflateH;
	float cx = g_GameStartPos.x + g_HitOffsetX;
	float cy = g_GameStartPos.y + g_HitOffsetY;

	float halfW = testW * 0.5f;
	float halfH = testH * 0.5f;

	g_GameStartHover =
		(mp.x >= cx - halfW) && (mp.x <= cx + halfW) &&
		(mp.y >= cy - halfH) && (mp.y <= cy + halfH);

	// ホバー時は少し拡大（スムーズに補間）
	const float targetScale = g_GameStartHover ? 1.08f : 1.0f;
	g_GameStartScale += (targetScale - g_GameStartScale) * 0.2f;

	// 左クリックは「ボタン上にあるときだけ」シーン遷移
	if (g_GameStartHover && IsMouseLeftTriggered()) {
		SetFade(FADE_OUT, MODE_TUTORIAL);
	}

	if (flag_alpha == TRUE)
	{
		alpha -= 0.02f;
		if (alpha <= 0.0f)
		{
			alpha = 0.0f;
			flag_alpha = FALSE;
		}
	}
	else
	{
		alpha += 0.02f;
		if (alpha >= 1.0f)
		{
			alpha = 1.0f;
			flag_alpha = TRUE;
		}
	}






#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitle(void)
{
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

	// タイトルの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// タイトルのロゴを描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
	//	SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);
		SetSprite(g_VertexBuffer, g_TitlePos.x, g_TitlePos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);
		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// 「ゲームスタート」ボタン画像を描画
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		float drawW = g_GameStartBaseW * g_GameStartScale;
		float drawH = g_GameStartBaseH * g_GameStartScale;

		// ホバー時はアルファを上げて存在感を出す（お好みで）
		float a = g_GameStartHover ? 1.0f : alpha;

		SetSpriteColor(g_VertexBuffer,
			g_GameStartPos.x, g_GameStartPos.y,
			drawW, drawH,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, a));

		GetDeviceContext()->Draw(4, 0);
	}
//	// 加減算のテスト
//	SetBlendState(BLEND_MODE_ADD);		// 加算合成
////	SetBlendState(BLEND_MODE_SUBTRACT);	// 減算合成
//	for(int i=0; i<30; i++)
//	{
//		// テクスチャ設定
//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
//
//		// １枚のポリゴンの頂点とテクスチャ座標を設定
//		float dx = 100.0f;
//		float dy = 100.0f;
//		float sx = (float)(rand() % 100);
//		float sy = (float)(rand() % 100);
//
//
//		SetSpriteColor(g_VertexBuffer, dx+sx, dy+sy, 50, 50, 0.0f, 0.0f, 1.0f, 1.0f,
//			XMFLOAT4(0.3f, 0.3f, 1.0f, 0.5f));
//
//		// ポリゴン描画
//		GetDeviceContext()->Draw(4, 0);
//	}
//	SetBlendState(BLEND_MODE_ALPHABLEND);	// 半透明処理を元に戻す

}





