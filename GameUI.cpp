//=============================================================================
//
// スコア処理 [GameUI.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "GameUI.h"
#include "sprite.h"
#include "player.h"
#include "bullet.h"
#include "item.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(16)	// キャラサイズ
#define TEXTURE_HEIGHT				(32)	// 
#define TEXTURE_MAX					(13)		// テクスチャの数


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
	"data/TEXTURE/HP00.png",
	"data/TEXTURE/HP01.png",
	"data/TEXTURE/revolver.png",
	"data/TEXTURE/shotgun.png",
	"data/2Dpicture/enemy/enemyWeb.png",
	"data/2Dpicture/UI/item_slot.png",
	"data/TEXTURE/rocket_launcher.png",
	"data/2Dpicture/enemy/bug02.png",
	"data/TEXTURE/cross.png",
	"data/TEXTURE/paused.png",

	"data/2Dpicture/UI/shooting.png",
	"data/2Dpicture/UI/crosshair.png",
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

BOOL g_BugEffectActive = FALSE;
float bugEffectTimer = 0.0f;

static float g_UIRecoilY = 0.0f;       // 現在のリコイル量
static float g_UIRecoilRecover = 2.0f; // リコイル回復速度


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGameUI(void)
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
void UnInitGameUI(void)
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
void UpdateGameUI(void)
{
	if (g_WebEffectTimer > 0.0f)
	{
		g_WebEffectTimer -= 0.05f / 60.0f;
		if (g_WebEffectTimer < 0.0f) g_WebEffectTimer = 0.0f;
	}

	if (g_BugEffectActive == TRUE)
	{
		bugEffectTimer += 1.0f / 60.0f;

		if (bugEffectTimer >= 2.0f)
		{
			GetPlayer()->HP -= 1;
			if (GetPlayer()->HP < 0) GetPlayer()->HP = 0;

			bugEffectTimer = 0.0f;
		}
	}


	// --- UIリコイルを徐々に回復 ---
	if (g_UIRecoilY < 0.0f) {
		g_UIRecoilY += g_UIRecoilRecover;
		if (g_UIRecoilY > 0.0f) g_UIRecoilY = 0.0f;
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGameUI(void)
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
	DrawHPBar();

	//HPのUI
	DrawHP();

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

	if (g_BugEffectActive)
	{
		MATERIAL m = {};
		m.Diffuse = XMFLOAT4(1, 1, 1, 1);
		SetMaterial(m);

		SetWorldViewProjection2D();
		SetAlphaTestEnable(FALSE);
		SetBlendState(BLEND_MODE_ALPHABLEND);

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[8]);

		SetSpriteColor(
			g_VertexBuffer,
			640.0f, 360.0f,
			1277.0f, 770.0f,
			0, 0, 1, 1,
			XMFLOAT4(1, 1, 1, 1)
		);

		GetDeviceContext()->Draw(4, 0);
	}


	//弾数表示の呼び出し
	DrawAmmoUI();

	//選択中のアイテム表示
	DrawItemSlot();

	DrawShootingHand();

}


void DrawHPBar()
{
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
	//ゲージの位置やテクスチャー座標を反映
	float pw = 280;		// ゲージの表示幅
	pw = pw * ((float)GetPlayer()->HP / GetPlayer()->HP_MAX);
	float x = ((float)GetPlayer()->HP / GetPlayer()->HP_MAX);

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteLeftTop(g_VertexBuffer, 2.0f, 6.0f, pw, 60, 0.0f, 0.0f, x, 1.0f);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}

void DrawHP()
{
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSprite(g_VertexBuffer, 130.0f, 30.0f, 400, 180, 0.0f, 0.0f, 1.0f, 1.0f);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);
}

void DrawShootingHand()
{
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[11]);

	float drawY = SCREEN_HEIGHT - 385 + g_UIRecoilY;
	SetSprite(g_VertexBuffer, SCREEN_CENTER_X + 240, drawY, 800, 800, 0.0f, 0.0f, 1.0f, 1.0f);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);


	//crosshair
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[12]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, SCREEN_CENTER_X+10, SCREEN_CENTER_Y-5, 50, 50, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
}

