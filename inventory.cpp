#include "inventory.h"

Inventory::Inventory(int ammoCap, int fireTypeCap, int conCap)
	: ammoCapacity(ammoCap), fireTypeCapacity(fireTypeCap), consumableCapacity(conCap) {
}


//新しいアイテムを追加する関数
bool Inventory::AddItem(const Item& item) {
	std::vector<Item>* target = nullptr;
	int* capacity = nullptr;

	switch (item.GetCategory()) {
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
		if (i.GetID() == item.GetID()) {
			i.SetCount(i.GetCount() + item.GetCount());
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

// アイテムを使用する関数
bool Inventory::UseItem(int itemId, ItemCategory category, int useCount) {
	std::vector<Item>* target = nullptr;

	switch (category) {
	case ItemCategory::WeaponPart_Ammo:      target = &ammoParts; break;
	case ItemCategory::WeaponPart_FireType:  target = &fireTypeParts; break;
	case ItemCategory::Consumable:           target = &consumables; break;
	}

	if (!target) return false;

	// アイテムを検索して使用
	for (auto it = target->begin(); it != target->end(); ++it) {
		if (it->GetID() == itemId) {
			int newCount = it->GetCount() - useCount;
			if (newCount <= 0) {
				// 使い切り
				target->erase(it);
			}
			else {
				// 数を更新
				it->SetCount(newCount);
			}
			return true;
		}
	}

	return false;  // 見当たらない
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
		if (it->GetID() == itemId) {
			target->erase(it);
			return true;
		}
	}

	// アイテムが見つからなかった場合
	return false;
}


bool Inventory::Has(ItemCategory category, int itemId) const {
	// 対象の格納先ベクタを選ぶ（プロジェクトのメンバ名に合わせて）
	const std::vector<Item>* target = nullptr;
	switch (category) {
	case ItemCategory::WeaponPart_Ammo:     target = &ammoParts;      break;
	case ItemCategory::WeaponPart_FireType:   target = &fireTypeParts;  break; // ← "Wepon" つづり注意
	case ItemCategory::Consumable:          target = &consumables;    break;
	default: return false;
	}
	if (!target) return false;

	// 同じ ID が見つかったら所持しているとみなす（Count>0 なら）
	for (const auto& it : *target) {
		if (it.GetID() == itemId && it.GetCount() > 0) return true;
	}
	return false;
}

int Inventory::Count(ItemCategory category, int itemId) const {
	const std::vector<Item>* target = nullptr;
	switch (category) {
	case ItemCategory::WeaponPart_Ammo:     target = &ammoParts;      break;
	case ItemCategory::WeaponPart_FireType:   target = &fireTypeParts;  break;
	case ItemCategory::Consumable:          target = &consumables;    break;
	default: return 0;
	}
	if (!target) return 0;

	int total = 0;
	for (const auto& it : *target) {
		if (it.GetID() == itemId) total += it.GetCount();
	}
	return total;
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

