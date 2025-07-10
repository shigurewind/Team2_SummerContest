#include "object.h"

Object::Object()
	: pos({ 0, 0, 0 }),
	velocity({ 0, 0, 0 }),
	gravity(0.5f),
	maxFallSpeed(6.0f),
	isGround(false),
	useGravity(true)
{
}

void Object::Update() {
	if (useGravity && !isGround) {
		velocity.y -= gravity;
		if (velocity.y < -maxFallSpeed)
			velocity.y = -maxFallSpeed;
	}

	pos.x += velocity.x;
	pos.y += velocity.y;
	pos.z += velocity.z;
}

void Object::SetPosition(const XMFLOAT3& p) { pos = p; }
XMFLOAT3 Object::GetPosition() const { return pos; }


void Object::SetVelocity(const XMFLOAT3& v) { velocity = v; }
XMFLOAT3 Object::GetVelocity() const { return velocity; }


//直接にスピードをアップする
void Object::AddForce(const XMFLOAT3& f) {
	velocity.x += f.x;
	velocity.y += f.y;
	velocity.z += f.z;
}