//========================================================
// 武器と弾数UI表示
//========================================================
void DrawAmmoUI(void)
{
	PLAYER* player = GetPlayer();
	Weapon* weapon = nullptr;
	int weaponTexNo = 0;

	switch (GetCurrentWeaponType()) {
	case WEAPON_REVOLVER:
		weapon = GetRevolver();
		weaponTexNo = 3;  // revolver.png
		break;
	case WEAPON_SHOTGUN:
		weapon = GetShotgun();
		weaponTexNo = 4;  // shotgun.png
		break;
	case WEAPON_ROCKET_LAUNCHER:
		weapon = GetRocket_Launcher();
		weaponTexNo = 7;  // rocket_launcher.png
		break;
	}

	// === 武器アイコン表示（現状維持） ===
	const float weaponIconX = 1025.0f;
	const float weaponIconY = 610.0f;
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[weaponTexNo]);
	SetSprite(g_VertexBuffer, weaponIconX, weaponIconY, 90, 60, 0.0f, 0.0f, 1.0f, 1.0f);
	GetDeviceContext()->Draw(4, 0);

	// === 追加：未所持なら「武器アイコン」に × を重ねる ===
	{
		const bool weaponUnlocked = player->IsWeaponUnlocked((int)GetCurrentWeaponType());
		if (!weaponUnlocked)
		{
			MATERIAL m = {};
			m.Diffuse = XMFLOAT4(1, 1, 1, 0.9f); // 半透明
			SetMaterial(m);

			// cross.png（9枚目＝index 8）を武器アイコンと同じ位置に重ね描き
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[9]);
			SetSprite(g_VertexBuffer, weaponIconX, weaponIconY, 90, 60, 0, 0, 1, 1);
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// === 弾数表示：総弾数のみ（現状維持） ===
	int currentAmmo = (GetCurrentBulletType() == BULLET_NORMAL)
		? player->ammoNormal
		: player->ammoFire;

	// その弾種を未所持なら表示は 0 にする
	if (!player->IsBulletUnlocked((int)GetCurrentBulletType())) {
		currentAmmo = 0;
	}

	// 弾種の色（既存のまま）
	MATERIAL material = {};
	if (GetCurrentBulletType() == BULLET_FIRE) {
		material.Diffuse = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);  // 赤
	}
	else {
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  // 白
	}
	SetMaterial(material);

	// 数字だけ描画（“/”やクリップは廃止）
	const float digitWidth = 16.0f;
	const float digitHeight = 32.0f;
	const float baseX = 1020.0f;
	const float baseY = 650.0f;

	char text[16];
	sprintf(text, "%d", currentAmmo);

	for (int i = 0; text[i] != '\0'; ++i) {
		int n = text[i] - '0';
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

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]); // number16x32.png
		GetDeviceContext()->Draw(4, 0);
	}

}

// 画面中央に "PAUSED" 画像を出す
void DrawPaused(void)
{
	// 半透明で少しだけ目立たせたい場合はここで色・アルファ調整
	MATERIAL m = {};
	m.Diffuse = XMFLOAT4(1, 1, 1, 1); // 不透明でOK
	SetMaterial(m);

	// 2D 描画設定
	SetWorldViewProjection2D();
	SetAlphaTestEnable(FALSE);
	SetBlendState(BLEND_MODE_ALPHABLEND);

	// 「paused.png」を読み込んだインデックス（上で TEXTURE_MAX を 11 にして配列末尾に追加したので index=10）
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[10]);

	// 画面中央に表示（解像度 1280x720 想定）
	const float w = 300.0f;
	const float h = 120.0f;
	const float cx = 640.0f;
	const float cy = 360.0f;

	SetSprite(g_VertexBuffer, cx, cy, w, h, 0, 0, 1, 1);
	GetDeviceContext()->Draw(4, 0);
}

//=============================================================================
// 蜘蛛のネット効果（画面に表示）を一定時間見せる関数
//=============================================================================
void ShowWebEffect(float time)
{
	g_WebEffectTimer = time; // time 秒間、画面に蜘蛛のネットを表示
}

//=============================================================================
// 肉虫の効果（画面に表示
//=============================================================================
void ShowBugEffect()
{
	g_BugEffectActive = TRUE;
}

