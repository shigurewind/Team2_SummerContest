#pragma once
#include "renderer.h"

typedef struct {
	XMFLOAT3	pos;			// �ʒu
	XMFLOAT3	scl;			// �X�P�[��
	MATERIAL	material;		// �}�e���A��

	float		dissolve;		// �n��x
	float		dissolveSpeed;	// �n�𑬓x
	float		dissolveTime;	// �n������

	bool		isDissolving;	// �n�𒆃t���O

}DissolveTest;





DissolveTest* GetDissolveTest();

HRESULT InitDissolveTest();
void UninitDissolveTest();
void UpdateDissolveTest();
void DrawDissolveTest();

void StartDissolve();