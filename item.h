#pragma once
#include <string>
#include "renderer.h"

enum class ItemCategory {
	WeaponPart_Ammo,		//�e�̎�ނ����߂�p�[�c
	WeaponPart_FireType,	//�ł��������߂�p�[�c
	Consumable,				//���Օi
	InstantEffect,		//�������ʂ̃A�C�e��
};

struct Item {
	int id;                 //�B�ꖳ���ID
	std::string name;       //���O
	int count;              //������(���̃A�C�e���̐��A�C���x���g���[�̒��̂ł͂Ȃ�)?
	ItemCategory category;  //�A�C�e���̃J�e�S��


	Item(int id = 0, const std::string& name = "unknown", int count = 1, ItemCategory category = ItemCategory::Consumable)
		: id(id), name(name), count(count), category(category) {
	}
};



// �A�C�e��ID�̒�`
enum ItemID
{
	//�e�̎�ނ����߂�p�[�c
	PART_FIRE,

	//�ł��������߂�p�[�c
	PART_SHUTGUN,

	//���Օi
	ITEM_APPLE,

	//�񕜕i
	ITEM_SAN,
	ITEM_BULLET,

	ITEM_ID_COUNT//ItemID�̐�
};


int SetItem(XMFLOAT3 pos, int itemID);

HRESULT InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();

Item CreateItemFromID(int id);



