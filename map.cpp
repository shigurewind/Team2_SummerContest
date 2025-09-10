#include "map.h"
#include "item.h"
#include "enemy.h"
#include "player.h"
#include "FBXmodel.h"
#include "light.h"
#include "sound.h"
#include <fstream>


static int g_CurrentMapID = -1;
static MapConfig* g_CurrentMapConfig = nullptr;

// マップ設定データ
static MapConfig g_MapConfigs[] = {
	// Tutorial Map (MODE_TUTORIAL)
	{
		0,                                          // mapID
		"data/MODEL/tutorial_map.fbx",             // modelPath
		"data/CONFIG/tutorial_items.json",         // itemConfigPath
		"data/CONFIG/tutorial_enemies.json",       // enemyConfigPath
		{0.0f, 0.0f, 0.0f},                        // playerSpawnPos
		{0.5f, -1.0f, 0.5f},                       // lightDirection
		{0.3f, 0.3f, 0.3f, 1.0f},                  // ambientColor
		"data/SOUND/tutorial_bgm.wav"              // backgroundMusic
	},
	// stage 1 (MODE_GAME)
	{
		1,                                          // mapID
		"data/MODEL/tutorial_map.fbx",                // modelPath
		"data/CONFIG/map1_items.json",             // itemConfigPath
		"data/CONFIG/map1_enemies.json",           // enemyConfigPath
		{20.0f, 0.0f, 30.0f},                      // playerSpawnPos
		{0.3f, -1.0f, 0.7f},                       // lightDirection
		{0.2f, 0.2f, 0.3f, 1.0f},                  // ambientColor		
		"data/SOUND/game_bgm.wav"                  // backgroundMusic
	}
};

// Mapコンフィグ数
static const int g_MapConfigCount = sizeof(g_MapConfigs) / sizeof(MapConfig);



HRESULT InitMapManager(void) {
	g_CurrentMapID = -1;
	g_CurrentMapConfig = nullptr;
	return S_OK;
}

void UninitMapManager(void) {
	UnloadCurrentMap();
}

//今のマップID取得
int GetCurrentMapID(void) {
	return g_CurrentMapID;
}

// 今のコンフィグ取得
MapConfig* GetCurrentMapConfig(void) {
	return g_CurrentMapConfig;
}


// マップをロード
HRESULT LoadMap(int mapID) {
	// 
	MapConfig* config = nullptr;
	for (int i = 0; i < g_MapConfigCount; i++) {
		if (g_MapConfigs[i].mapID == mapID) {
			config = &g_MapConfigs[i];
			break;
		}
	}

	if (config == nullptr) {
		// ID存在しない
		return E_FAIL;
	}

	// 今のマップをアンロード
	UnloadCurrentMap();

	// FBXモデル
	FBXMAPMODEL* fbxModel = GetFBXMapModel();
	if (fbxModel && fbxModel->model) {
		// もうFBXモデルがロードされている
		UninitFBXMapModel();
	}

	InitFBXMapModel(config->modelPath); // 対応のFBXモデルをロード

	// Itemsロード
	LoadMapItems(config->itemConfigPath);

	// エネミーロード
	LoadMapEnemies(config->enemyConfigPath);

	// 初期位置設置
	SetPlayerSpawnPosition(config->playerSpawnPos);

	// TODO: 環境とBGMを設定

	// 今のマップ
	g_CurrentMapID = mapID;
	g_CurrentMapConfig = config;

	return S_OK;
}

// 今のマップをアンロード
void UnloadCurrentMap(void) {
	if (g_CurrentMapID == -1) return;


	// FBXモデル解放
	UninitFBXMapModel();

	// アイテム解放
	ClearAllItems();

	// エネミー解放
	ClearAllEnemies();

	g_CurrentMapID = -1;
	g_CurrentMapConfig = nullptr;
}


//ロード関数
void LoadMapItems(const char* configPath)
{
	if (!configPath || strlen(configPath) == 0) {
		return;
	}

	// アイテムをロード
	LoadItemData(std::string(configPath));
}

void LoadMapEnemies(const char* configPath)
{
	if (!configPath || strlen(configPath) == 0) {
		return;
	}

	// エネミーをロード
	LoadEnemyData(std::string(configPath));
}

void SetPlayerSpawnPosition(const XMFLOAT3& pos)
{
	PLAYER* player = GetPlayer();
	if (player) {
		player->SetPosition(pos);
	}
}



