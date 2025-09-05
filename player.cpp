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
#include "inputManager.h"
#include "item.h"

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

const char* SAVE_FILE_PATH = "player_save.dat";

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
//static WeaponType currentWeapon = WEAPON_REVOLVER;
//static BulletType currentBullet = BULLET_NORMAL;

// どの武器/弾を使うために、どの "PART_***" が必要かの表。
// ここを編集すれば「最初から使える/使えない」を柔軟に変更可。
static int RequiredPartForWeapon(int weapon) {
	switch (weapon) {
	case WEAPON_REVOLVER:        return PART_REVOLVER;      // リボルバーは PART_REVOLVER を拾ったら解禁
	case WEAPON_SHOTGUN:         return PART_SHUTGUN;       // ショットガンは PART_SHUTGUN
	case WEAPON_ROCKET_LAUNCHER: return PART_ROCKET;        // ロケランは PART_ROCKET
	default: return -1; // 未知 → 解禁不可（false）
	}
}
static int RequiredPartForBullet(int bullet) {
	switch (bullet) {
	case BULLET_NORMAL: return PART_NORMAL_AMMO;            // ノーマル弾は PART_NORMAL_AMMO
	case BULLET_FIRE:   return PART_FIRE;                   // ファイア弾は PART_FIRE
	default: return -1;
	}
}

static bool IsFireTypeUnlocked(const class Inventory* inv, WeaponType w);
static bool IsAmmoUnlocked(const class Inventory* inv, BulletType b);


// Init時にロードするかどうかの内部フラグ（既定 false）
namespace {
	bool g_LoadOnInit = false;
}

// 外部から切り替えるための関数
void SetLoadOnInit(bool enable) {
	g_LoadOnInit = enable;
}





