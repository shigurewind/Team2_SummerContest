//=============================================================================
//
// 
// 
//
//=============================================================================
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
#include "navmesh.h"
#include "Octree.h"
#include "object.h"



//*****************************************************************************
//
//*****************************************************************************

std::vector<BaseEnemy*> g_enemies;
ID3D11Buffer* g_VertexBufferEnemy = nullptr;

#define ENEMY_MAX (1)

#define ENEMY_OFFSET_Y  (-50.0f)



static std::vector<NavNode> g_NavMeshNodes;
static bool g_NavMeshBuilt = false;

//PLAYER* player = GetPlayer();
BULLET* bullet = GetBullet();

//*****************************************************************************
// 
//*****************************************************************************
BaseEnemy::BaseEnemy() : scl({ 1,1,1 }) {
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
BaseEnemy::~BaseEnemy() {}

SpiderEnemy::SpiderEnemy() {
	material = new MATERIAL{}; 
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

	SetPosition(XMFLOAT3(0.0f, -50.0f, 20.0f));
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	use = true;

	speed = 0.5f;
	dropRate = 0.5f;

	currentFrame = 0;
	frameCounter = 0;
	frameInterval = 15;
	maxFrames = 3;

	isAttacking = false;
	attackFrameTimer = 0.0f;
	attackCooldownTimer = 0.0f;
	attackCooldown = 1.5f;

	moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	moveChangeTimer = 2.0f;

	minDistance = 100.0f;
	HP = 1;

	EnableGravity(false);
	SetMaxFallSpeed(6.0f);
}

void SpiderEnemy::Update() {
	if (!use) return;

	attackCooldownTimer -= 1.0f / 60.0f;
	if (attackCooldownTimer < 0.0f) attackCooldownTimer = 0.0f;

	if (!g_NavMeshBuilt) {
		g_NavMeshNodes = GenerateNavMeshFromFloorTriangles();
		g_NavMeshBuilt = true;
	}
	pathUpdateTimer -= 1.0f / 60.0f;
	if (pathUpdateTimer <= 0.0f) {
		pathUpdateTimer = pathUpdateInterval;
		XMFLOAT3 start = GetPosition();
		XMFLOAT3 goal = GetPlayer()->GetPosition();
		FindPathAStar(start, goal, g_NavMeshNodes, pathPoints);
		currentPathIndex = 0;
	}

	if (isAttacking) {
		attackFrameTimer -= 1.0f / 60.0f;
		if (attackFrameTimer <= 0.0f) {
			isAttacking = false;
			frameCounter = 0;
			currentFrame = 0;
		}
		else {
			currentFrame = 2;
		}
	}
	else {
		frameCounter++;
		if (frameCounter >= frameInterval) {
			frameCounter = 0;
			currentFrame = (currentFrame + 1) % 2;
		}
	}

	ZeroXZVelocity();

	if (!pathPoints.empty() && currentPathIndex < (int)pathPoints.size()) {
		XMFLOAT3 self = GetPosition();
		XMFLOAT3 target = pathPoints[currentPathIndex];

		XMFLOAT3 dir = { target.x - self.x, 0.0f, target.z - self.z };
		float distSq = dir.x * dir.x + dir.z * dir.z;

		if (distSq < 4.0f) {
			currentPathIndex++;
		}
		else {
			XMVECTOR vdir = XMVector3Normalize(XMLoadFloat3(&dir));
			XMStoreFloat3(&dir, vdir);
			XMFLOAT3 v = GetVelocity();
			v.x = dir.x * speed;
			v.z = dir.z * speed;
			SetVelocity(v);
		}
	}

	XMFLOAT3 self = GetPosition();
	XMFLOAT3 ply = GetPlayer()->GetPosition();
	XMFLOAT3 d = { ply.x - self.x, 0.0f, ply.z - self.z };
	float distSq = d.x * d.x + d.z * d.z;
	float range = 200.0f;

	if (distSq < range * range) {
		ChasingPlayer(speed, range);
		if (!isAttacking && attackCooldownTimer <= 0.0f) {
			Attack();
		}
	}
	else {
		NormalMovement();
	}

	for (int i = 0; i < MAX_BULLET; i++) {
		if (!bullet[i].use) continue;
		if (CheckSphereAABBCollision(bullet[i].pos, bullet[i].size,
			GetPosition(), GetColliderHalf()))
		{
			bullet[i].use = false;
			if (--HP <= 0) { use = false; DropItems(GetPosition(), SPIDER); }
		}
	}

	PhysicsStepAndResolve();

#ifdef _DEBUG
	PrintDebugProc("Enemy Pos: X:%f Y:%f Z:%f\n",
		GetPosition().x, GetPosition().y, GetPosition().z);
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
	XMFLOAT3 p = GetPosition();
	XMMATRIX mtxTranslate = XMMatrixTranslation(p.x, p.y, p.z);
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
	moveChangeTimer -= 1.0f / 60.0f;
	if (moveChangeTimer <= 0.0f) {
		moveChangeTimer = 2.0f;
		int dir = rand() % 4;
		switch (dir) {
		case 0: moveDir = XMFLOAT3(1.0f, 0.0f, 0.0f); break;
		case 1: moveDir = XMFLOAT3(-1.0f, 0.0f, 0.0f); break;
		case 2: moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f); break;
		case 3: moveDir = XMFLOAT3(0.0f, 0.0f, -1.0f); break;
		}
	}

	XMFLOAT3 v = GetVelocity();
	v.x = moveDir.x * speed;
	v.z = moveDir.z * speed;
	SetVelocity(v);

	const float minX = -200.0f, maxX = 200.0f;
	const float minZ = -100.0f, maxZ = 100.0f;
	XMFLOAT3 p = GetPosition();
	if (p.x <= minX || p.x >= maxX || p.z <= minZ || p.z >= maxZ) {
		moveChangeTimer = 0.0f;
	}
}
void SpiderEnemy::Attack()
{
	if (attackCooldownTimer <= 0.0f && !isAttacking)
	{
		isAttacking = true;
		GetPlayer()->HP -= 1;
		attackFrameTimer = 0.5f;              // çUåÇÇÃÉtÉåÅ[ÉÄÇÃï`âÊÇÃéûä‘
		currentFrame = 2;                     // çUåÇÇÃÉtÉåÅ[ÉÄÇÃï`âÊ
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


void BaseEnemy::SetPosition(const XMFLOAT3& p) { Object::SetPosition(p); }

XMFLOAT3 BaseEnemy::GetPosition() const { return Object::GetPosition(); }

void BaseEnemy::SetScale(const XMFLOAT3& s) {
	scl = s;
}

XMFLOAT3 BaseEnemy::GetScale() const {
	return scl;
}

void BaseEnemy::ChasingPlayer(float speed, float chaseRange)
{
	XMFLOAT3 self = GetPosition();
	XMFLOAT3 ply = GetPlayer()->GetPosition();

	XMFLOAT3 dir = { ply.x - self.x, 0.0f, ply.z - self.z };
	float distSq = dir.x * dir.x + dir.z * dir.z;

	float maxSq = chaseRange * chaseRange;
	float minSq = minDistance * minDistance;

	ZeroXZVelocity();

	if (distSq < maxSq && distSq > minSq) {
		XMVECTOR vdir = XMVector3Normalize(XMLoadFloat3(&dir));
		XMStoreFloat3(&dir, vdir);

		XMFLOAT3 v = GetVelocity();
		v.x = dir.x * speed;
		v.z = dir.z * speed;
		SetVelocity(v);
	}
}
void BaseEnemy::ZeroXZVelocity()
{
	XMFLOAT3 v = GetVelocity();
	v.x = 0.0f;
	v.z = 0.0f;
	SetVelocity(v);
}

void BaseEnemy::PhysicsStepAndResolve()
{
	Object::Update();
	Object::HandleGroundCheck(FootOffset());
	XMFLOAT3 push{};
	XMFLOAT3 c = GetPosition();
	XMFLOAT3 h = GetColliderHalf();

	if (Oct::IntersectWallAABB(c, h, push)) {
		c.x += push.x; c.y += push.y; c.z += push.z;
		SetPosition(c);

		if (push.y > 0.0f) {
			XMFLOAT3 v = GetVelocity();
			v.y = 0.0f;
			SetVelocity(v);
			isGround = true;
		}
	}
	else {
		isGround = false;
	}
}

//*****************************************************************************
// 
//*****************************************************************************

GhostEnemy::GhostEnemy() {
	material = new MATERIAL{};
	
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

	SetPosition(XMFLOAT3(0.0f, 0.0f, ENEMY_OFFSET_Y));
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	use = true;

	moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	moveChangeTimer = 2.0f;
	speed = 0.5f;

	currentFrame = 0;
	frameCounter = 0;
	frameInterval = 15;
	maxFrames = 2;

	HP = 50;

	EnableGravity(true);
}
void GhostEnemy::Update()
{
	frameCounter++;
	if (frameCounter >= frameInterval) {
		frameCounter = 0;
		currentFrame = (currentFrame + 1) % maxFrames;
	}
	if (!use) return;

	ZeroXZVelocity();

	XMFLOAT3 self = GetPosition();
	XMFLOAT3 ply = GetPlayer()->GetPosition();
	XMFLOAT3 d = { ply.x - self.x, ply.y - self.y, ply.z - self.z };

	float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
	float range = 100.0f;

	XMFLOAT3 v = GetVelocity();
	if (distSq < range * range) {
		XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&d));
		XMStoreFloat3(&d, dir);
		v.x = d.x * speed;
		v.y = d.y * speed; 
		v.z = d.z * speed;
	}
	else {
		NormalMovement();
		v = GetVelocity();
	}
	SetVelocity(v);

	for (int i = 0; i < MAX_BULLET; i++) {
		if (!bullet[i].use) continue;
		if (CheckSphereAABBCollision(bullet[i].pos, bullet[i].size,
			GetPosition(), GetColliderHalf()))
		{
			bullet[i].use = false;
			if (--HP <= 0) { use = false; DropItems(GetPosition(), GHOST); }
		}
	}

	Object::Update();
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
	XMFLOAT3 p = GetPosition();
	XMMATRIX mtxTranslate = XMMatrixTranslation(p.x, p.y, p.z);
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
	moveChangeTimer -= 1.0f / 60.0f;
	if (moveChangeTimer <= 0.0f) {
		moveChangeTimer = 2.0f;
		int dir = rand() % 6;
		switch (dir) {
		case 0: moveDir = XMFLOAT3(1.0f, 0.0f, 0.0f); break;
		case 1: moveDir = XMFLOAT3(-1.0f, 0.0f, 0.0f); break;
		case 2: moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f); break;
		case 3: moveDir = XMFLOAT3(0.0f, 0.0f, -1.0f); break;
		case 4: moveDir = XMFLOAT3(0.0f, 1.0f, 0.0f); break;
		case 5: moveDir = XMFLOAT3(0.0f, -1.0f, 0.0f); break;
		}
	}
	XMFLOAT3 v = GetVelocity();
	v.x = moveDir.x * speed;
	v.y = moveDir.y * speed;
	v.z = moveDir.z * speed;
	SetVelocity(v);
}

void GhostEnemy::Attack()
{
}

