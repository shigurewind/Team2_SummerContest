#pragma once
#include "Item.h"
#include <vector>



class Inventory {

	// インベントリのカテゴリ
private:
	std::vector<Item> ammoParts;
	std::vector<Item> fireTypeParts;
	std::vector<Item> consumables;

	// インベントリの容量
	int ammoCapacity;
	int fireTypeCapacity;
	int consumableCapacity;

public:
	// コンストラクタ
	Inventory(int ammoCap = 10, int fireTypeCap = 10, int conCap = 20);

	bool AddItem(const Item& item);
	bool UseItem(int itemId, ItemCategory category, int useCount = 1);
	bool RemoveItem(int itemId, ItemCategory category);

	const std::vector<Item>& GetAmmoParts() const;
	const std::vector<Item>& GetFireTypeParts() const;
	const std::vector<Item>& GetConsumables() const;

	/// 指定カテゴリに itemId のパーツを「持っているか？」
	bool Has(ItemCategory category, int itemId) const;      // そのIDを持っている？
	/// 指定カテゴリ・itemId の所持数（0 なら未所持）
	/// 所持数（スタック合計）※パーツは基本 1
	int  Count(ItemCategory category, int itemId) const;    // スタック数（0なら未所持）
};


