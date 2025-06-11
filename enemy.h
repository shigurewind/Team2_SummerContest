#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

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

	bool IsUsed() const { return use; }
	void SetUsed(bool b) { use = b; }

	void SetPosition(const XMFLOAT3& p);
	XMFLOAT3 GetPosition() const;

	void SetScale(const XMFLOAT3& s);
	XMFLOAT3 GetScale() const;

protected:
	XMFLOAT3 pos;
	XMFLOAT3 scl;
	XMFLOAT4X4 mtxWorld;
	bool use;
};

//*****************************************************************************
// 
//*****************************************************************************
class ScarecrowEnemy : public BaseEnemy {
public:
	ScarecrowEnemy();
	~ScarecrowEnemy();

	void Init() override;
	void Update() override;
	void Draw() override;

private:
	ID3D11ShaderResourceView* texture;
	struct MATERIAL* material;
	float width, height;
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
