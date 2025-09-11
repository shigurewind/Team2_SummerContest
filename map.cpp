#include "map.h"
#include "item.h"
#include "enemy.h"
#include "player.h"
#include "FBXmodel.h"
#include "light.h"
#include "sound.h"
#include <fstream>
#include "fade.h"


static int g_CurrentMapID = -1;
static MapConfig* g_CurrentMapConfig = nullptr;

// マップ設定データ
static MapConfig g_MapConfigs[] = {
	// stage 1 (MODE_TUTORIAL)
	{
		0,                                          // mapID
		"data/MODEL/stage1_collision.fbx",             // collision modelPath
		"data/MODEL/stage1_Nocollision.fbx",           // Nocollision modelPath
		"data/CONFIG/tutorial_items.json",         // itemConfigPath
		"data/CONFIG/tutorial_enemies.json",       // enemyConfigPath
		{0.0f, 0.0f, 0.0f},                        // playerSpawnPos
		{0.5f, -1.0f, 0.5f},                       // lightDirection
		{0.3f, 0.3f, 0.3f, 1.0f},                  // ambientColor
		"",              // backgroundMusic
		{
			  { // Zone 0: Stage1 -> Stage2
				  {-357.0f, -10.0f, 125.0f},               // pos
				  {20.0f, 30.0f, 20.0f},				// size
				  1,                                   // targetMapID (stage2)
				  true,                                // enabled
				  {0.0f, 1.0f, 0.0f, 0.5f},           // debugColor (緑)
				  "To Stage2"                          // name
			  }
		  },
		  1
	},
	// stage 2 (MODE_GAME)
	{
		1,                                         // mapID
		"data/MODEL/stage2_collision.fbx",         // collision modelPath
		"",											// Nocollision modelPath
		"data/CONFIG/map1_items.json",             // itemConfigPath
		"data/CONFIG/map1_enemies.json",           // enemyConfigPath
		{0.0f, 0.0f, 0.0f},                      // playerSpawnPos
		{0.3f, -1.0f, 0.7f},                       // lightDirection
		{0.2f, 0.2f, 0.3f, 1.0f},                  // ambientColor		
		"",                  // backgroundMusic
		{
			  { // Zone 0: Stage2 -> Stage1
				  {0.0f, 10.0f, 0.0f},               // center
				  {20.0f, 20.0f, 20.0f},             // size
				  0,                                  // targetMapID (stage1)
				  true,                               // enabled
				  {1.0f, 1.0f, 0.0f, 0.5f},          // debugColor (黄色)
				  "To Stage1"                         // name
			  },
			  { // Zone 1: Stage2 -> Stage3
				  {200.0f, 10.0f, 200.0f},           // center
				  {20.0f, 20.0f, 20.0f},             // size
				  3,                                  // targetMapID (stage3)
				  true,                               // enabled
				  {0.0f, 0.0f, 1.0f, 0.5f},          // debugColor (青色)
				  "To Stage3"                         // name
			  }
		  },
		  2
	},
	// stage 3 ()
  {
	  3,                                          // mapID
	  "data/MODEL/stage3_collision.fbx",         // collisionModelPath
	  "data/MODEL/stage3_Nocollision.fbx",       // decorationModelPath
	  "data/CONFIG/map3_items.json",             // itemConfigPath
	  "data/CONFIG/map3_enemies.json",           // enemyConfigPath
	  {0.0f, 0.0f, 0.0f},                        // playerSpawnPos
	  {0.3f, -1.0f, 0.7f},                       // lightDirection
	  {0.2f, 0.2f, 0.3f, 1.0f},                  // ambientColor
	  "",                                          // backgroundMusic
	  {
			  { // Zone 0: Stage3 -> Stage2
				  {200.0f, 10.0f, 200.0f},           // center
				  {20.0f, 20.0f, 20.0f},             // size
				  1,                                  // targetMapID (stage2)
				  true,                               // enabled
				  {0.5f, 0.0f, 0.5f, 0.5f},          // debugColor (紫色)
				  "To Stage2"                         // name
			  }
		  },
		  1
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

	InitFBXMapModel(config->collisionModelPath, config->decorationModelPath); // 対応のFBXモデルをロード

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


// Player位置リセット
void ResetPlayerPosition(void)
{
	// 今のマップコンフィグを取得
	MapConfig* currentConfig = GetCurrentMapConfig();
	if (currentConfig == nullptr) {
		return;
	}

	// Player位置をリセット
	PLAYER* player = GetPlayer();
	if (player) {
		player->SetPosition(currentConfig->playerSpawnPos);
	}
}


// プレイヤーがトランジションゾーンにいるかチェック
void CheckPlayerInTransitionZones()
{
	if (GetFade() != FADE_NONE) return; // 切り替え中は無視
	if (g_CurrentMapConfig == nullptr) return;

	PLAYER* player = GetPlayer();
	XMFLOAT3 playerPos = player->GetPosition();

	for (int i = 0; i < g_CurrentMapConfig->transitionZoneCount; i++)
	{
		SceneTransitionZone* zone = &g_CurrentMapConfig->transitionZones[i];
		if (!zone->enabled) continue;

		XMFLOAT3 min = zone->GetMin();
		XMFLOAT3 max = zone->GetMax();

		// プレイヤー位置チェック
		if (playerPos.x >= min.x && playerPos.x <= max.x &&
			playerPos.y >= min.y && playerPos.y <= max.y &&
			playerPos.z >= min.z && playerPos.z <= max.z)
		{
			// ターゲットマップIDに基づいてモードを決定
			int targetMode;
			switch (zone->targetMapID) {
			case 0: targetMode = MODE_TUTORIAL; break;     // Stage1
			case 1: targetMode = MODE_STAGE2; break;   // Stage2
			case 3: targetMode = MODE_STAGE3; break;   // Stage3
			default: continue;
			}

			SetFade(FADE_OUT, targetMode);
			break; 
		}
	}
}


// 現在のマップのトランジションゾーン配列を取得
SceneTransitionZone* GetCurrentMapTransitionZones()
{
	if (g_CurrentMapConfig == nullptr) return nullptr;
	return g_CurrentMapConfig->transitionZones;
}

// 現在のマップのトランジションゾーン数を取得
int GetCurrentMapTransitionZoneCount()
{
	if (g_CurrentMapConfig == nullptr) return 0;
	return g_CurrentMapConfig->transitionZoneCount;
}

// トランジションゾーンを更新
void UpdateTransitionZone(int index, const SceneTransitionZone& zone)
{
	if (g_CurrentMapConfig == nullptr) return;
	if (index < 0 || index >= g_CurrentMapConfig->transitionZoneCount) return;

	g_CurrentMapConfig->transitionZones[index] = zone;
}



