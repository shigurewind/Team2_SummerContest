#pragma once
#include "model.h"

class BaseEnemy {
public:
	BaseEnemy();
	virtual ~BaseEnemy();

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	bool IsUsed() const { return use; }
	void SetUsed(bool b) { use = b; }

protected:
	XMFLOAT3 pos;
	XMFLOAT3 scl;
	XMFLOAT4X4 mtxWorld;
	bool use;

};

class ScarecrowEnemy : public BaseEnemy {
public:
	ScarecrowEnemy();
	~ScarecrowEnemy();

	BOOL				use;
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float				spd;				// 移動スピード
	float				size;				// 当たり判定の大きさ
	int					shadowIdx;			// 影のインデックス番号

	float				time;				// 線形補間用
	int					tblNo;				// 行動データのテーブル番号
	int					tblMax;				// そのテーブルのデータ数

	int					moveCounter;		// 向き変わるタイマー

	//エネミーが発射するとき
	float				fireCooldown;		//
	float				fireTimer;			//
};

private:
	int texID;  // ??ID
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY *GetEnemy(void);

void ChangeEnemyDirection(int i);
void ChasingPlayer(int i);
void GhostMovement(int i);
void SkeletonMovement(int i);
void SpiderMovement(int i);


