//=============================================================================
//
// スコア処理 [score.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "GameUI.h"
#include "sprite.h"
#include "player.h"
#include "bullet.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(16)	// キャラサイズ
#define TEXTURE_HEIGHT				(32)	// 
#define TEXTURE_MAX					(7)		// テクスチャの数


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/number16x32.png",
	"data/2Dpicture/UI/HPbar.png",
	"data/2Dpicture/UI/HPgauge.png",
	"data/TEXTURE/revolver.png",
	"data/TEXTURE/shotgun.png",
	"data/2Dpicture/enemy/enemyWeb.png",
	"data/2Dpicture/UI/hand.png",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号

static int						g_Score;					// スコア

static BOOL						g_Load = FALSE;

int Min2(int a, int b) {
	return (a < b) ? a : b;
}

static float g_WebEffectTimer = 0.0f;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitScore(void)
{
	ID3D11Device* pDevice = GetDevice();

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


	// プレイヤーの初期化
	g_Use = TRUE;
	g_w = TEXTURE_WIDTH;
	g_h = TEXTURE_HEIGHT;
	g_Pos = { 500.0f, 20.0f, 0.0f };
	g_TexNo = 0;

	g_Score = 0;	// スコアの初期化

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitScore(void)
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
void UpdateScore(void)
{
	if (g_WebEffectTimer > 0.0f)
	{
		g_WebEffectTimer -= 0.05f / 60.0f;
		if (g_WebEffectTimer < 0.0f) g_WebEffectTimer = 0.0f;
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawScore(void)
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

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);



	PLAYER* player = GetPlayer();

	//ケージのHPバー
	{// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
		//ゲージの位置やテクスチャー座標を反映
		float pw = 280;		// ゲージの表示幅
		pw = pw * ((float)player->HP / player->HP_MAX);
		float x = ((float)player->HP / player->HP_MAX);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLeftTop(g_VertexBuffer, 2.0f, 6.0f, pw, 60, 0.0f, 0.0f, x, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	//HPのUI
	{// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, 130.0f, 30.0f, 400, 180, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	//クモの攻撃のエフェクト
	if (g_WebEffectTimer > 0.0f)
	{
		MATERIAL m = {};
		m.Diffuse = XMFLOAT4(1, 1, 1, 1);
		SetMaterial(m);

		SetWorldViewProjection2D();
		SetAlphaTestEnable(FALSE);
		SetBlendState(BLEND_MODE_ALPHABLEND);
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[5]);

		float alpha = g_WebEffectTimer; // 1.0 -> 0.0
		SetSpriteColor(g_VertexBuffer, 640.0f, 360.0f, 1277.0f, 770.0f, 0, 0, 1, 1, XMFLOAT4(1, 1, 1, alpha));

		GetDeviceContext()->Draw(4, 0);
	}


	//弾数表示の呼び出し
	DrawAmmoUI();

	//右手UIざっくり描画
	DrawHandUI();

}

//========================================================
// 武器と弾数UI表示
//========================================================
void DrawAmmoUI(void)
{
	PLAYER* player = GetPlayer();
	Weapon* weapon = (GetCurrentWeaponType() == WEAPON_REVOLVER) ? GetRevolver() : GetShotgun();

	// === 武器アイコン表示 ===
	const float weaponIconX = 1025.0f;  //表示位置
	const float weaponIconY = 610.0f;

	int weaponTexNo = (GetCurrentWeaponType() == WEAPON_REVOLVER) ? 3 : 4;
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[weaponTexNo]);

	SetSprite(g_VertexBuffer,
		weaponIconX, weaponIconY,
		60, 60,  // サイズ
		0.0f, 0.0f, 1.0f, 1.0f
	);
	GetDeviceContext()->Draw(4, 0);

	// === 弾数表示 === 
	int clipSize = weapon->clipSize;

	int ammoInClip = 0;
	int ammoSpare = 0;

	if (GetCurrentBulletType() == BULLET_NORMAL) {
		ammoInClip = Min2(player->ammoNormal, clipSize);
		ammoSpare = player->maxAmmoNormal;
	}
	else {
		ammoInClip = Min2(player->ammoFire, clipSize);
		ammoSpare = player->maxAmmoFire;
	}

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));

	if (GetCurrentBulletType() == BULLET_FIRE) {
		material.Diffuse = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);  // 🔴 赤
	}
	else {
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  // ⚪ 白（ノーマル弾）
	}

	SetMaterial(material);

	// 数字スプライト設定（0〜9が1列に並んでいる）
	const float digitWidth = 16.0f;
	const float digitHeight = 32.0f;

	const float baseX = 1000.0f;  // 表示位置（右下に調整）
	const float baseY = 650.0f;

	char text[16];
	sprintf(text, "%d/%d", ammoInClip, ammoSpare);

	// 数字を1文字ずつ描画
	for (int i = 0; text[i] != '\0'; ++i) {
		char c = text[i];
		if (c == '/') {
			continue;  // スラッシュは今は表示しない（必要なら別途テクスチャ用意）
		}

		int n = c - '0';
		if (n < 0 || n > 9) continue;

		float u = (n % 10) / 10.0f;
		float v = 0.0f;
		float uw = 1.0f / 10.0f;
		float vh = 1.0f;

		SetSpriteLeftTop(
			g_VertexBuffer,
			baseX + i * digitWidth, baseY,
			digitWidth, digitHeight,
			u, v, uw, vh
		);

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
		GetDeviceContext()->Draw(4, 0);
	}
}


//=============================================================================
// 右手のUI
//=============================================================================
void DrawHandUI(void)
{
	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[6]);
	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSprite(g_VertexBuffer, 800.0f, 700.0f, 500.0f, 500.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}





//=============================================================================
// スコアを加算する
// 引数:add :追加する点数。マイナスも可能
//=============================================================================
void AddScore(int add)
{
	g_Score += add;
	if (g_Score > SCORE_MAX)
	{
		g_Score = SCORE_MAX;
	}

}


int GetScore(void)
{
	return g_Score;
}

//=============================================================================
// 蜘蛛のネット効果（画面に表示）を一定時間見せる関数
//=============================================================================
void ShowWebEffect(float time)
{
	g_WebEffectTimer = time; // time 秒間、画面に蜘蛛のネットを表示
}
