#include "itemDatabase.h"


// アイテムデータベースのコンストラクタ
ItemDatabase::ItemDatabase() {
	texturePaths[ITEM_APPLE] = "data/2Dpicture/item/apple.png";
	texturePaths[ITEM_SAN] = "data/2Dpicture/item/heal.png";
	texturePaths[ITEM_BULLET] = "data/2Dpicture/item/bullet.png";

}


// アイテムIDに対応するテクスチャパスを取得する関数
const std::string& ItemDatabase::GetTexturePath(int itemID) const {
	auto it = texturePaths.find(itemID);
	if (it != texturePaths.end()) {
		return it->second;
	}

	static std::string missing = "textures/missing.png";
	return missing;
}


//使用例
//ItemDatabase db;
//int itemId = ITEM_APPLE;
//std::string path = db.GetTexturePath(itemId);
//
//// 例：DirectXでスプライト描画に使う
//DrawTexture(path, x, y);

