#pragma once
#include <string>
#include "main.h"
#include "renderer.h"


struct MapConfig {
	int mapID;
	char collisionModelPath[256];       // 当たり判定マップのモデルパス
	char decorationModelPath[256];		// 飾りマップのモデルパス
	char itemConfigPath[256];     // Item JSON配置ファイルパス
	char enemyConfigPath[256];    // Enemy JSON配置ファイルパス
	XMFLOAT3 playerSpawnPos;      // Playerの初期位置

	// 環境設定
	XMFLOAT3 lightDirection;
	XMFLOAT4 ambientColor;
	char backgroundMusic[128];
};


// マップ管理関数
HRESULT InitMapManager(void);
void UninitMapManager(void);
HRESULT LoadMap(int mapID);
void UnloadCurrentMap(void);
int GetCurrentMapID(void);
MapConfig* GetCurrentMapConfig(void);

// ロード関数
void LoadMapItems(const char* configPath);
void LoadMapEnemies(const char* configPath);
void SetPlayerSpawnPosition(const XMFLOAT3& pos);

// Player位置リセット
void ResetPlayerPosition(void);