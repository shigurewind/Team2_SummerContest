#pragma once

#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

struct NavTriangle {
	XMFLOAT3 v0, v1, v2;
	XMFLOAT3 normal;
};

// �͌^����NavMesh���������iworldMatrix�̓��f���̃��[���h�ϊ��s��j
void InitNavMeshFromModel(struct AMODEL* model, const XMMATRIX& worldMatrix);

// �w�肵��XZ���W�ɂ�����NavMesh�̍������擾�iY���W�j
// �߂�l: ������ true�A���s�� false�BoutY��Y�l���i�[
bool GetNavMeshHeight(float x, float z, float* outY = nullptr);

// �w�肵��XZ���W��NavMesh���ɂ��邩�ǂ���
bool IsPointOnNavMesh(float x, float z);