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

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER		(1)					// プレイヤーの数

#define	PLAYER_SIZE		(5.0f)				// 当たり判定の大きさ





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


	int shadowIdx;
	bool alive;

	//武器関連
	WeaponType currentWeapon;
	BulletType currentBullet;
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


PLAYER* GetPlayer(void);
bool CheckPlayerGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY);
