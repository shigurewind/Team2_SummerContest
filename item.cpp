#include "item.h"
#include "itemDatabase.h"
#include "main.h"
#include "camera.h"
#include "collision.h"
#include "player.h"
#include "FBXmodel.h"

#include <cstdlib> // for rand()
#include <ctime>   // for time()

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


#define MAX_ITEM (128)
#define ITEM_ID_MAX ITEM_ID_COUNT // アイテムIDの最大値

#define ITEM_FLOAT_OFFSET (1.0f)
#define ITEM_FLOAT_FREQUENCE (3.0f)


#define ITEM_WIDTH (20.0f)		// 
#define ITEM_HEIGHT (20.0f)	// 

#define ITEM_SIZE (20.0f)

#define ITEM_OFFSET_Y (50.0f)



static ITEM_OBJ			g_aItem[MAX_ITEM]; // アイテム配列

static ItemDatabase			g_ItemDB;

static BOOL					g_bAlpaTest;		// アルファテストON/OFF

static ID3D11Buffer* g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11ShaderResourceView* g_ItemTextures[ITEM_ID_MAX]; // アイテム用テクスチャ（ItemID最大値）

static float g_ItemGlobalTime = 0.0f;





HRESULT MakeVertexItem(void);



ITEM_OBJ::ITEM_OBJ()
	: scl({ 1.0f, 1.0f, 1.0f }),
	material{},
	width(ITEM_WIDTH),
	height(ITEM_HEIGHT),
	use(false),
	basePosY(0.0f),
	timeOffset(0.0f),
	hasLanded(false),
	renderPos(XMFLOAT3(0.0f, 0.0f, 0.0f))
{
	material.Diffuse = XMFLOAT4(1, 1, 1, 1);
	EnableGravity(true);
}

void ITEM_OBJ::SetItem(const Item& item_)
{
	item = item_;
	timeOffset = static_cast<float>(rand()) / RAND_MAX * XM_2PI;
}

void ITEM_OBJ::Update()
{
	if (!use) return;

	Object::Update(); // 重力
	HandleGroundCheck(); // 地面判定

	// すべり止め：接地している間は水平速度に摩擦をかける
	{
		XMFLOAT3 v = GetVelocity();

		if (isGround) {
			const float groundFriction = 0.85f; // 0?1（小さいほど早く止まる）
			const float stopEps = 0.05f;  // これ未満は0に丸める

			v.x *= groundFriction;
			v.z *= groundFriction;

			if (fabsf(v.x) < stopEps) v.x = 0.0f;
			if (fabsf(v.z) < stopEps) v.z = 0.0f;

			// 縦は既に HandleGroundCheck() で 0 にされるが、念のため保持
			// v.y は変更しない（地形の段差で浮き直す可能性があるため）
		}
		else {
			// （任意）空中はほんの少しだけ空気抵抗をかけてもOK
			// v.x *= 0.99f; v.z *= 0.99f;
		}

		SetVelocity(v);
	}

	renderPos = pos;

	if (isGround && !hasLanded) {
		// 着地、初期位置を設定
		basePosY = pos.y;
		hasLanded = true;
	}

	// 浮遊アニメーション
	if (hasLanded && isGround) {
		float t = g_ItemGlobalTime + timeOffset;
		renderPos.y = basePosY + sinf(t) * ITEM_FLOAT_OFFSET;
	}


	// 当たり判定
	if (CollisionBC(pos, GetPlayer()->GetPosition(), ITEM_SIZE, GetPlayer()->size)) {
		
		Inventory* playerInventory = GetPlayerInventory();

		switch (item.GetCategory())
		{
		case ItemCategory::WeaponPart_Ammo:
		case ItemCategory::WeaponPart_FireType:
		case ItemCategory::Consumable:
			//インベントリーに入れる
			if (playerInventory->AddItem(item)) {
				

				use = false;  // アイテムを消す
			}
			break;
		case ItemCategory::InstantEffect:
			//相応の効果
			ApplyInstantItemEffect(item.GetID());
			use = false;
			break;
		default:
			break;
		}
	}
}

