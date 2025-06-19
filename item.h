#pragma once
#include <string>
#include "renderer.h"

enum class ItemCategory {
	WeaponPart_Ammo,		//弾の種類を決めるパーツ
	WeaponPart_FireType,	//打ち方を決めるパーツ
	Consumable,				//消耗品
	InstantEffect,		//即時効果のアイテム
};

struct Item {
	int id;                 //唯一無二のID
	std::string name;       //名前
	int count;              //所持数(このアイテムの数、インベントリーの中のではない)?
	ItemCategory category;  //アイテムのカテゴリ


	Item(int id = 0, const std::string& name = "unknown", int count = 1, ItemCategory category = ItemCategory::Consumable)
		: id(id), name(name), count(count), category(category) {
	}
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

HRESULT InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();

Item CreateItemFromID(int id);



