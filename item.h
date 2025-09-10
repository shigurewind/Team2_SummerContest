#pragma once
#include <string>
#include "renderer.h"
#include "object.h"

enum class ItemCategory {
	WeaponPart_Ammo,		//弾の種類を決めるパーツ
	WeaponPart_FireType,	//打ち方を決めるパーツ
	Consumable,				//消耗品
	InstantEffect,		//即時効果のアイテム
};



class Item
{
public:
	Item(int id = 0, const std::string& name = "unknown", int count = 1, ItemCategory category = ItemCategory::Consumable)
		: id(id), name(name), count(count), category(category) {
	}

	int GetID() const { return id; }
	const std::string& GetName() const { return name; }
	ItemCategory GetCategory() const { return category; }

	int GetCount() const { return count; }
	void SetCount(int c) { count = c; }



private:
	int id;
	std::string name;
	int count;
	ItemCategory category;
};



class ITEM_OBJ : public Object
{

public:
	ITEM_OBJ();
	void Update();
	//void Draw();
	void SetItem(const Item& item);
	bool IsUsed() const { return use; }
	void SetUsed(bool b) { use = b; }
	Item& GetItem() { return item; }

	void SetScale(const XMFLOAT3& scale) { scl = scale; }
	XMFLOAT3 GetScale() const { return scl; }

	void SetMaterial(const MATERIAL& mat) { material = mat; }
	const MATERIAL& GetMaterial() const { return material; }

	void SetWidth(float w) { width = w; }
	void SetHeight(float h) { height = h; }
	float GetWidth() const { return width; }
	float GetHeight() const { return height; }



	void SetBasePosY(float y) { basePosY = y; }

	XMFLOAT3 GetRenderPosition() const { return renderPos; }

	void HandleGroundCheck();
	void ExplodeBug();

	float bugTimer;


	void SetSleeping(bool s) { sleeping = s; }
	bool IsSleeping() const { return sleeping; }

private:
	Item item;
	XMFLOAT3 scl;
	MATERIAL material;
	float width, height;
	bool use;
	float basePosY;
	float timeOffset;

	bool hasLanded;// 着地したかのフラグ
	XMFLOAT3 renderPos;


	bool sleeping = false;
	//当たり判定
	void ApplyCollision();
	void ApplyFriction();// 摩擦力

};


// 全てのアイテムIDの定義
enum ItemID
{
	//弾の種類を決めるパーツ
	PART_NORMAL_AMMO,
	PART_FIRE,

	//打ち方を決めるパーツ
	PART_REVOLVER,
	PART_SHUTGUN,
	PART_ROCKET,

	//消耗品
	ITEM_APPLE,
	ITEM_SPEED_UP,

	//回復品
	ITEM_SAN,
	ITEM_BULLET,

	ITEM_BUG,

	ITEM_ID_COUNT//ItemIDの数
};


int SpawnItem(XMFLOAT3 pos, int itemID);

HRESULT InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();

Item CreateItemFromID(int id);

void ClearAllItems();
void SaveItemData(const std::string& filename);
void LoadItemData(const std::string& filename);


ITEM_OBJ* GetItemOBJ();

//InstantEffectアイテムの効果を適用関数
void ApplyInstantItemEffect(int itemID);

//Consumableアイテムの効果を適用関数
void ApplyConsumableItemEffect(int itemID);
void UseCurrentItem();
void SwitchToPreviousItem();
void SwitchToNextItem();

ID3D11ShaderResourceView* GetItemTexture(int itemID);

// アイテム配列の有効長（MAX_ITEM）を外から取得するために公開
int GetItemCount();