int Min(int a, int b) {
	return (a < b) ? a : b;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{

	g_Player.Init();



	if (g_LoadOnInit) {
		LoadPlayerFromFile();
	}
	return S_OK;
}

void PLAYER::Init()
{

	// 基本初期化
	pos = { 0, PLAYER_OFFSET_Y + 50.0f, 0 };
	rot = { 0, 0, 0 };
	scl = { 1, 1, 1 };
	velocity = { 0, 0, 0 };
	speed = 2.0f;
	size = PLAYER_SIZE;

	EnableGravity(true);
	SetMaxFallSpeed(6.0f);
	jumpPower = 8.0f;

	ammoNormal = 30;
	maxAmmoNormal = 36;
	ammoFire = 20;
	maxAmmoFire = 20;

	HP = HP_MAX = 100;
	alive = true;

	meleeCDTime = 0.8f;


	//試しにリボルバーとノーマル弾だけ持っている状態

	Inventory* inv = GetPlayerInventory();
	if (inv)
	{
		// まだ未所持なら、該当パーツをインベントリへ追加する（重複追加を避ける）
		// ・武器種（FireType）＝リボルバー
		if (!inv->Has(ItemCategory::WeaponPart_FireType, PART_REVOLVER))  // 所持確認
		{
			inv->AddItem(CreateItemFromID(PART_REVOLVER));                 // 追加
		}

		// ・弾種（Ammo）＝ノーマル弾
		if (!inv->Has(ItemCategory::WeaponPart_Ammo, PART_NORMAL_AMMO))    // 所持確認
		{
			inv->AddItem(CreateItemFromID(PART_NORMAL_AMMO));              // 追加
		}
	}



	//初期選択武器、弾
	currentWeapon = WEAPON_REVOLVER;
	currentBullet = BULLET_NORMAL;

	currentConsumableIndex = 0;

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


		g_Player.OnUpdate(); // プレイヤーの更新処理

		//スポットライトの更新
		UpdateSpotlight();




		//HP減るtest
		if (GetKeyboardTrigger(DIK_H))
		{
			g_Player.HP = g_Player.HP - 50;
		}


		if (g_Player.HP <= 0 && g_Player.alive)
		{
			g_Player.alive = false;
			SavePlayerToFile();
			 /*GameOver Continue*/
		}

	}

	if (!g_Player.alive && GetKeyboardTrigger(DIK_C))
	{
		g_Player.HP = g_Player.HP_MAX;
		g_Player.alive = true;
		g_Player.SetPosition(XMFLOAT3(0.0f, PLAYER_OFFSET_Y + 50.0f, 0.0f));
		LoadPlayerFromFile();
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

	PrintDebugProc(
		"1キーで武器切り替え\n"
		"2キーで弾切り替え");
#endif

}


void PLAYER::OnUpdate() {
	HandleInput();          // W/A/S/D移動 & 方向制御
	HandleJump();           // スペースキー処理

	ApplyCollision();      // 衝突判定と適用　TODO:時間かかりすぎ
	Object::Update();
	HandleGroundCheck();    // 地面接地判定 TODO:時間かかりすぎ

	HandleShooting();       // 弾発射
	//HandleReload();         // Rでリロード

	EventCheck();          // イベントチェック
}

//ジャンプ
void PLAYER::HandleJump() {
	if (g_pInputManager->IsActionTriggered(ACTION_JUMP) && isGround) {
		velocity.y = jumpPower;
		isGround = false;
	}
}

//移動処理
void PLAYER::HandleInput()
{

	CAMERA* cam = GetCamera();



	// 移動処理
	XMFLOAT3 move = {};
	XMFLOAT2 inputVector = {};

	if (g_pInputManager->IsActionPressed(ACTION_MOVE_FORWARD)) {
		inputVector.y += 1.0f;
	}
	if (g_pInputManager->IsActionPressed(ACTION_MOVE_BACKWARD)) {
		inputVector.y -= 1.0f;
	}
	if (g_pInputManager->IsActionPressed(ACTION_MOVE_LEFT)) {
		inputVector.x -= 1.0f;
	}
	if (g_pInputManager->IsActionPressed(ACTION_MOVE_RIGHT)) {
		inputVector.x += 1.0f;
	}

	float stickX = g_pInputManager->GetLeftStickXValue();
	float stickY = g_pInputManager->GetLeftStickYValue();

	if (fabs(stickX) > 0.1f || fabs(stickY) > 0.1f) {
		inputVector.x = stickX;
		inputVector.y = -stickY;
	}

	//カメラの向きに合わせて移動
	if (inputVector.x != 0.0f || inputVector.y != 0.0f) {
		move.x += sinf(cam->rot.y) * inputVector.y + cosf(cam->rot.y) * inputVector.x;
		move.z += cosf(cam->rot.y) * inputVector.y - sinf(cam->rot.y) * inputVector.x;
	}

	velocity.x = move.x * speed;
	velocity.z = move.z * speed;



	//近接攻撃
	if (g_pInputManager->IsActionTriggered(ACTION_MELEE) && meleeCooldown <= 0.0f)
	{
		meleeCooldown = meleeCDTime;
		PlayMeleeAnimation();
		//enemy 

		auto& enemies = GetEnemies();
		XMFLOAT3 p = GetPosition();
		for (auto enemy : enemies) {
			if (!enemy->IsUsed()) continue;

			XMFLOAT3 ePos = enemy->GetPosition();
			float dx = p.x - ePos.x;
			float dz = p.z - ePos.z;
			float distance = sqrtf(dx * dx + dz * dz);

			if (distance > 100.0f) continue;

			enemy->SetUsed(false);
		}
	}

	//Item関連
	if (g_pInputManager->IsActionTriggered(ACTION_USE_ITEM)) {
		UseCurrentItem();  // 今のアイテムを使用
	}

	if (g_pInputManager->IsActionTriggered(ACTION_LAST_ITEM)) {
		SwitchToPreviousItem();  // 先のアイテム
	}

	if (g_pInputManager->IsActionTriggered(ACTION_NEXT_ITEM)) {
		SwitchToNextItem();  // 次のアイテム
	}


	//スポットライトの切り替え
	if (g_pInputManager->IsActionTriggered(ACTION_LIGHT_SWITCH))
	{
		SetSpotlightEnabled(!GetSpotlightEnabled());
	}


	// --- 武器の切り替え（UIの選択だけ。ロック判定はしない） ---
	if (GetKeyboardTrigger(DIK_1))
	{
		switch (currentWeapon)
		{
		case WEAPON_REVOLVER:         currentWeapon = WEAPON_SHOTGUN;         break;
		case WEAPON_SHOTGUN:          currentWeapon = WEAPON_ROCKET_LAUNCHER; break;
		case WEAPON_ROCKET_LAUNCHER:  currentWeapon = WEAPON_REVOLVER;        break;
		}
	}
	// --- 弾の切り替え（UIの選択だけ。ロック判定はしない） ---
	if (GetKeyboardTrigger(DIK_2))
	{
		currentBullet = (currentBullet == BULLET_NORMAL) ? BULLET_FIRE : BULLET_NORMAL;
	}



}


void PLAYER::ApplyCollision()
{
	//次の位置を予測
	XMFLOAT3 nextPos = pos;
	nextPos.x += velocity.x;
	nextPos.z += velocity.z;

	//BOXの計算
	float halfSize = size;
	XMFLOAT3 min = { nextPos.x - halfSize, pos.y - 0.1f, nextPos.z - halfSize };
	XMFLOAT3 max = { nextPos.x + halfSize, pos.y + 0.1f, nextPos.z + halfSize };

	if (CheckWallCollisionLOD(min, max, this))
	{
		// 壁に沿ってスライド
		//velocity-(velocity・normal)*normal

		XMFLOAT3 remainingVelocity = { velocity.x, 0.0f, velocity.z };
		const int maxIterations = 3; //回数制限
		const float minVelocityThreshold = 0.1f; //移動最小値

		for (int iteration = 0; iteration < maxIterations; iteration++)
		{
			//移動できるかどうか確認
			float velocityMagnitude = sqrtf(remainingVelocity.x * remainingVelocity.x +
				remainingVelocity.z * remainingVelocity.z);
			if (velocityMagnitude < minVelocityThreshold)
			{
				break; //小さい移動量なら終了
			}

			//壁法線を取得
			XMFLOAT3 wallNormal = GetWallCollisionNormal(pos, remainingVelocity, halfSize);

			if (wallNormal.x == 0.0f && wallNormal.z == 0.0f)
			{
				break; // 壁法線が取得できなかった場合終了
			}

			//スライドベクトルを計算
			XMVECTOR vel = XMLoadFloat3(&remainingVelocity);
			XMVECTOR normal = XMLoadFloat3(&wallNormal);

			float dotProduct = XMVectorGetX(XMVector3Dot(vel, normal));
			XMVECTOR slide = XMVectorSubtract(vel, XMVectorScale(normal, dotProduct));

			XMFLOAT3 slideVelocity;
			XMStoreFloat3(&slideVelocity, slide);

			//スライド位置
			XMFLOAT3 testPos = pos;
			testPos.x += slideVelocity.x;
			testPos.z += slideVelocity.z;

			XMFLOAT3 testMin = { testPos.x - halfSize, pos.y - 0.1f, testPos.z - halfSize };
			XMFLOAT3 testMax = { testPos.x + halfSize, pos.y + 0.1f, testPos.z + halfSize };

			if (!CheckWallCollisionLOD(testMin, testMax, this))
			{
				// スライド応用
				velocity.x = slideVelocity.x;
				velocity.z = slideVelocity.z;
				return;
			}
			else
			{
				//まだ壁に当たる場合、残りの速度を更新
				remainingVelocity = slideVelocity;

				// 速度を降ろす
				remainingVelocity.x *= 0.8f;
				remainingVelocity.z *= 0.8f;
			}
		}

		//止まる
		velocity.x = 0;
		velocity.z = 0;


	}
}

//接地判定
void PLAYER::HandleGroundCheck()
{


	const float groundThreshold = 0.2f;
	float groundY;
	if (CheckPlayerGroundSimple(pos, PLAYER_OFFSET_Y, groundY) && GetVelocity().y <= 0.0f)
	{
		float targetY = groundY;
		float distanceToGround = pos.y - targetY;
		if (distanceToGround <= groundThreshold)
		{
			pos.y = targetY;
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
	// ================== アンロック判定（この関数内で完結） ==================
	Inventory* inv = GetPlayerInventory();

	// inv から該当IDを持っているか調べる小ヘルパ（Count>0 も許可）
	auto hasItem = [](const std::vector<Item>& v, int id) -> bool {
		for (const auto& it : v) if (it.GetID() == id && it.GetCount() > 0) return true;
		return false;
		};

	// 武器（FireType）側のアンロック確認
	const auto& fireParts = inv->GetFireTypeParts(); // PART_REVOLVER / PART_SHUTGUN / PART_ROCKET を保持
	bool weaponUnlocked = false;
	switch (currentWeapon) {
	case WEAPON_REVOLVER:        weaponUnlocked = hasItem(fireParts, PART_REVOLVER); break;
	case WEAPON_SHOTGUN:         weaponUnlocked = hasItem(fireParts, PART_SHUTGUN);  break;
	case WEAPON_ROCKET_LAUNCHER: weaponUnlocked = hasItem(fireParts, PART_ROCKET);   break;
	default: weaponUnlocked = false; break;
	}
	if (!weaponUnlocked) {
		// （任意）PrintDebugProc("Locked weapon. Pick up the weapon part.\n");
		return; // 未解禁の武器は撃てない
	}

	// 弾（Ammo）側のアンロック確認
	const auto& ammoParts = inv->GetAmmoParts();     // PART_NORMAL_AMMO / PART_FIRE を保持
	bool ammoUnlocked = false;
	switch (currentBullet) {
	case BULLET_NORMAL: ammoUnlocked = hasItem(ammoParts, PART_NORMAL_AMMO); break;
	case BULLET_FIRE:   ammoUnlocked = hasItem(ammoParts, PART_FIRE);        break;
	default: ammoUnlocked = false; break;
	}
	if (!ammoUnlocked) {
		// （任意）PrintDebugProc("Locked ammo. Pick up the ammo part.\n");
		return; // 未解禁の弾は撃てない
	}
	// =====================================================================

	// 現在の弾種の“総弾数”ポインタを取得（既存仕様をそのまま使用）
	int* currentAmmo = (currentBullet == BULLET_NORMAL) ? &ammoNormal : &ammoFire;

	// 武器ごとの消費数（既存仕様）
	int requiredCost = 1;
	switch (currentWeapon) {
	case WEAPON_REVOLVER:         requiredCost = 1; break;
	case WEAPON_SHOTGUN:          requiredCost = 3; break;
	case WEAPON_ROCKET_LAUNCHER:  requiredCost = 5; break;
	}

	// クリックトリガ & 弾が足りる場合のみ発射（既存仕様）
	if (IsMouseLeftTriggered() && *currentAmmo >= requiredCost)
	{
		XMFLOAT3 pos = GetGunMuzzlePosition();
		XMFLOAT3 rot = GetGunMuzzleRotation();

		if (currentWeapon == WEAPON_REVOLVER)
		{
			SetRevolverBullet(currentBullet, pos, rot);
		}
		else if (currentWeapon == WEAPON_SHOTGUN)
		{
			SetShotgunBullet(currentBullet, pos, rot); // ばら撒きは既存のまま
		}
		else if (currentWeapon == WEAPON_ROCKET_LAUNCHER)
		{
			SetRocketLauncherBullet(currentBullet, pos, rot);
		}

		// 武器ごとのコストを消費（既存仕様）
		*currentAmmo -= requiredCost;
		if (*currentAmmo < 0) *currentAmmo = 0; // 念のため
	}
}

//void PLAYER::HandleReload()
//{
//	// Rキーでリロード処理
//	if (GetKeyboardTrigger(DIK_R))
//	{
//		Weapon* weapon = nullptr;
//		switch (currentWeapon)
//		{
//		case WEAPON_REVOLVER:
//			weapon = GetRevolver();
//			break;
//		case WEAPON_SHOTGUN:
//			weapon = GetShotgun();
//			break;
//		case WEAPON_ROCKET_LAUNCHER:
//			weapon = GetRocket_Launcher();
//			break;
//		}
//
//		int clipSize = weapon->clipSize;
//
//		int* ammo = (currentBullet == BULLET_NORMAL) ? &g_Player.ammoNormal : &g_Player.ammoFire;
//		int* maxAmmo = (currentBullet == BULLET_NORMAL) ? &g_Player.maxAmmoNormal : &g_Player.maxAmmoFire;
//
//		if (*ammo < clipSize && *maxAmmo > 0)
//		{
//			int need = clipSize - *ammo;
//			int reload = Min(need, *maxAmmo);
//			*ammo += reload;
//			*maxAmmo -= reload;
//		}
//	}
//}


XMFLOAT3 PLAYER::GetWallCollisionNormal(XMFLOAT3 currentPos, XMFLOAT3 moveVector, float halfSize)
{
	XMFLOAT3 rayStart = currentPos;
	rayStart.y += 1.0f;

	XMFLOAT3 rayDir = { moveVector.x * 2.0f, 0.0f, moveVector.z * 2.0f };

	return GetWallCollisionNormalLOD(rayStart, rayDir, 100.0f, this);
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
PLAYER* GetPlayer(void)
{
	return &g_Player;
}

WeaponType GetCurrentWeaponType(void)
{
	return g_Player.currentWeapon;
}

BulletType GetCurrentBulletType(void)
{
	return g_Player.currentBullet;
}


bool CheckPlayerGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY)
{
	// LOD版の地面判定
	XMFLOAT3 rayStart = pos;
	rayStart.y += 10.0f;

	XMFLOAT3 rayDir = { 0.0f, -10.0f, 0.0f }; // したへ10.0fの射線

	float hitDistance = 10.0f;
	XMFLOAT3 hitPos, hitNormal;

	if (CheckGroundCollisionLOD(rayStart, rayDir, &hitDistance, &hitPos, &hitNormal, &g_Player))
	{
		groundY = hitPos.y;
		return true;
	}

	return false;
}

void SavePlayerToFile() {
	PlayerSaveData data;
	data.weapon = static_cast<int>(g_Player.currentWeapon);
	data.bullet = static_cast<int>(g_Player.currentBullet);
	data.ammoNormal = g_Player.ammoNormal;
	data.ammoFire = g_Player.ammoFire;

	std::ofstream out(SAVE_FILE_PATH, std::ios::binary);
	if (out) {
		out.write(reinterpret_cast<const char*>(&data), sizeof(PlayerSaveData));
	}
}

void LoadPlayerFromFile() {
	PlayerSaveData data{};
	std::ifstream in(SAVE_FILE_PATH, std::ios::binary);
	if (in) {
		in.read(reinterpret_cast<char*>(&data), sizeof(PlayerSaveData));

		g_Player.currentWeapon = static_cast<WeaponType>(data.weapon);
		g_Player.currentBullet = static_cast<BulletType>(data.bullet);
		g_Player.ammoNormal = data.ammoNormal;
		g_Player.ammoFire = data.ammoFire;
	}
}


//プレイヤーのインベントリーを取得
Inventory* GetPlayerInventory(void) 
{
	PLAYER* player = GetPlayer();
	return &(player->inventory);
}

// ★追加: インベントリに該当パーツがあるかを調べるヘルパ
static bool HasItemById(const std::vector<Item>& list, int id) {
	for (const auto& it : list) if (it.GetID() == id) return true;
	return false;
}

// ★追加: 武器（FireType）のアンロック判定
static bool IsFireTypeUnlocked(const Inventory* inv, WeaponType w) {
	// Inventory 内の FireType パーツ一覧
	const auto& parts = inv->GetFireTypeParts(); // vector<Item>
	switch (w) {
	case WEAPON_REVOLVER:        return HasItemById(parts, PART_REVOLVER);
	case WEAPON_SHOTGUN:         return HasItemById(parts, PART_SHUTGUN);
	case WEAPON_ROCKET_LAUNCHER: return HasItemById(parts, PART_ROCKET);
	default: return false;
	}
}

// ★追加: 弾（Ammo）のアンロック判定
static bool IsAmmoUnlocked(const Inventory* inv, BulletType b) {
	const auto& parts = inv->GetAmmoParts(); // vector<Item>
	switch (b) {
	case BULLET_NORMAL: return HasItemById(parts, PART_NORMAL_AMMO);
	case BULLET_FIRE:   return HasItemById(parts, PART_FIRE);
	default: return false;
	}
}


bool PLAYER::IsWeaponUnlocked(int weapon) const
{
	return IsFireTypeUnlocked(&inventory, static_cast<WeaponType>(weapon));
}

bool PLAYER::IsBulletUnlocked(int bullet) const
{
	return IsAmmoUnlocked(&inventory, static_cast<BulletType>(bullet));
}
bool PLAYER::IsCurrentLoadoutUsable() const
{
	return IsWeaponUnlocked(static_cast<int>(currentWeapon))
		&& IsBulletUnlocked(static_cast<int>(currentBullet));
}