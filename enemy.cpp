//=============================================================================
//
// 
// 
//
//=============================================================================
#pragma once
#include "enemy.h"
#include "player.h"
#include "bullet.h"
#include "debugproc.h"
#include "camera.h"
#include "main.h"
#include "renderer.h"
#include "sprite.h"
#include "input.h"
#include "collision.h"
#include "GameUI.h"
#include "item.h"
#include <cstdlib>
#include <ctime>




//*****************************************************************************
//
//*****************************************************************************

std::vector<BaseEnemy*> g_enemies;
ID3D11Buffer* g_VertexBufferEnemy = nullptr;

#define ENEMY_MAX (1)
static BOOL g_bAlphaTestEnemy;

#define ENEMY_OFFSET_Y  (-50.0f)




//PLAYER* player = GetPlayer();
BULLET* bullet = GetBullet();

//*****************************************************************************
// 
//*****************************************************************************
BaseEnemy::BaseEnemy() : pos({ 0,0,0 }), scl({ 1,1,1 }), use(false) {
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
BaseEnemy::~BaseEnemy() {}

SpiderEnemy::SpiderEnemy() :
	texture(nullptr), width(100.0f), height(100.0f)
{
	material = new MATERIAL{};
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
SpiderEnemy::~SpiderEnemy() {
	if (texture) {
		texture->Release();
		texture = nullptr;
	}
	delete material;
	material = nullptr;
}

void SpiderEnemy::Init() {
	D3DX11CreateShaderResourceViewFromFile(
		GetDevice(),
		"data/2Dpicture/enemy/enemy001.png",
		NULL, NULL, &texture, NULL);


	*material = {};
	material->Diffuse = XMFLOAT4(1, 1, 1, 1);

	pos = XMFLOAT3(0.0f, -50.0f, 20.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	use = true;
	speed = 1.0f;
	dropRate = 0.5f;

	currentFrame = 0;
	frameCounter = 0;
	frameInterval = 15;//change speed
	maxFrames = 3;

	tblNo = 0;
	//tblMax = _countof(g_MoveTbl0);
	time = 0.0f;

	isAttacking = false;
	attackFrameTimer = 0.0f;
	attackCooldownTimer = 0.0f;
	attackCooldown = 1.5f;  // 1.5 秒ことに攻撃する

	moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f);       // 現在の動き方向
	moveChangeTimer = 2.0f;  // 向き変わるタイマー
	speed = 0.5f;			//エネミーのスピード
	currentFrame = 0;
	frameCounter = 0;
	frameInterval = 15;//change speed


	minDistance = 100.0f;

	HP = 1;

	EnableGravity(true);
	SetMaxFallSpeed(6.0f);
}

void SpiderEnemy::Update() {
	if (!use) return;


	if (isAttacking)    //攻撃のアニメーション処理
	{
		attackFrameTimer -= 1.0f / 60.0f;
		if (attackFrameTimer <= 0.0f) {
			isAttacking = false;
			frameCounter = 0;
			currentFrame = 0;
		}
		else
		{
			currentFrame = 2;
		}
	}
	else
	{
		// 移動のアニメーション処理
		frameCounter++;
		if (frameCounter >= frameInterval) {
			frameCounter = 0;
			currentFrame = (currentFrame + 1) % 2;
		}
	}


	// エネミーからプレイヤーまでのベクトル
	XMFLOAT3 dir;
	dir.x = GetPlayer()->GetPosition().x - pos.x;
	dir.y = 0.0f;
	dir.z = GetPlayer()->GetPosition().z - pos.z;



	//// プレイヤーの座標までの計算
	XMFLOAT3 toPlayer = { dir.x, dir.y, dir.z };

	float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
	float range = 200.0f; // 発射範囲

	attackCooldownTimer -= 1.0f / 60.0f;
	if (attackCooldownTimer < 0.0f) attackCooldownTimer = 0.0f;


	//プレイヤーを追いかける行う範囲
	if (distSq < range * range)
	{
		ChasingPlayer(speed, range);

		if (!isAttacking && attackCooldownTimer <= 0.0f)
		{
			Attack();
		}

	}
	else
	{
		NormalMovement();
	}


	//弾と当たり判定？
	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!bullet[i].use) continue;

		XMFLOAT3 enemyHalfSize = { width, height - 20.0f, 50.f }; //エネミーの当たり判定のサイズ

		if (CheckSphereAABBCollision(bullet[i].pos, bullet[i].size, pos, enemyHalfSize))
		{
			bullet[i].use = false;
			HP -= 1;
			if (HP <= 0)
			{
				use = false;
				DropItems(pos, SPIDER);
			}
		}

	}


#ifdef _DEBUG

	float dist = sqrtf(distSq);
	PrintDebugProc("Enemy Pos: X:%f Y:%f Z:%f\n", pos.x, pos.y, pos.z);

#endif
}