void ITEM_OBJ::HandleGroundCheck()
{
	const float groundThreshold = 0.2f;
	float groundY;
	if (CheckItemGroundSimple(pos, ITEM_OFFSET_Y, groundY) && velocity.y <= 0.0f)
	{
		float targetY = groundY;
		float distanceToGround = pos.y - targetY;
		if (distanceToGround <= groundThreshold)
		{
			pos.y = targetY;
			velocity.y = 0.0f;
			isGround = true;
		}
		else
		{
			isGround = false;
		}
	}
	else
	{
		isGround = false;
	}
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
	g_ItemGlobalTime += ITEM_FLOAT_FREQUENCE / 60.0f;
	for (int i = 0; i < MAX_ITEM; i++)
		g_aItem[i].Update();
}





int SpawnItem(XMFLOAT3 pos, int itemID)
{
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (!g_aItem[i].IsUsed())
		{
			Item item = CreateItemFromID(itemID);
			g_aItem[i].SetItem(item);
			g_aItem[i].SetUsed(true);
			g_aItem[i].SetPosition(pos);
			g_aItem[i].SetBasePosY(pos.y);
			return i;
		}
	}
	return -1;
}


HRESULT InitItem()
{

	srand((unsigned int)time(nullptr));

	g_ItemDB = ItemDatabase();  // TODO:ヒープ領域に移動するかもしれないので注意
	InitItemTextures();         // テクスチャ読み込み

	MakeVertexItem();

	for (int CntItem = 0; CntItem < MAX_ITEM; CntItem++)
	{
		//ZeroMemory(&g_aItem[CntItem].material, sizeof(g_aItem[CntItem].material));
		//g_aItem[CntItem].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		MATERIAL mat = {};
		mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aItem[CntItem].SetMaterial(mat);


		g_aItem[CntItem].SetUsed(false);
		g_aItem[CntItem].SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		g_aItem[CntItem].SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
		g_aItem[CntItem].SetWidth(ITEM_WIDTH);
		g_aItem[CntItem].SetHeight(ITEM_HEIGHT);

	}

	g_bAlpaTest = TRUE;

	SpawnItem(XMFLOAT3(30.0f, 0.0f, 20.0f), ITEM_APPLE); // アイテムをセット（例）
	SpawnItem(XMFLOAT3(50.0f, 0.0f, 0.0f), ITEM_SAN); // アイテムをセット（例）



	return S_OK;

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
		if (g_aItem[i].IsUsed())
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
			XMFLOAT3 scl = g_aItem[i].GetScale();
			mtxScl = XMMatrixScaling(scl.x,scl.y,scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 移動を反映
			XMFLOAT3 pos = g_aItem[i].GetRenderPosition();
			mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);

			SetMaterial(g_aItem[i].GetMaterial());


			// ItemDatabaseから得たテクスチャで描画
			int texID = g_aItem[i].GetItem().GetID();
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

Item CreateItemFromID(int id) {
	switch (id) {
	case ITEM_SPEED_UP:
		return Item(id, "Speed Up", 1, ItemCategory::Consumable);
	case ITEM_APPLE:
		return Item(id, "Apple", 1, ItemCategory::Consumable);

	case ITEM_SAN:
		return Item(id, "San", 1, ItemCategory::InstantEffect);
	case ITEM_BULLET:
		return Item(id, "Bullet", 10, ItemCategory::InstantEffect);

	default:
		return Item(id, "Unknown", 1, ItemCategory::Consumable);
	}
}




void SaveItemData(const std::string& filename)
{
	json j = json::array();
	for (int i = 0; i < MAX_ITEM; ++i)
	{
		if (g_aItem[i].IsUsed())
		{
			Item& item = g_aItem[i].GetItem();
			XMFLOAT3 pos = g_aItem[i].GetPosition();
			XMFLOAT3 scl = g_aItem[i].GetScale();

			json itemObj;
			itemObj["id"] = item.GetID();
			itemObj["pos"] = { pos.x, pos.y, pos.z };
			itemObj["scl"] = { scl.x, scl.y, scl.z };
			j.push_back(itemObj);
		}
	}

	std::ofstream file(filename);
	file << j.dump(4);
}

void LoadItemData(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file) return;

	json j;
	file >> j;

	for (int i = 0; i < MAX_ITEM; ++i)
		g_aItem[i].SetUsed(false);

	for (const auto& itemObj : j)
	{
		int id = itemObj["id"];
		XMFLOAT3 pos = XMFLOAT3(itemObj["pos"][0], itemObj["pos"][1], itemObj["pos"][2]);
		int index = SpawnItem(pos, id);
		if (index >= 0)
		{
			XMFLOAT3 scl = XMFLOAT3(itemObj["scl"][0], itemObj["scl"][1], itemObj["scl"][2]);
			g_aItem[index].SetScale(scl);
		}
	}
}

