//=============================================================================
//
// 
// 
//
//=============================================================================
#pragma once
#include "enemy.h"
#include "spider.h"
#include "ghost.h"
#include "bug.h"
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


ID3D11ShaderResourceView* BaseEnemy::s_BloodTexture = nullptr;



//*****************************************************************************
// 
//*****************************************************************************
BaseEnemy::BaseEnemy() : pos({ 0,0,0 }), scl({ 1,1,1 }), use(false),
isDying(false), dissolveTimer(0.0f), dissolveAmount(0.0f), hasDroppedItems(false), dissolveTexture(nullptr)
{
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());

	//dissolveテクスチャ読み込み
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		GetDevice(),
		"data/TEXTURE/sampleNoise.png",
		NULL, NULL, &dissolveTexture, NULL);

	

}


BaseEnemy::~BaseEnemy() {
	//dissolveテクスチャ解放
	if (dissolveTexture) {
		dissolveTexture->Release();
		dissolveTexture = nullptr;
	}
}

bool BaseEnemy::LoadBloodTexture()
{
	if (s_BloodTexture) return true; 

	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		GetDevice(),
		"data/TEXTURE/bloodStain.png",  
		NULL, NULL, &s_BloodTexture, NULL);



	return SUCCEEDED(hr);
}

void BaseEnemy::UnloadBloodTexture()
{
	if (s_BloodTexture) {
		s_BloodTexture->Release();
		s_BloodTexture = nullptr;
	}
}

//*****************************************************************************
// 
//*****************************************************************************
void InitEnemy() {
	MakeVertexEnemy();

	BaseEnemy::LoadBloodTexture();

	g_enemies.clear();
	for (int i = 0; i < ENEMY_MAX; ++i) {

		//EnemySpawner(XMFLOAT3(-50.0f + i * 30.0f, -50.0f, 20.0f), SPIDER);
		//EnemySpawner(XMFLOAT3(-50.0f + i * 30.0f, -50.0f, 20.0f), BUG);
		//EnemySpawner(XMFLOAT3(0, -50.0f, 20.0f), BUG);
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

	BaseEnemy::UnloadBloodTexture();

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
	case BUG: { // BugEnemy
		BugEnemy* bug = new BugEnemy();
		bug->Init();
		bug->SetUsed(true);
		bug->SetPosition(position);
		newEnemy = bug;
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
		SpawnItem(dropPos, itemId);
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

	case BUG:
		dropItemAtOffset(ITEM_BUG);
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