void SpiderEnemy::Draw() {

	if (!use || !texture || !g_VertexBufferEnemy) return;


	SetLightEnable(FALSE);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferEnemy, &stride, &offset);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	CAMERA* cam = GetCamera();
	XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);

	XMMATRIX mtxWorld = XMMatrixIdentity();
	mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

	XMMATRIX mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
	XMMATRIX mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);


	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	VERTEX_3D* v = (VERTEX_3D*)msr.pData;

	float w = width, h = height;
	v[0].Position = XMFLOAT3(-w / 2, h, 0);
	v[1].Position = XMFLOAT3(w / 2, h, 0);
	v[2].Position = XMFLOAT3(-w / 2, 0, 0);
	v[3].Position = XMFLOAT3(w / 2, 0, 0);

	for (int i = 0; i < 4; ++i) {
		v[i].Normal = XMFLOAT3(0, 0, -1);
		v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
	}

	float tw = 1.0f / maxFrames;
	float th = 1.0f;
	float tx = currentFrame * tw;
	float ty = 0.0f;

	v[0].TexCoord = XMFLOAT2(tx, ty);
	v[1].TexCoord = XMFLOAT2(tx + tw, ty);
	v[2].TexCoord = XMFLOAT2(tx, ty + th);
	v[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

	GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);

	SetAlphaTestEnable(FALSE);
	SetBlendState(BLEND_MODE_ALPHABLEND);
	SetWorldMatrix(&mtxWorld);
	SetMaterial(*material);
	GetDeviceContext()->PSSetShaderResources(0, 1, &texture);


	GetDeviceContext()->Draw(4, 0);

}
void SpiderEnemy::NormalMovement()
{

	// 動き方向変わりタイマー
	moveChangeTimer -= 1.0f / 60.0f; // 60fpsことに動き方向変わり
	if (moveChangeTimer <= 0.0f) {
		moveChangeTimer = 2.0f; // reset timer

		// 動き方向変わることはランダム設定
		int dir = rand() % 4;
		switch (dir) {
		case 0: moveDir = XMFLOAT3(1.0f, 0.0f, 0.0f); break;  // 右
		case 1: moveDir = XMFLOAT3(-1.0f, 0.0f, 0.0f); break; // 左
		case 2: moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f); break;  // 前（+ｚ）
		case 3: moveDir = XMFLOAT3(0.0f, 0.0f, -1.0f); break; // 後ろ（-ｚ）
		}
	}

	// 新しい位置を計算
	XMFLOAT3 newPos = pos;
	newPos.x += moveDir.x * speed;
	newPos.y += moveDir.y * speed;
	newPos.z += moveDir.z * speed;

	// 範囲制限（例えば：XとZは -50.0f ? +50.0f）
	const float minX = -200.0f;
	const float maxX = 200.0f;
	const float minZ = -100.0f;
	const float maxZ = 100.0f;

	// 範囲内なら移動
	if (newPos.x >= minX && newPos.x <= maxX &&
		newPos.z >= minZ && newPos.z <= maxZ) {
		pos = newPos;
	}
	else {
		// 範囲外に出そうなら方向を変える
		moveChangeTimer = 0.0f; // すぐ次の方向へ変更
	}
}
void SpiderEnemy::Attack()
{
	if (attackCooldownTimer <= 0.0f && !isAttacking)
	{
		isAttacking = true;
		GetPlayer()->HP -= 1;
		attackFrameTimer = 0.5f;              // 攻撃のフレームの描画の時間
		currentFrame = 2;                     // 攻撃のフレームの描画
		attackCooldownTimer = attackCooldown; // Reset cooldown

		ShowWebEffect(0.5f);
	}
}
//*****************************************************************************
// 
//*****************************************************************************
void InitEnemy() {
	MakeVertexEnemy();
	g_enemies.clear();
	for (int i = 0; i < ENEMY_MAX; ++i) {

		EnemySpawner(XMFLOAT3(-50.0f + i * 30.0f, -50.0f, 20.0f), SPIDER);
		EnemySpawner(XMFLOAT3(-50.0f + i * 30.0f, 0.0f, 20.0f), GHOST);

	}
}

