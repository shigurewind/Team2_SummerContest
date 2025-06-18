#include "item.h"
#include "itemDatabase.h"
#include "main.h"
#include "camera.h"


#define MAX_ITEM (128)
#define ITEM_ID_MAX (20) // アイテムIDの最大値


#define ITEM_WIDTH (40.0f)		// 
#define ITEM_HEIGHT (40.0f)	// 


typedef struct {
	XMFLOAT3	pos;        // 位置
	XMFLOAT3	scl;        // スケール
	MATERIAL	material;   // マテリアル（色）

	float		fWidth;			// 幅
	float		fHeight;		// 高さ

	int         itemID;     // アイテムID（ItemDatabaseと連携）
	BOOL		use;        // 使用中かどうか
} ITEM_OBJ;

static ITEM_OBJ				g_aItem[MAX_ITEM]; // アイテム配列

static ItemDatabase			g_ItemDB;

static BOOL					g_bAlpaTest;		// アルファテストON/OFF

static ID3D11Buffer*		g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11ShaderResourceView*	g_ItemTextures[ITEM_ID_MAX]; // アイテム用テクスチャ（ItemID最大値）



HRESULT MakeVertexItem(void);









int SetItem(XMFLOAT3 pos, int itemID)
{
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (!g_aItem[i].use)
		{
			g_aItem[i].pos = pos;
			g_aItem[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
			g_aItem[i].itemID = itemID;
			g_aItem[i].material.Diffuse = XMFLOAT4(1, 1, 1, 1);
			g_aItem[i].use = TRUE;
			return i;
		}
	}
	return -1;
}

HRESULT InitItem() 
{

	g_ItemDB = ItemDatabase();  // TODO:ヒープ領域に移動するかもしれないので注意
	InitItemTextures();         // テクスチャ読み込み

	MakeVertexItem();

	for (int CntItem = 0; CntItem < MAX_ITEM; CntItem++)
	{
		ZeroMemory(&g_aItem[CntItem].material, sizeof(g_aItem[CntItem].material));
		g_aItem[CntItem].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aItem[CntItem].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aItem[CntItem].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aItem[CntItem].fWidth = ITEM_WIDTH;
		g_aItem[CntItem].fHeight = ITEM_HEIGHT;
		g_aItem[CntItem].use = FALSE;
	}

	g_bAlpaTest = TRUE;

	SetItem(XMFLOAT3(10.0f, 0.0f, 20.0f), ITEM_APPLE); // アイテムをセット（例）


	return S_OK;

}


void UninitItem()
{
	for (int nCntTex = 0; nCntTex < ITEM_ID_MAX; nCntTex++)
	{
		if (g_ItemTextures[nCntTex] != NULL)
		{// テクスチャの解放
			g_ItemTextures[nCntTex]->Release();
			g_ItemTextures[nCntTex] = NULL;
		}
	}

	if (g_VertexBuffer != NULL)
	{// 頂点バッファの解放
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}


void UpdateItem()
{
	// アイテムの更新処理
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_aItem[i].use)
		{
			//Playerと当たり判定

		}
	}
}

void DrawItem()
{
	if (g_bAlpaTest == TRUE)
	{
		// αテストを有効に
		SetAlphaTestEnable(TRUE);
	}

	SetLightEnable(FALSE);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA* cam = GetCamera();

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_aItem[i].use)
		{
			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// ビューマトリックスを取得
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// 正方行列（直交行列）を転置行列させて逆行列を作ってる版(速い)
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];


			// スケールを反映
			mtxScl = XMMatrixScaling(g_aItem[i].scl.x, g_aItem[i].scl.y, g_aItem[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_aItem[i].pos.x, g_aItem[i].pos.y, g_aItem[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			SetMaterial(g_aItem[i].material);


			// ItemDatabaseから得たテクスチャで描画
			int texID = g_aItem[i].itemID;
			if (g_ItemTextures[texID]) {
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_ItemTextures[texID]);
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}



//
void InitItemTextures()
{
	for (int id = 0; id < ITEM_ID_MAX; ++id)
	{
		std::string path = g_ItemDB.GetTexturePath(id);
		if (path.empty()) continue;

		ID3D11ShaderResourceView* tex = nullptr;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(), path.c_str(), NULL, NULL, &tex, NULL);
		g_ItemTextures[id] = tex;
	}
}

//TODO:find out fwidth and fheight
HRESULT MakeVertexItem(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = ITEM_WIDTH;
	float fHeight = ITEM_HEIGHT;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// 法線の設定
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}