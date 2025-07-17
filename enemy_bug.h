#pragma once
#include"enemy.h"
#include"renderer.h"
#include "player.h"
#include "bullet.h"
#include "debugproc.h"
#include "camera.h"
#include "main.h"
#include "sprite.h"
#include "input.h"
#include "collision.h"
#include "GameUI.h"
#include "item.h"
#include <cstdlib>
#include <ctime>


class BugEnemy : public BaseEnemy {
public:
	BugEnemy();
	~BugEnemy();

	void Init() override;
	void Update() override;
	void Draw() override;

	void NormalMovement() override;
	void Attack() override;

private:
	ID3D11ShaderResourceView* texture;
	struct MATERIAL* material;
	float width, height;
	float speed, size;						//エネミーのスピード
	XMFLOAT3 moveDir;       // 現在の動き方向
	float moveChangeTimer;  // 向き変わるタイマー


	int currentFrame;
	int frameCounter;
	int frameInterval;
	int maxFrames;

	float time = 0.0f;
	int tblNo = 0;
	int tblMax = 0;

	//エネミーが発射するとき
	float fireTimer = 0.0f;
	const float fireCooldown = 1.0f;

	float attackCooldownTimer;  // 攻撃間の待つ時間
	float attackCooldown;

	bool isAttacking;
	float attackFrameTimer;
};