void UpdateEnemy() {
	for (auto enemy : g_enemies) {
		if (enemy->IsUsed()) enemy->Update();
	}
}

void DrawEnemy() {
	for (auto enemy : g_enemies) {
		if (enemy->IsUsed()) enemy->Draw();
	}


#ifdef _DEBUG
	for (auto enemy : g_enemies)
	{
		if (enemy->IsUsed())
		{
			XMFLOAT3 pos = enemy->GetPosition();
			PrintDebugProc("Enemy Pos: X:%f Y:%f Z:%f\n", pos.x, pos.y, pos.z);
			break;
		}
	}
#endif
}

void UninitEnemy() {
	for (auto enemy : g_enemies) {
		delete enemy;
	}
	g_enemies.clear();

	if (g_VertexBufferEnemy) {
		g_VertexBufferEnemy->Release();
		g_VertexBufferEnemy = nullptr;
	}
}

std::vector<BaseEnemy*>& GetEnemies() {
	return g_enemies;
}

//*****************************************************************************
// 
//*****************************************************************************

HRESULT MakeVertexEnemy() {
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = GetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBufferEnemy);
	if (FAILED(hr)) return hr;

	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	VERTEX_3D* v = (VERTEX_3D*)msr.pData;

	float w = 60.0f, h = 90.0f;
	v[0].Position = XMFLOAT3(-w / 2, h, 0);
	v[1].Position = XMFLOAT3(w / 2, h, 0);
	v[2].Position = XMFLOAT3(-w / 2, 0, 0);
	v[3].Position = XMFLOAT3(w / 2, 0, 0);

	for (int i = 0; i < 4; ++i) {
		v[i].Normal = XMFLOAT3(0, 0, -1);
		v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
	}

	v[0].TexCoord = XMFLOAT2(0, 0);
	v[1].TexCoord = XMFLOAT2(1, 0);
	v[2].TexCoord = XMFLOAT2(0, 1);
	v[3].TexCoord = XMFLOAT2(1, 1);

	GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);
	return S_OK;
}

void EnemySpawner(XMFLOAT3 position, int type) {
	BaseEnemy* newEnemy = nullptr;

	switch (type) {
	case SPIDER: { // SpiderEnemy
		SpiderEnemy* spider = new SpiderEnemy();
		spider->Init();
		spider->SetUsed(true);
		spider->SetPosition(position);
		newEnemy = spider;
		break;
	}
	case GHOST: { // GhostEnemy
		GhostEnemy* ghost = new GhostEnemy();
		ghost->Init();
		ghost->SetUsed(true);
		ghost->SetPosition(position);
		newEnemy = ghost;
		break;
	}
	default:
		return;
	}

	if (newEnemy) {
		g_enemies.push_back(newEnemy);
	}
}

void DropItems(const XMFLOAT3& pos, ENEMY_TYPE enemyType)
{
	int itemCount = 0;

	auto getRandomOffsetX = []() -> float {
		return ((float)(rand() % 41) - 20.0f);
		};

	auto dropItemAtOffset = [&](int itemId) {
		XMFLOAT3 dropPos = pos;
		dropPos.x += getRandomOffsetX();
		SetItem(dropPos, itemId);
		};

	float random = (float)rand() / RAND_MAX;
	switch (enemyType)
	{
	case SPIDER:
		dropItemAtOffset(ITEM_APPLE);  // Apple 100%
		if (random < 0.5f)  // San 50%
		{
			dropItemAtOffset(ITEM_SAN);
		}
		if (random < 0.2f)  // fire 20%
		{
			dropItemAtOffset(PART_FIRE);
		}
		if (random < 0.2f)  // shutgun 20%
		{
			dropItemAtOffset(PART_SHUTGUN);
		}
		if (random < 0.5f)  // bullet 50%
		{
			dropItemAtOffset(ITEM_BULLET);
		}
		break;

	case GHOST:
		if (random < 0.5f)  // Apple 50%
		{
			dropItemAtOffset(ITEM_APPLE);
		}
		if (random < 0.2f)  // San 20%
		{
			dropItemAtOffset(ITEM_SAN);
		}
		if (random < 0.2f)  // fire 20%
		{
			dropItemAtOffset(PART_FIRE);
		}
		if (random < 0.2f)  // shutgun 20%
		{
			dropItemAtOffset(PART_SHUTGUN);
		}
		if (random < 0.5f)  // bullet 50%
		{
			dropItemAtOffset(ITEM_BULLET);
		}
		break;

	default:
		break;
	}
}

