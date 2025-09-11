//=============================================================================
//
// モデル処理 [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"
#include "bullet.h"
#include "object.h"
#include "inventory.h"
#include <fstream>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER		(1)					// プレイヤーの数

#define	PLAYER_SIZE		(10.0f)				// 当たり判定の大きさ





class PLAYER : public Object
{
public:
	void Init();
	void OnUpdate();
	//void Draw();

	void HandleInput();//移動と他のInput処理
	void ApplyCollision();//マップ当たり判定処理
	void HandleGroundCheck();//地面チェック

	void HandleShooting();
	void HandleReload();
	void HandleJump();

	void EventCheck();

	void CheckPlayerDeath();

	void ApplySlow(float factor, int durationFrames);

	//壁のノーマル取得（Slide機能ために）
	XMFLOAT3 GetWallCollisionNormal(XMFLOAT3 currentPos, XMFLOAT3 moveVector, float halfSize);



	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	BOOL				load;
	DX11_MODEL			model;				// モデル情報

	// クォータニオン
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// 自分が立っている所


	float HP, HP_MAX;
	int ammoNormal, maxAmmoNormal;
	int ammoFire, maxAmmoFire;

	//移動関連
	float			size;				// 当たり判定の大きさ
	float			speed;				// 移動スピード
	float			jumpPower;	//jumpのパワー

	//攻撃
	float meleeCDTime; // 近接攻撃のクールダウン時間

	//蜘蛛のエフェクトのせいでプレイヤーの速度遅くなる
	int slowTimer;      // エフェクトのカウンターダウン
	float currentSpeed; // 現在の速度


	int shadowIdx;
	bool alive;

	//武器関連
	WeaponType currentWeapon;
	BulletType currentBullet;

	/// 現在のインベントリ内容で、指定の武器が解放済みか？
	bool IsWeaponUnlocked(int weapon) const;

	/// 現在のインベントリ内容で、指定の弾種が解放済みか？
	bool IsBulletUnlocked(int bullet) const;

	/// 現在の「武器×弾の組合せ」が使えるか？（射撃の直前ガードで使用）
	bool IsCurrentLoadoutUsable() const;


	//インベントリ
	Inventory inventory;
	int currentConsumableIndex;//現在選択している消費アイテムのインデックス
};



struct PlayerSaveData {
	int weapon;      
	int bullet;       
	int ammoNormal;
	int ammoFire;
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

WeaponType GetCurrentWeaponType(void);
BulletType GetCurrentBulletType(void);

Inventory* GetPlayerInventory(void);


PLAYER* GetPlayer(void);
bool CheckPlayerGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY);

void SavePlayerToFile();
void LoadPlayerFromFile();

void SetLoadOnInit(bool enable);