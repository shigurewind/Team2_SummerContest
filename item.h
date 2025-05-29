#pragma once
#include <string>

enum class ItemCategory {
	WeaponPart_Ammo,		//�e�̎�ނ����߂�p�[�c
	WeaponPart_FireType,	//�ł��������߂�p�[�c
	Consumable,				//���Օi
};

struct Item {
	int id;                 //�B�ꖳ���ID
	std::string name;       //���O
	int count;              //������(���̃A�C�e���̐��A�C���x���g���[�̒��̂ł͂Ȃ�)?
	ItemCategory category;  //�A�C�e���̃J�e�S��

	Item(int id, const std::string& name, int count, ItemCategory category)
		: id(id), name(name), count(count), category(category) {
	}
};

// �A�C�e��ID�̒�`
enum ItemID
{
	//�e�̎�ނ����߂�p�[�c

	//�ł��������߂�p�[�c

	//���Օi
	ITEM_APPLE,
};