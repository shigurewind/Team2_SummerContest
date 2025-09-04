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

	ID3D11ShaderResourceView* dissolveTexture;// ディゾルブ用テクスチャ
	static ID3D11ShaderResourceView* s_BloodTexture;// 血痕テクスチャ

	static bool LoadBloodTexture();    
	static void UnloadBloodTexture();
	

protected:
	XMFLOAT3 pos;
	XMFLOAT3 scl;
	XMFLOAT4X4 mtxWorld;
	bool use;

	float minDistance;
	float dropRate;
	int HP, maxHP;

	//ディゾルブ関連
	bool isDying;
	float dissolveTimer;
	float dissolveAmount;   // dissolve程度
	bool hasDroppedItems;


	
};


enum ENEMY_TYPE
{
	SPIDER,
	GHOST,
	BUG,

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

extern ID3D11Buffer* g_VertexBufferEnemy;
