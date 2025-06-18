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

	bool isInstantEffect;

	Item(int id, const std::string& name, int count, ItemCategory category, bool isInstant = false)
		: id(id), name(name), count(count), category(category) , isInstantEffect(isInstant) {
	}
};



// �A�C�e��ID�̒�`
enum ItemID
{
	//�e�̎�ނ����߂�p�[�c

	//�ł��������߂�p�[�c

	//���Օi
	ITEM_APPLE,

	//�񕜕i
	ITEM_SAN,
	ITEM_BULLET,
};


int SetItem(XMFLOAT3 pos, int itemID);

HRESULT InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();



