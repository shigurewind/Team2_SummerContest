//=============================================================================
//
// モデル処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "player.h"
#include "shadow.h"
#include "debugproc.h"
#include "meshfield.h"
#include "FBXmodel.h"
#include "Octree.h"
#include "collision.h"
#include "overlay2D.h"
#include "enemy.h"

//*****************************************************************************
// マクロ定義	
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/player.obj"			// 読み込むモデル名


#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(2)								// プレイヤーのパーツの数



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
PLAYER	g_Player;						// プレイヤー

//static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static float		roty = 0.0f;

static LIGHT		g_Light;

//重力
//static float gravity = 0.5f;
//近接攻撃クールダウン
static float meleeCooldown = 0.0f;
//チュートリアル判定用
static bool tutorialTriggered = false;


//weponとbullet弾の状態
static WeaponType currentWeapon = WEAPON_REVOLVER;
static BulletType currentBullet = BULLET_NORMAL;








int Min(int a, int b) {
	return (a < b) ? a : b;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{

	g_Player.Init();





	return S_OK;
}

void PLAYER::Init()
{

	// 基本初期化
	pos = { 0, PLAYER_OFFSET_Y + 50.0f, 0 };
	rot = { 0, 0, 0 };
	scl = { 1, 1, 1 };
	velocity = { 0, 0, 0 };
	speed = 0;
	size = PLAYER_SIZE;

	EnableGravity(true);
	SetMaxFallSpeed(6.0f);
	jumpPower = 8.0f;

	ammoNormal = 0;
	maxAmmoNormal = 30;
	ammoFire = 0;
	maxAmmoFire = 20;

	HP = HP_MAX = 5;
	alive = true;

	meleeCDTime = 0.8f;

	currentWeapon = WEAPON_REVOLVER;
	currentBullet = BULLET_NORMAL;

	load = TRUE;
	LoadModel(MODEL_PLAYER, &model);

	// 影
	XMFLOAT3 shadowPos = pos;
	shadowPos.y -= (PLAYER_OFFSET_Y - 0.1f);
	shadowIdx = CreateShadow(shadowPos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);

}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	// モデルの解放処理
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}




}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{


	if (meleeCooldown > 0.0f) {
		meleeCooldown -= 1.0f / 60.0f;
	}

	if (g_Player.alive)
	{

		g_Player.HandleInput();
		g_Player.HandleShooting();
		g_Player.HandleReload();
		g_Player.HandleJump();
		g_Player.HandleGroundCheck();
		g_Player.EventCheck(); // イベントチェック






		//HP減るtest
		if (GetKeyboardTrigger(DIK_H))
		{
			g_Player.HP = g_Player.HP - 1;
		}

	}



#ifdef _DEBUG

#endif









	// 影もプレイヤーの位置に合わせる
	XMFLOAT3 pos = g_Player.GetPosition();
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);





	// ポイントライトのテスト
	{
		LIGHT* light = GetLightData(1);
		XMFLOAT3 pos = g_Player.GetPosition();
		pos.y += 20.0f;

		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(1, light);
	}






#ifdef _DEBUG
	// デバッグ表示
	//PrintDebugProc("Player X:%f Y:%f Z:%f \n\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);

	PrintDebugProc("Rキーでリロード\n"
		"1キーで武器切り替え\n"
		"2キーで弾切り替え");
#endif

}


void PLAYER::Update() {
	HandleInput();          // W/A/S/D移動 & 方向制御
	HandleJump();           // スペースキー処理
	//UpdatePhysics();        // 重力 & 速度反映
	HandleGroundCheck();    // 地面接地判定
	HandleShooting();       // 弾発射
	HandleReload();         // Rでリロード
}

//ジャンプ
void PLAYER::HandleJump() {
	if (GetKeyboardTrigger(DIK_SPACE) && isGround) {
		velocity.y = jumpPower;
		isGround = false;
	}
}