//*****************************************************************************
// 
//*****************************************************************************


void BaseEnemy::SetPosition(const XMFLOAT3& p) {
	pos = p;
}

XMFLOAT3 BaseEnemy::GetPosition() const {
	return pos;
}

void BaseEnemy::SetScale(const XMFLOAT3& s) {
	scl = s;
}

XMFLOAT3 BaseEnemy::GetScale() const {
	return scl;
}

void BaseEnemy::ChasingPlayer(float speed, float chaseRange)
{


	// エネミーからプレイヤーまでのベクトル
	XMFLOAT3 dir;
	dir.x = GetPlayer()->GetPosition().x - pos.x;
	dir.y = 0.0f;
	dir.z = GetPlayer()->GetPosition().z - pos.z;

	float distSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;

	float maxSq = chaseRange * chaseRange;
	float minSq = minDistance * minDistance;

	if (distSq < maxSq && distSq > minSq) {
		// 正規化ベクトル
		XMVECTOR vec = XMVector3Normalize(XMLoadFloat3(&dir));
		XMStoreFloat3(&dir, vec);

		// 位置アップデート
		pos.x += dir.x * speed;
		pos.y += dir.y * speed;
		pos.z += dir.z * speed;
	}
}



//*****************************************************************************
// 
//*****************************************************************************

GhostEnemy::GhostEnemy() :
	texture(nullptr), width(100.0f), height(100.0f)
{
	material = new MATERIAL{};
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
GhostEnemy::~GhostEnemy() {
	if (texture) {
		texture->Release();
		texture = nullptr;
	}
	delete material;
	material = nullptr;
}

void GhostEnemy::Init()
{
	D3DX11CreateShaderResourceViewFromFile(
		GetDevice(),
		"data/2Dpicture/enemy/ghost.png",
		NULL, NULL, &texture, NULL);


	*material = {};
	material->Diffuse = XMFLOAT4(1, 1, 1, 1);

	pos = XMFLOAT3(0.0f, 0.0f, ENEMY_OFFSET_Y);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	use = true;
	moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f);       // 現在の動き方向
	moveChangeTimer = 2.0f;  // 向き変わるタイマー
	speed = 0.5f;			//エネミーのスピード
	currentFrame = 0;
	frameCounter = 0;
	frameInterval = 15;//change speed
	maxFrames = 2;

	HP = 50;

	//幽霊は重力いらない
	EnableGravity(false);
}

void GhostEnemy::Update()
{
	frameCounter++;
	if (frameCounter >= frameInterval) {
		frameCounter = 0;
		currentFrame = (currentFrame + 1) % maxFrames;
	}


	if (!use) return;



	// エネミーからプレイヤーまでのベクトル
	XMFLOAT3 dir;
	dir.x = GetPlayer()->GetPosition().x - pos.x;
	dir.y = GetPlayer()->GetPosition().y - pos.y;
	dir.z = GetPlayer()->GetPosition().z - pos.z;



	//// プレイヤーの座標までの計算
	XMFLOAT3 toPlayer = { dir.x, dir.y, dir.z };

	float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
	float range = 100.0f; // 発射範囲



	//攻撃行う範囲
	if (distSq < range * range)
	{
		ChasingPlayer(speed, range);
	}
	else
	{
		NormalMovement();

	}

	//弾と当たり判定？
	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!bullet[i].use) continue;

		XMFLOAT3 enemyHalfSize = { width / 2, height, 50.f }; //エネミーの当たり判定のサイズ

		if (CheckSphereAABBCollision(bullet[i].pos, bullet[i].size, pos, enemyHalfSize))
		{
			bullet[i].use = false;
			HP -= 1;
			if (HP <= 0)
			{
				use = false;
				DropItems(pos, GHOST);
			}
		}

	}