void HideBugEffect()
{
	g_BugEffectActive = FALSE;
}

//=============================================================================
// 発砲時にUIをリコイルさせる
//=============================================================================

void AddUIRecoil()
{
	g_UIRecoilY = -15.0f; // 上方向に15px移動
}


//=============================================================================
// 選択中のアイテム描画
//=============================================================================

void DrawItemSlot(void)
{
	const float slotX = 80.0f;   // 位置
	const float slotY = 620.0f;
	const float slotSize = 64.0f; // サイズ

	// アイテムスロットの枠を描画
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[6]); // item_slot

	MATERIAL slotMaterial;
	ZeroMemory(&slotMaterial, sizeof(slotMaterial));
	slotMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.8f); 
	SetMaterial(slotMaterial);

	SetSprite(g_VertexBuffer, slotX, slotY, slotSize, slotSize, 0.0f, 0.0f, 1.0f, 1.0f);
	GetDeviceContext()->Draw(4, 0);

	// 今のアイテムを取得して描画
	Inventory* inventory = GetPlayerInventory();
	const std::vector<Item>& consumables = inventory->GetConsumables();

	if (!consumables.empty()) {
		PLAYER* player = GetPlayer();
		int currentIndex = player->currentConsumableIndex;

		// 有効確認
		if (currentIndex >= 0 && currentIndex < (int)consumables.size()) {
			const Item& currentItem = consumables[currentIndex];

			// アイテムアイコンを描画
			ID3D11ShaderResourceView* itemTexture = GetItemTexture(currentItem.GetID());
			if (itemTexture) {
				
				MATERIAL itemMaterial;
				ZeroMemory(&itemMaterial, sizeof(itemMaterial));
				itemMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				SetMaterial(itemMaterial);

				// アイテムアイコンを枠内に収めて描画
				const float iconSize = slotSize * 0.8f;
				const float iconX = slotX + (slotSize - iconSize) * 0.5f;
				const float iconY = slotY + (slotSize - iconSize) * 0.5f;

				GetDeviceContext()->PSSetShaderResources(0, 1, &itemTexture);
				SetSprite(g_VertexBuffer, iconX, iconY, iconSize, iconSize, 0.0f, 0.0f, 1.0f, 1.0f);
				GetDeviceContext()->Draw(4, 0);
			}

			// 数を表示
			if (currentItem.GetCount() > 1) { 
				MATERIAL countMaterial;
				ZeroMemory(&countMaterial, sizeof(countMaterial));
				countMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 黄色
				SetMaterial(countMaterial);

				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]); 

				int count = currentItem.GetCount();
				if (count > 99) count = 99; // MAX99まで表示

				 
				if (count >= 10) {
					
					int tensDigit = count / 10;
					float u1 = (tensDigit % 10) / 10.0f;
					float digitSize = 10.0f;

					SetSpriteLeftTop(g_VertexBuffer,
						slotX + slotSize - digitSize * 2 - 4, 
						slotY + slotSize - digitSize - 4,
						digitSize, digitSize,
						u1, 0.0f, 0.1f, 1.0f);
					GetDeviceContext()->Draw(4, 0);

					
					int onesDigit = count % 10;
					float u2 = (onesDigit % 10) / 10.0f;

					SetSpriteLeftTop(g_VertexBuffer,
						slotX + slotSize - digitSize - 4,
						slotY + slotSize - digitSize - 4,
						digitSize, digitSize,
						u2, 0.0f, 0.1f, 1.0f);
					GetDeviceContext()->Draw(4, 0);
				}
				else {
					
					float u = (count % 10) / 10.0f;
					float digitSize = 14.0f;

					SetSpriteLeftTop(g_VertexBuffer,
						slotX + slotSize - digitSize - 4,
						slotY + slotSize - digitSize - 4,
						digitSize, digitSize,
						u, 0.0f, 0.1f, 1.0f);
					GetDeviceContext()->Draw(4, 0);
				}
			}
			
			
		}
	}

	
	MATERIAL defaultMaterial;
	ZeroMemory(&defaultMaterial, sizeof(defaultMaterial));
	defaultMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(defaultMaterial);
}