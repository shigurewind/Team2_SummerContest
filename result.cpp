//=============================================================================
//
// リザルト画面処理 [result.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "result.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "GameUI.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(4)				// テクスチャの数

#define TEXTURE_WIDTH_LOGO			(480)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(80)			// 

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/bg.png",
	"data/TEXTURE/restart.png",
	"data/TEXTURE/number16x32.png",
	"data/TEXTURE/title02.png"
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static BOOL						g_Load = FALSE;

// --- Result画面のボタン（title / restart）用 ---------------
static XMFLOAT3 g_TitleBtnPos;    // 左下に置く
static XMFLOAT3 g_RestartBtnPos;  // 右下に置く

// ボタンの基準サイズ（必要に応じて微調整）
static float g_TitleBtnBaseW = 400.0f;
static float g_TitleBtnBaseH = 160.0f;
static float g_RestartBtnBaseW = 400.0f;
static float g_RestartBtnBaseH = 160.0f;

// 現在の拡大率＆ホバー中フラグ
static float g_TitleBtnScale = 1.0f;
static float g_RestartBtnScale = 1.0f;
static bool  g_TitleBtnHover = false;
static bool  g_RestartBtnHover = false;

// 当たり判定の手動補正（必要に応じて数値調整）
static float g_TitleHitOffsetX = 50.0f;
static float g_TitleHitOffsetY = 80.0f;
static float g_TitleHitInflateW = 0.0f;
static float g_TitleHitInflateH = 0.0f;

static float g_RestartHitOffsetX = 50.0f;
static float g_RestartHitOffsetY = 80.0f;
static float g_RestartHitInflateW = 0.0f;
static float g_RestartHitInflateH = 0.0f;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitResult(void)
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
	g_Pos   = { g_w / 2, 50.0f, 0.0f };
	g_TexNo = 0;

	// BGM再生
	PlaySound(SOUND_LABEL_BGM_sample002);


	const float margin = 100.0f;

	// 左下に「TITLE」ボタン
	g_TitleBtnPos.x = margin + g_TitleBtnBaseW * 0.5f;
	g_TitleBtnPos.y = SCREEN_HEIGHT - margin - g_TitleBtnBaseH * 0.5f;
	g_TitleBtnPos.z = 0.0f;

	// 右下に「RESTART」ボタン
	g_RestartBtnPos.x = SCREEN_WIDTH - margin - g_RestartBtnBaseW * 0.5f;
	g_RestartBtnPos.y = SCREEN_HEIGHT - margin - g_RestartBtnBaseH * 0.5f;
	g_RestartBtnPos.z = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitResult(void)
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
void UpdateResult(void)
{

	if (GetKeyboardTrigger(DIK_RETURN))
	{// Enter押したら、ステージを切り替える
		SetFade(FADE_OUT, MODE_TITLE);
	}
	// ゲームパッドで入力処理
	else if (IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_TITLE);
	}
	else if (IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_TITLE);
	}



	POINT mp;
	GetCursorPos(&mp); // マウス座標取得

	// ==== TITLEボタン ====
	{
		float drawW = g_TitleBtnBaseW * g_TitleBtnScale;
		float drawH = g_TitleBtnBaseH * g_TitleBtnScale;

		// 判定用サイズ（手動補正を適用）
		float testW = drawW + g_TitleHitInflateW;
		float testH = drawH + g_TitleHitInflateH;
		float cx = g_TitleBtnPos.x + g_TitleHitOffsetX;
		float cy = g_TitleBtnPos.y + g_TitleHitOffsetY;

		float halfW = testW * 0.5f;
		float halfH = testH * 0.5f;

		g_TitleBtnHover =
			(mp.x >= cx - halfW) && (mp.x <= cx + halfW) &&
			(mp.y >= cy - halfH) && (mp.y <= cy + halfH);

		const float targetScale = g_TitleBtnHover ? 1.08f : 1.0f;
		g_TitleBtnScale += (targetScale - g_TitleBtnScale) * 0.2f;

		if (g_TitleBtnHover && IsMouseLeftTriggered()) {
			SetFade(FADE_OUT, MODE_TITLE); // ← タイトルへ
		}
	}

	// ==== RESTARTボタン ====
	{
		float drawW = g_RestartBtnBaseW * g_RestartBtnScale;
		float drawH = g_RestartBtnBaseH * g_RestartBtnScale;

		float testW = drawW + g_RestartHitInflateW;
		float testH = drawH + g_RestartHitInflateH;
		float cx = g_RestartBtnPos.x + g_RestartHitOffsetX;
		float cy = g_RestartBtnPos.y + g_RestartHitOffsetY;

		float halfW = testW * 0.5f;
		float halfH = testH * 0.5f;

		g_RestartBtnHover =
			(mp.x >= cx - halfW) && (mp.x <= cx + halfW) &&
			(mp.y >= cy - halfH) && (mp.y <= cy + halfH);

		const float targetScale = g_RestartBtnHover ? 1.08f : 1.0f;
		g_RestartBtnScale += (targetScale - g_RestartBtnScale) * 0.2f;

		if (g_RestartBtnHover && IsMouseLeftTriggered()) {
			SetFade(FADE_OUT, MODE_GAME);   // ← ゲームへ（※あなたの環境でMODE_GAME1なら置き換え）
			// 例: SetFade(FADE_OUT, MODE_TUTORIAL); にしたい場合はここを変更
		}
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawResult(void)
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

	// リザルトの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}



	// --- ここから追記: TITLEボタン（テクスチャ[3]）を左下に描画 ---
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]); // title02.png

		float drawW = g_TitleBtnBaseW * g_TitleBtnScale;
		float drawH = g_TitleBtnBaseH * g_TitleBtnScale;

		// ホバー時に少しだけ目立たせたいならα上げる（任意）
		float a = g_TitleBtnHover ? 1.0f : 1.0f;

		SetSpriteColor(
			g_VertexBuffer,
			g_TitleBtnPos.x, g_TitleBtnPos.y,
			drawW, drawH,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1, 1, 1, a)
		);

		GetDeviceContext()->Draw(4, 0);
	}

	// --- ここから追記: RESTARTボタン（テクスチャ[1]）を右下に描画 ---
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]); // restart.png

		float drawW = g_RestartBtnBaseW * g_RestartBtnScale;
		float drawH = g_RestartBtnBaseH * g_RestartBtnScale;

		float a = g_RestartBtnHover ? 1.0f : 1.0f;

		SetSpriteColor(
			g_VertexBuffer,
			g_RestartBtnPos.x, g_RestartBtnPos.y,
			drawW, drawH,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1, 1, 1, a)
		);

		GetDeviceContext()->Draw(4, 0);
	}


	//// リザルトのロゴを描画
	//{
	//	// テクスチャ設定
	//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

	//	// １枚のポリゴンの頂点とテクスチャ座標を設定
	//	SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);

	//	// ポリゴン描画
	//	GetDeviceContext()->Draw(4, 0);
	//}


	



}




