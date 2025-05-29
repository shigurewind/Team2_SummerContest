#pragma once
#include "Item.h"
#include <vector>



class Inventory {

	// �C���x���g���̃J�e�S��
private:
	std::vector<Item> ammoParts;
	std::vector<Item> fireTypeParts;
	std::vector<Item> consumables;

	// �C���x���g���̗e��
	int ammoCapacity;
	int fireTypeCapacity;
	int consumableCapacity;

public:
	// �R���X�g���N�^
	Inventory(int ammoCap = 10, int fireTypeCap = 10, int conCap = 20);

	bool AddItem(const Item& item);
	bool RemoveItem(int itemId, ItemCategory category);

	const std::vector<Item>& GetAmmoParts() const;
	const std::vector<Item>& GetFireTypeParts() const;
	const std::vector<Item>& GetConsumables() const;
};


