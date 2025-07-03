//=============================================================================
//
// enemy.h
// 
//
//=============================================================================

#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

using namespace DirectX;

//*****************************************************************************
// 
//*****************************************************************************

enum ENEMY_TYPE
{
	SPIDER,
	GHOST,

	MAX
};

//*****************************************************************************
// 
//*****************************************************************************
class BaseEnemy {
public:
	BaseEnemy();
	virtual ~BaseEnemy();

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void NormalMovement() {}
	virtual void Attack() {}

	void ChasingPlayer(float speed, float chaseRange);


	bool IsUsed() { return use; }
	void SetUsed(bool b) { use = b; }

	void SetPosition(const XMFLOAT3& p);
	XMFLOAT3 GetPosition() const;

	void SetScale(const XMFLOAT3& s);
	XMFLOAT3 GetScale() const;

	virtual ENEMY_TYPE GetType() const = 0;

protected:
	XMFLOAT3 pos;
	XMFLOAT3 scl;
	XMFLOAT4X4 mtxWorld;
	bool use;
	float minDistance;
	int HP, maxHP;

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
	ENEMY_TYPE GetType() const override { return SPIDER; }


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


class GhostEnemy : public BaseEnemy {
public:
	GhostEnemy();
	~GhostEnemy();

	void Init() override;
	void Update() override;
	void Draw() override;

	void NormalMovement() override;
	void Attack() override;
	ENEMY_TYPE GetType() const override { return GHOST; }



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
void SaveEnemyData(const std::string& filename);
void LoadEnemyData(const std::string& filename);