ITEM_OBJ* GetItemOBJ()
{
	return g_aItem;;
}

int GetItemCount() { return MAX_ITEM; }


bool CheckItemGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY)
{
	const auto& tris = GetFloorTriangles();

	XMFLOAT3 rayStart = pos;
	rayStart.y += 50.0f;
	XMFLOAT3 rayEnd = pos;
	rayEnd.y -= 100.0f;

	XMFLOAT3 hit, normal;
	for (const auto& tri : tris)
	{
		if (RayCast(tri.v0, tri.v1, tri.v2, rayStart, rayEnd, &hit, &normal))
		{
			groundY = hit.y;
			return true;
		}
	}
	return false;
}



// 即時効果を持つアイテムの効果を適用する関数
void ApplyInstantItemEffect(int itemID) {
	PLAYER* player = GetPlayer();

	switch (itemID) {
	case ITEM_SAN:
		// HPを回復
		player->HP += 1.0f;//TODO：数値調整
		if (player->HP > player->HP_MAX) {
			player->HP = player->HP_MAX;  
		}
		break;

	case ITEM_BULLET:
		// 弾数補充
		player->ammoNormal += 5;//TODO：数値調整
		if (player->ammoNormal > player->maxAmmoNormal) {
			player->ammoNormal = player->maxAmmoNormal;  
		}
		break;

		
	default:
		
		break;
	}
}

// 消費アイテムの効果を適用する関数
void ApplyConsumableItemEffect(int itemID) {
	PLAYER* player = GetPlayer();

	switch (itemID) {
	case ITEM_APPLE:
		// HPを回復
		player->HP += 2.0f;  // TODO：数値調整
		if (player->HP > player->HP_MAX) {
			player->HP = player->HP_MAX;
		}
		break;

	case ITEM_SPEED_UP:
		// Speed Up
		player->speed += 1.0f;  // TODO：数値調整(永久？)
		break;

	default:
		break;
	}
}


//Item使用
void UseCurrentItem() {
	PLAYER* player = GetPlayer();
	Inventory* inventory = &(player->inventory);
	const std::vector<Item>& consumables = inventory->GetConsumables();

	if (consumables.empty()) return;  // アイテムがない

	// インデックスの範囲をチェック
	if (player->currentConsumableIndex >= (int)consumables.size()) {
		player->currentConsumableIndex = 0;
	}

	if (player->currentConsumableIndex < 0) {
		player->currentConsumableIndex = (int)consumables.size() - 1;
	}

	// 今のアイテムを取得
	const Item& currentItem = consumables[player->currentConsumableIndex];

	// 応用
	ApplyConsumableItemEffect(currentItem.GetID());

	// インベントリーから数を減らす
	inventory->UseItem(currentItem.GetID(), ItemCategory::Consumable, 1);

	// インデックスを調整
	const std::vector<Item>& newConsumables = inventory->GetConsumables();
	if (newConsumables.empty()) {
		player->currentConsumableIndex = 0;
	}
	else if (player->currentConsumableIndex >= (int)newConsumables.size()) {
		player->currentConsumableIndex = (int)newConsumables.size() - 1;
	}
}


//先のItem切り替える
void SwitchToPreviousItem() {
	PLAYER* player = GetPlayer();
	const std::vector<Item>& consumables = player->inventory.GetConsumables();

	if (consumables.empty()) return;

	player->currentConsumableIndex--;
	if (player->currentConsumableIndex < 0) {
		player->currentConsumableIndex = (int)consumables.size() - 1;
	}
}

//次のItem切り替える
void SwitchToNextItem() {
	PLAYER* player = GetPlayer();
	const std::vector<Item>& consumables = player->inventory.GetConsumables();

	if (consumables.empty()) return;

	player->currentConsumableIndex++;
	if (player->currentConsumableIndex >= (int)consumables.size()) {
		player->currentConsumableIndex = 0;
	}
}



// アイテムIDに対応するテクスチャを取得
ID3D11ShaderResourceView* GetItemTexture(int itemID) {
	if (itemID >= 0 && itemID < ITEM_ID_MAX) {
		return g_ItemTextures[itemID];
	}
	return nullptr;  // 無効なIDの場合はnullptrを返す
}







