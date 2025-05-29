#include "inventory.h"

Inventory::Inventory(int ammoCap, int fireTypeCap, int conCap)
	: ammoCapacity(ammoCap), fireTypeCapacity(fireTypeCap), consumableCapacity(conCap) {
}


//新しいアイテムを追加する関数
bool Inventory::AddItem(const Item& item) {
	std::vector<Item>* target = nullptr;
	int* capacity = nullptr;

	switch (item.category) {
	case ItemCategory::WeaponPart_Ammo:
		target = &ammoParts;
		capacity = &ammoCapacity;
		break;
	case ItemCategory::WeaponPart_FireType:
		target = &fireTypeParts;
		capacity = &fireTypeCapacity;
		break;
	case ItemCategory::Consumable:
		target = &consumables;
		capacity = &consumableCapacity;
		break;
	}

	// 同じIDのアイテムがあるか確認（スタックする）
	for (auto& i : *target) {
		if (i.id == item.id) {
			i.count += item.count;
			return true; // 既存アイテムに加算して終了
		}
	}

	// 新規スロットとして追加（容量チェック）
	if ((int)target->size() < *capacity) {
		target->push_back(item);
		return true;
	}

	return false; // 容量オーバー
}


// アイテムを削除する関数
bool Inventory::RemoveItem(int itemId, ItemCategory category) {
	std::vector<Item>* target = nullptr;

	// カテゴリに応じてターゲットを設定
	switch (category) {
	case ItemCategory::WeaponPart_Ammo:      target = &ammoParts; break;
	case ItemCategory::WeaponPart_FireType:  target = &fireTypeParts; break;
	case ItemCategory::Consumable:           target = &consumables; break;
	}

	// アイテムを検索して削除
	for (auto it = target->begin(); it != target->end(); ++it) {
		if (it->id == itemId) {
			target->erase(it);
			return true;
		}
	}

	// アイテムが見つからなかった場合
	return false;
}



const std::vector<Item>& Inventory::GetAmmoParts() const {
	return ammoParts;
}

const std::vector<Item>& Inventory::GetFireTypeParts() const {
	return fireTypeParts;
}

const std::vector<Item>& Inventory::GetConsumables() const {
	return consumables;
}

