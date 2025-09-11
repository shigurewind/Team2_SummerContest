#pragma once
#include <string>
#include "main.h"
#include "renderer.h"

#define MAX_TRANSITION_ZONES_PER_MAP 2 // マップあたりの最大遷移ゾーン数

// 遷移ゾーン構造体
struct SceneTransitionZone {
	XMFLOAT3 center;        // 位置
	XMFLOAT3 size;          // サイズ
	int targetMapID;        // 目標マップID
	bool enabled;           // 起用状態
	XMFLOAT4 debugColor;    // debug表示用の色
	char name[64];          // 名前

	XMFLOAT3 GetMin() const {
		return {
			center.x - size.x / 2.0f,
			center.y - size.y / 2.0f,
			center.z - size.z / 2.0f
		};
	}

	XMFLOAT3 GetMax() const {
		return {
			center.x + size.x / 2.0f,
			center.y + size.y / 2.0f,
			center.z + size.z / 2.0f
		};
	}
};


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

	// 遷移ゾーン
	SceneTransitionZone transitionZones[MAX_TRANSITION_ZONES_PER_MAP];
	int transitionZoneCount;
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

// 遷移ゾーン管理関数
void CheckPlayerInTransitionZones();
SceneTransitionZone* GetCurrentMapTransitionZones();
int GetCurrentMapTransitionZoneCount();
void UpdateTransitionZone(int index, const SceneTransitionZone& zone);