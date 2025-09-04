#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Object.h"
#include "enemy.h"

using namespace DirectX;

//*****************************************************************************
// 
//*****************************************************************************

class GhostEnemy : public BaseEnemy {
public:
	GhostEnemy();
	~GhostEnemy();

	void Init() override;
	void Update() override;
	void Draw() override;

	void NormalMovement() override;
	void Attack() override;


private:
	ID3D11ShaderResourceView* texture;
	struct MATERIAL* material;
	float width, height;
	XMFLOAT3 moveDir;       // 現在の動き方向
	float moveChangeTimer;  // 向き変わるタイマー
	float speed;			//エネミーのスピード

	int currentFrame;
	int frameCounter;
	int frameInterval;
	int maxFrames;



	//エネミーが発射するとき
	float fireTimer = 0.0f;
	const float fireCooldown = 1.0f;

	float attackCooldownTimer;  // 攻撃間の待つ時間
	float attackCooldown;

	bool isAttacking;
	float attackFrameTimer;


};



