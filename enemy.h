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

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	int texID;  // ??ID
};
void InitEnemy();
void UpdateEnemy();
void DrawEnemy();
void UninitEnemy();
