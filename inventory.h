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
	bool RemoveItem(int itemId, ItemCategory category);

	const std::vector<Item>& GetAmmoParts() const;
	const std::vector<Item>& GetFireTypeParts() const;
	const std::vector<Item>& GetConsumables() const;
};


