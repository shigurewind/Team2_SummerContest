#include "inventory.h"

Inventory::Inventory(int ammoCap, int fireTypeCap, int conCap)
	: ammoCapacity(ammoCap), fireTypeCapacity(fireTypeCap), consumableCapacity(conCap) {
}


//�V�����A�C�e����ǉ�����֐�
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

	// ����ID�̃A�C�e�������邩�m�F�i�X�^�b�N����j
	for (auto& i : *target) {
		if (i.id == item.id) {
			i.count += item.count;
			return true; // �����A�C�e���ɉ��Z���ďI��
		}
	}

	// �V�K�X���b�g�Ƃ��Ēǉ��i�e�ʃ`�F�b�N�j
	if ((int)target->size() < *capacity) {
		target->push_back(item);
		return true;
	}

	return false; // �e�ʃI�[�o�[
}


// �A�C�e�����폜����֐�
bool Inventory::RemoveItem(int itemId, ItemCategory category) {
	std::vector<Item>* target = nullptr;

	// �J�e�S���ɉ����ă^�[�Q�b�g��ݒ�
	switch (category) {
	case ItemCategory::WeaponPart_Ammo:      target = &ammoParts; break;
	case ItemCategory::WeaponPart_FireType:  target = &fireTypeParts; break;
	case ItemCategory::Consumable:           target = &consumables; break;
	}

	// �A�C�e�����������č폜
	for (auto it = target->begin(); it != target->end(); ++it) {
		if (it->id == itemId) {
			target->erase(it);
			return true;
		}
	}

	// �A�C�e����������Ȃ������ꍇ
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

