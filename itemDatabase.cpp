#include "itemDatabase.h"


// �A�C�e���f�[�^�x�[�X�̃R���X�g���N�^
ItemDatabase::ItemDatabase() {
	texturePaths[ITEM_APPLE] = "data/2Dpicture/item/apple.png";
	texturePaths[ITEM_SAN] = "data/2Dpicture/item/heal.png";
	texturePaths[ITEM_BULLET] = "data/2Dpicture/item/bullet.png";

}


// �A�C�e��ID�ɑΉ�����e�N�X�`���p�X���擾����֐�
const std::string& ItemDatabase::GetTexturePath(int itemID) const {
	auto it = texturePaths.find(itemID);
	if (it != texturePaths.end()) {
		return it->second;
	}

	static std::string missing = "textures/missing.png";
	return missing;
}


//�g�p��
//ItemDatabase db;
//int itemId = ITEM_APPLE;
//std::string path = db.GetTexturePath(itemId);
//
//// ��FDirectX�ŃX�v���C�g�`��Ɏg��
//DrawTexture(path, x, y);

