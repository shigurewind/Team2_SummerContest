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

	bool isInstantEffect;

	Item(int id, const std::string& name, int count, ItemCategory category, bool isInstant = false)
		: id(id), name(name), count(count), category(category) , isInstantEffect(isInstant) {
	}
};



// アイテムIDの定義
enum ItemID
{
	//弾の種類を決めるパーツ

	//打ち方を決めるパーツ

	//消耗品
	ITEM_APPLE,

	//回復品
	ITEM_SAN,
	ITEM_BULLET,
};


int SetItem(XMFLOAT3 pos, int itemID);

HRESULT InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();



