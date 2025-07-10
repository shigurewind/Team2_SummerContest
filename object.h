#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Object {
public:
	Object();
	virtual ~Object() {}

	virtual void Update(); // 毎フレーム位置と重力更新

	void SetPosition(const XMFLOAT3& p);
	XMFLOAT3 GetPosition() const;

	void SetVelocity(const XMFLOAT3& v);
	XMFLOAT3 GetVelocity() const;

	void AddForce(const XMFLOAT3& f);
	void EnableGravity(bool b) { useGravity = b; }
	void SetMaxFallSpeed(float s) { maxFallSpeed = s; }

protected:
	XMFLOAT3 pos;
	XMFLOAT3 velocity;
	bool isGround;

	float gravity;
	float maxFallSpeed;
	bool useGravity;
};