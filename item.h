#pragma once
#include <string>

enum class ItemCategory {
	WeaponPart_Ammo,		//弾の種類を決めるパーツ
	WeaponPart_FireType,	//打ち方を決めるパーツ
	Consumable,				//消耗品
};

struct Item {
	int id;                 //唯一無二のID
	std::string name;       //名前
	int count;              //所持数(このアイテムの数、インベントリーの中のではない)?
	ItemCategory category;  //アイテムのカテゴリ

	Item(int id, const std::string& name, int count, ItemCategory category)
		: id(id), name(name), count(count), category(category) {
	}
};

// アイテムIDの定義
enum ItemID
{
	//弾の種類を決めるパーツ

	//打ち方を決めるパーツ

	//消耗品
	ITEM_APPLE,
};