#pragma once
#include <unordered_map>
#include <string>
#include "item.h"


class ItemDatabase {
private:
	std::unordered_map<int, std::string> texturePaths;

public:
	ItemDatabase();
	const std::string& GetTexturePath(int itemID) const;
};

