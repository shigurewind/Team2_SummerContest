//=============================================================================
//
// 
// 
//
//=============================================================================
#pragma once
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

#define ENEMY_OFFSET_Y  (-50.0f)


//*****************************************************************************
// 
//*****************************************************************************

BugEnemy::BugEnemy() :
	texture(nullptr), width(60.0f), height(60.0f)
{
	material = new MATERIAL{};
	XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}
BugEnemy::~BugEnemy() {
	if (texture) {
		texture->Release();
		texture = nullptr;
	}
	delete material;
	material = nullptr;
}

void BugEnemy::Init()
{
	D3DX11CreateShaderResourceViewFromFile(
		GetDevice(),
		"data/2Dpicture/enemy/bug01.png",
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
	maxFrames = 1;

	HP = 1;

	//幽霊は重力いらない
	EnableGravity(false);
}

void BugEnemy::Update()
{
	if (!use) return;

	frameCounter++;
	if (frameCounter >= frameInterval) {
		frameCounter = 0;
		currentFrame = (currentFrame + 1) % maxFrames;
	}


	// エネミーからプレイヤーまでのベクトル
	XMFLOAT3 dir;
	dir.x = GetPlayer()->GetPosition().x - pos.x;
	dir.y = GetPlayer()->GetPosition().y - pos.y;
	dir.z = GetPlayer()->GetPosition().z - pos.z;



	//// プレイヤーの座標までの計算
	XMFLOAT3 toPlayer = { dir.x, dir.y, dir.z };

	float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
	float range = 50.0f; // 発射範囲



	//攻撃行う範囲
	if (distSq < range * range)
	{
		ShowBugEffect(this);

		bugEffectTimer += 1.0f / 60.0f;

		if (bugEffectTimer >= 2.0f)
		{
			GetPlayer()->HP -= 1;
			if (GetPlayer()->HP < 0) GetPlayer()->HP = 0;

			bugEffectTimer = 0.0f;
		}
	}

	if (bugEffectVisible) return;
	BULLET* bullet = GetBullet();
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
				DropItems(pos, BUG);
			}
		}

	}

}

void BugEnemy::Draw()
{
	if (!use || !texture || !g_VertexBufferEnemy) return;
#ifdef _DEBUG

	if (IsBugEffectActive()) {
		PrintDebugProc("Bug effect is active! Enemy will not draw.\n");
		return;
	}
#endif

	if (IsBugEffectActive()) return;

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

void BugEnemy::NormalMovement()
{
}

void BugEnemy::Attack()
{
}