#ifdef _DEBUG

	float dist = sqrtf(distSq);
	PrintDebugProc("Enemy2 HP: %d\n", HP);

#endif

}

void GhostEnemy::Draw()
{
	if (!use || !texture || !g_VertexBufferEnemy) return;


	SetLightEnable(FALSE);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferEnemy, &stride, &offset);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	CAMERA* cam = GetCamera();
	XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);

	XMMATRIX mtxWorld = XMMatrixIdentity();
	mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
	mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
	mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

	mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
	mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
	mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

	mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
	mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
	mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

	XMMATRIX mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
	XMMATRIX mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);


	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	VERTEX_3D* v = (VERTEX_3D*)msr.pData;

	float w = width, h = height;
	v[0].Position = XMFLOAT3(-w / 2, h, 0);
	v[1].Position = XMFLOAT3(w / 2, h, 0);
	v[2].Position = XMFLOAT3(-w / 2, 0, 0);
	v[3].Position = XMFLOAT3(w / 2, 0, 0);

	for (int i = 0; i < 4; ++i) {
		v[i].Normal = XMFLOAT3(0, 0, -1);
		v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
	}

	float tw = 1.0f / maxFrames;
	float th = 1.0f;
	float tx = currentFrame * tw;
	float ty = 0.0f;

	v[0].TexCoord = XMFLOAT2(tx, ty);
	v[1].TexCoord = XMFLOAT2(tx + tw, ty);
	v[2].TexCoord = XMFLOAT2(tx, ty + th);
	v[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

	GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);

	SetAlphaTestEnable(FALSE);
	SetBlendState(BLEND_MODE_ALPHABLEND);
	SetWorldMatrix(&mtxWorld);
	SetMaterial(*material);
	GetDeviceContext()->PSSetShaderResources(0, 1, &texture);


	GetDeviceContext()->Draw(4, 0);

}

void GhostEnemy::NormalMovement()
{
	// 動き方向変わりタイマー
	moveChangeTimer -= 1.0f / 60.0f; // 60fpsことに動き方向変わり
	if (moveChangeTimer <= 0.0f) {
		moveChangeTimer = 2.0f; // reset timer

		// 動き方向変わることはランダム設定
		int dir = rand() % 6;
		switch (dir)
		{
		case 0: moveDir = XMFLOAT3(1.0f, 0.0f, 0.0f); break;  // 右
		case 1: moveDir = XMFLOAT3(-1.0f, 0.0f, 0.0f); break; // 左
		case 2: moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f); break;  // 前（+ｚ）
		case 3: moveDir = XMFLOAT3(0.0f, 0.0f, -1.0f); break; // 後ろ（-ｚ）
		case 4: moveDir = XMFLOAT3(0.0f, 1.0f, 0.0f); break; // 上（+ｙ）
		case 5: moveDir = XMFLOAT3(0.0f, -1.0f, 0.0f); break; // 下（-ｙ）
		}
	}

	// 新しい位置を計算
	XMFLOAT3 newPos = pos;
	newPos.x += moveDir.x * speed;
	newPos.y += moveDir.y * speed;
	newPos.z += moveDir.z * speed;

	// 範囲制限（例えば：XとZは -50.0f ? +50.0f）
	const float minX = -200.0f;
	const float maxX = 200.0f;
	const float minZ = -100.0f;
	const float maxZ = 100.0f;
	const float minY = -50.0f;
	const float maxY = 100.0f;

	// 範囲内なら移動
	if (newPos.x >= minX && newPos.x <= maxX &&
		newPos.y >= minY && newPos.y <= maxY &&
		newPos.z >= minZ && newPos.z <= maxZ) {
		pos = newPos;
	}
	else {
		// 範囲外に出そうなら方向を変える
		moveChangeTimer = 0.0f; // すぐ次の方向へ変更
	}
}

void GhostEnemy::Attack()
{
}

