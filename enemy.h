#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Object.h"

using namespace DirectX;

//*****************************************************************************
// 
//*****************************************************************************
class BaseEnemy : public Object
{
public:
	BaseEnemy();
	virtual ~BaseEnemy();

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void NormalMovement() {}
	virtual void Attack() {}

	void ChasingPlayer(float speed, float chaseRange);


	bool IsUsed() const { return use; }
	void SetUsed(bool b) { use = b; }

	void SetPosition(const XMFLOAT3& p);
	XMFLOAT3 GetPosition() const;

	void SetScale(const XMFLOAT3& s);
	XMFLOAT3 GetScale() const;

	void PhysicsStepAndResolve();
	void ZeroXZVelocity();
	virtual float FootOffset() const { return 0.0f; }

	virtual XMFLOAT3 GetColliderHalf() const = 0;


protected:
	
	XMFLOAT3 scl;
	XMFLOAT4X4 mtxWorld;
	bool use = false;
	float minDistance = 0.0f;
	float dropRate = 0.0f;
	int HP = 1, maxHP = 1;

};

//*****************************************************************************
// 
//*****************************************************************************
class SpiderEnemy : public BaseEnemy {
public:
	SpiderEnemy();
	~SpiderEnemy();

	void Init() override;
	void Update() override;
	void Draw() override;

	void NormalMovement() override;
	void Attack() override;
	XMFLOAT3 GetColliderHalf() const override {
		return XMFLOAT3(width * 0.5f, (height - 20.0f) * 0.5f, 50.0f * 0.5f);
	}
private:
	ID3D11ShaderResourceView* texture{ nullptr };
	struct MATERIAL* material{ nullptr };
	float width{ 100.0f }, height{ 100.0f };
	float speed{ 0.5f }, size{ 1.0f };

	XMFLOAT3 moveDir{ 0,0,1 };
	float moveChangeTimer{ 2.0f };

	int currentFrame{ 0 };
	int frameCounter{ 0 };
	int frameInterval{ 15 };
	int maxFrames{ 3 };

	float time{ 0.0f };
	int tblNo{ 0 };
	int tblMax{ 0 };

	float fireTimer{ 0.0f };
	const float fireCooldown{ 1.0f };
	float attackCooldownTimer{ 0.0f };
	float attackCooldown{ 1.5f };
	bool  isAttacking{ false };
	float attackFrameTimer{ 0.0f };

	


	//===================== A* 自動尋路用 =====================
	std::vector<XMFLOAT3> pathPoints;  // 経路点リスト
	int currentPathIndex = 0;          // 今向かっている目標点のインデックス
	float pathUpdateTimer = 0.0f;      // 再計算タイマー
	const float pathUpdateInterval = 2.0f;  
	//========================================================


};


class GhostEnemy : public BaseEnemy {
public:
	GhostEnemy();
	~GhostEnemy();

	void Init() override;
	void Update() override;
	void Draw() override;

	void NormalMovement() override;
	void Attack() override;
	XMFLOAT3 GetColliderHalf() const override {
		return XMFLOAT3(width * 0.5f, height * 0.5f, 50.0f * 0.5f);
	}

private:
	ID3D11ShaderResourceView* texture{ nullptr };
	struct MATERIAL* material{ nullptr };
	float width{ 100.0f }, height{ 100.0f };

	XMFLOAT3 moveDir{ 0,0,1 };
	float moveChangeTimer{ 2.0f };
	float speed{ 0.5f };

	int currentFrame{ 0 };
	int frameCounter{ 0 };
	int frameInterval{ 15 };
	int maxFrames{ 2 };

	float fireTimer{ 0.0f };
	const float fireCooldown{ 1.0f };
	float attackCooldownTimer{ 0.0f };
	float attackCooldown{ 1.5f };
	bool  isAttacking{ false };
	float attackFrameTimer{ 0.0f };



};

enum ENEMY_TYPE
{
	SPIDER,
	GHOST,

	MAX
};



//*****************************************************************************
// 
//*****************************************************************************
std::vector<BaseEnemy*>& GetEnemies();
HRESULT MakeVertexEnemy();
void InitEnemy();
void UpdateEnemy();
void DrawEnemy();
void UninitEnemy();

void EnemySpawner(XMFLOAT3 position, int type);
void DropItems(const XMFLOAT3& pos, ENEMY_TYPE enemyType);
