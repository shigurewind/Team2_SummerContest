#pragma once
#include <string>
#include "renderer.h"
#include "object.h"

enum class ItemCategory {
	WeaponPart_Ammo,		//�e�̎�ނ����߂�p�[�c
	WeaponPart_FireType,	//�ł��������߂�p�[�c
	Consumable,				//���Օi
	InstantEffect,		//�������ʂ̃A�C�e��
};

//struct Item {
//	int id;                 //�B�ꖳ���ID
//	std::string name;       //���O
//	int count;              //������(���̃A�C�e���̐��A�C���x���g���[�̒��̂ł͂Ȃ�)?
//	ItemCategory category;  //�A�C�e���̃J�e�S��
//
//
//	Item(int id = 0, const std::string& name = "unknown", int count = 1, ItemCategory category = ItemCategory::Consumable)
//		: id(id), name(name), count(count), category(category) {
//	}
//};

class Item : public Object
{
public:
	Item(int id = 0, const std::string& name = "unknown", int count = 1, ItemCategory category = ItemCategory::Consumable)
		: id(id), name(name), count(count), category(category) {
	}

	int GetID() const { return id; }
	const std::string& GetName() const { return name; }
	ItemCategory GetCategory() const { return category; }

	int GetCount() const { return count; }
	void SetCount(int c) { count = c; }

private:
	int id;
	std::string name;
	int count;
	ItemCategory category;
};

//typedef struct {
//	XMFLOAT3	pos;        // �ʒu
//	XMFLOAT3	scl;        // �X�P�[��
//	MATERIAL	material;   // �}�e���A���i�F�j
//
//	float		fWidth;			// ��
//	float		fHeight;		// ����
//
//
//	Item		item;
//	BOOL		use;        // �g�p�����ǂ���
//
//	float		timeOffset;
//	float		basePosY;
//} ITEM_OBJ;

class ITEM_OBJ : public Object
{
public:
	ITEM_OBJ();
	void Update();
	//void Draw();
	void SetItem(const Item& item);
	bool IsUsed() const { return use; }
	void SetUsed(bool b) { use = b; }
	Item& GetItem() { return item; }

	void SetScale(const XMFLOAT3& scale) { scl = scale; }
	XMFLOAT3 GetScale() const { return scl; }

	void SetMaterial(const MATERIAL& mat) { material = mat; }
	const MATERIAL& GetMaterial() const { return material; }

	void SetBasePosY(float y) { basePosY = y; }

private:
	Item item;
	XMFLOAT3 scl;
	MATERIAL material;
	float width, height;
	bool use;
	float basePosY;
	float timeOffset;
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

void InitItem();
void UninitItem();
void UpdateItem();
void DrawItem();

void InitItemTextures();

Item CreateItemFromID(int id);
int SetItem(XMFLOAT3 pos, int itemID);

void SaveItemData(const std::string& filename);
void LoadItemData(const std::string& filename);


ITEM_OBJ* GetItemOBJ();