//移動処理
void PLAYER::HandleInput()
{
	//移動処理TODO：変更必要
	CAMERA* cam = GetCamera();

	//g_Player.speed *= 0.7f;

	// 移動処理
	XMFLOAT3 move = {};
	bool isMoving = false;

	if (GetKeyboardPress(DIK_W)) {
		move.x += sinf(cam->rot.y);
		move.z += cosf(cam->rot.y);
		isMoving = true;
	}
	if (GetKeyboardPress(DIK_S)) {
		move.x -= sinf(cam->rot.y);
		move.z -= cosf(cam->rot.y);
		isMoving = true;
	}
	if (GetKeyboardPress(DIK_A)) {
		move.x -= cosf(cam->rot.y);
		move.z += sinf(cam->rot.y);
		isMoving = true;
	}
	if (GetKeyboardPress(DIK_D)) {
		move.x += cosf(cam->rot.y);
		move.z -= sinf(cam->rot.y);
		isMoving = true;
	}

	XMFLOAT3 newPos = pos;
	if (isMoving) {
		XMVECTOR moveVec = XMVector3Normalize(XMLoadFloat3(&move));
		XMFLOAT3 testPos = pos;
		testPos.x += XMVectorGetX(moveVec) * speed;
		testPos.z += XMVectorGetZ(moveVec) * speed;

		XMFLOAT3 wallBoxMin = testPos;
		XMFLOAT3 wallBoxMax = testPos;
		float halfSize = size;

		wallBoxMin.x -= halfSize;
		wallBoxMin.y -= 0.1f;
		wallBoxMin.z -= halfSize;

		wallBoxMax.x += halfSize;
		wallBoxMax.y += 0.1f;
		wallBoxMax.z += halfSize;

		if (!AABBHitOctree(GetWallTree(), GetWallTriangles(), wallBoxMin, wallBoxMax, 0, 5, 5)) {
			newPos.x = testPos.x;
			newPos.z = testPos.z;
		}
	}



	//近接攻撃
	if (IsMouseRightTriggered() && meleeCooldown <= 0.0f)
	{
		meleeCooldown = meleeCDTime;
		PlayMeleeAnimation();
		//enemy 

		auto& enemies = GetEnemies();
		for (auto enemy : enemies) {
			if (!enemy->IsUsed()) continue;

			XMFLOAT3 ePos = enemy->GetPosition();
			float dx = g_Player.pos.x - ePos.x;
			float dz = g_Player.pos.z - ePos.z;
			float distance = sqrtf(dx * dx + dz * dz);

			if (distance > 100.0f) continue;

			enemy->SetUsed(false);
		}
	}


	//武器切り替え
	//キーボードの1　武器の切り替え
	if (GetKeyboardTrigger(DIK_1))
	{
		currentWeapon = (currentWeapon == WEAPON_REVOLVER) ? WEAPON_SHOTGUN : WEAPON_REVOLVER;
	}
	//キーボードの2　弾の切り替え
	if (GetKeyboardTrigger(DIK_2))
	{
		currentBullet = (currentBullet == BULLET_NORMAL) ? BULLET_FIRE : BULLET_NORMAL;
	}


}

//接地判定
void PLAYER::HandleGroundCheck()
{


	const float groundThreshold = 0.2f;
	float groundY;
	if (CheckPlayerGroundSimple(newPos, PLAYER_OFFSET_Y, groundY) && GetVelocity().y <= 0.0f)
	{
		float targetY = groundY;
		float distanceToGround = newPos.y - targetY;
		if (distanceToGround <= groundThreshold)
		{
			newPos.y = targetY;
			SetVelocity(XMFLOAT3(GetVelocity().x, 0.0f, GetVelocity().z));
			isGround = TRUE;
		}
		else
		{
			isGround = FALSE;
		}
	}
	else
	{
		isGround = FALSE;
	}


	pos = newPos;//TODO:Objectの移動処理衝突かも

}

void PLAYER::EventCheck()
{
	//特定の地域入るとゲームを停止
	if (!tutorialTriggered &&
		pos.x > 50.0f && pos.x < 100.0f &&
		pos.z > 50.0f && pos.z < 100.0f)
	{
		SetTutorialShowing(true);
		tutorialTriggered = true;
	}

}


void PLAYER::HandleShooting()
{
	// 弾発射処理
	int* currentAmmo = (currentBullet == BULLET_NORMAL) ? &ammoNormal : &ammoFire;
	if (IsMouseLeftTriggered() && currentAmmo > 0)
	{
		XMFLOAT3 pos = GetGunMuzzlePosition();
		XMFLOAT3 rot = GetGunMuzzleRotation();
		if (currentWeapon == WEAPON_REVOLVER)
		{
			SetRevolverBullet(currentBullet, pos, rot);
		}
		else {
			SetShotgunBullet(currentBullet, pos, rot);
		}
		(currentAmmo)--;
	}
}


void PLAYER::HandleReload()
{
	// Rキーでリロード処理
	if (GetKeyboardTrigger(DIK_R))
	{
		Weapon* weapon = (currentWeapon == WEAPON_REVOLVER) ? GetRevolver() : GetShotgun();
		int clipSize = weapon->clipSize;

		int* ammo = (currentBullet == BULLET_NORMAL) ? &ammoNormal : &ammoFire;
		int* maxAmmo = (currentBullet == BULLET_NORMAL) ? &maxAmmoNormal : &maxAmmoFire;

		if (*ammo < clipSize && *maxAmmo > 0)
		{
			int need = clipSize - *ammo;
			int reload = Min(need, *maxAmmo);
			*ammo += reload;
			*maxAmmo -= reload;
		}
	}
}



//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	XMMATRIX mtxFootOffset = XMMatrixTranslation(0.0f, -PLAYER_OFFSET_Y, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxFootOffset);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// クォータニオンを反映
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.GetPosition().x, g_Player.GetPosition().y, g_Player.GetPosition().z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMFLOAT4X4 temp;
	DirectX::XMStoreFloat4x4(&temp, mtxWorld);
	g_Player.mtxWorld = temp;


	// 縁取りの設定
	SetFuchi(1);

	// モデル描画
	DrawModel(&g_Player.model);



	SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER GetPlayer(void)
{
	return g_Player;
}

WeaponType GetCurrentWeaponType(void)
{
	return currentWeapon;
}

BulletType GetCurrentBulletType(void)
{
	return currentBullet;
}
bool CheckPlayerGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY)
{
	const auto& tris = GetFloorTriangles();

	XMFLOAT3 rayStart = pos;
	rayStart.y += 50.0f;
	XMFLOAT3 rayEnd = pos;
	rayEnd.y -= 100.0f;

	XMFLOAT3 hit, normal;
	for (const auto& tri : tris)
	{
		if (RayCast(tri.v0, tri.v1, tri.v2, rayStart, rayEnd, &hit, &normal))
		{
			groundY = hit.y;
			return true;
		}
	}
	return false;
}