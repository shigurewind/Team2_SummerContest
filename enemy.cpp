#include "enemy.h"
#include "renderer.h"  // 假设里面有 LoadTexture(), DrawSprite3D()
#include "debugproc.h"

BaseEnemy::BaseEnemy()
	: pos({ 0,0,0 }), scl({ 1,1,1 }), use(false)
{
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}

BaseEnemy::~BaseEnemy() {}

//----------------------------

ScarecrowEnemy::ScarecrowEnemy()
	: texID(-1)
{
}

ScarecrowEnemy::~ScarecrowEnemy() {}

void ScarecrowEnemy::Init()
{
	texID = LoadTexture("data/TEXTURE/scarecrow.png");  // 替换成你实际贴图路径
	pos = XMFLOAT3(0.0f, 7.0f, 20.0f);  // 显示位置
	scl = XMFLOAT3(1.5f, 1.5f, 1.5f);  // 缩放
	use = true;
}

void ScarecrowEnemy::Update()
{
	// 不动，无需逻辑
}

void ScarecrowEnemy::Draw()
{
	XMMATRIX mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
	XMMATRIX mtxTrans = XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX mtxWorld = XMMatrixMultiply(mtxScl, mtxTrans);
	XMStoreFloat4x4(&this->mtxWorld, mtxWorld);

	SetWorldMatrix(&this->mtxWorld);
	DrawSprite3D(texID);
}




static BaseEnemy* g_enemy = nullptr;

void InitEnemy()
{
	g_enemy = new ScarecrowEnemy();
	g_enemy->Init();
}

void UpdateEnemy()
{
	if (g_enemy && g_enemy->IsUsed()) g_enemy->Update();
}

void DrawEnemy()
{
	if (g_enemy && g_enemy->IsUsed()) g_enemy->Draw();
}

void UninitEnemy()
{
	if (g_enemy)
	{
		delete g_enemy;
		g_enemy = nullptr;
	}
}