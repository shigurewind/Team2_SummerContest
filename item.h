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

//struct Item {
//	int id;                 //唯一無二のID
//	std::string name;       //名前
//	int count;              //所持数(このアイテムの数、インベントリーの中のではない)?
//	ItemCategory category;  //アイテムのカテゴリ
//
//
//	Item(int id = 0, const std::string& name = "unknown", int count = 1, ItemCategory category = ItemCategory::Consumable)
//		: id(id), name(name), count(count), category(category) {
//	}
//};

class Item : public Object
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

//typedef struct {
//	XMFLOAT3	pos;        // 位置
//	XMFLOAT3	scl;        // スケール
//	MATERIAL	material;   // マテリアル（色）
//
//	float		fWidth;			// 幅
//	float		fHeight;		// 高さ
//
//
//	Item		item;
//	BOOL		use;        // 使用中かどうか
//
//	float		timeOffset;
//	float		basePosY;
//} ITEM_OBJ;

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

	void SetBasePosY(float y) { basePosY = y; }

private:
	Item item;
	XMFLOAT3 scl;
	MATERIAL material;
	float width, height;
	bool use;
	float basePosY;
	float timeOffset;
};


// アイテムIDの定義
enum ItemID
{
	//弾の種類を決めるパーツ
	PART_FIRE,

	//打ち方を決めるパーツ
	PART_SHUTGUN,

	//消耗品
	ITEM_APPLE,

	//回復品
	ITEM_SAN,
	ITEM_BULLET,

	ITEM_ID_COUNT//ItemIDの数
};


int SetItem(XMFLOAT3 pos, int itemID);

void InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();

Item CreateItemFromID(int id);
int SetItem(XMFLOAT3 pos, int itemID);

void SaveItemData(const std::string& filename);
void LoadItemData(const std::string& filename);


ITEM_OBJ* GetItemOBJ();
