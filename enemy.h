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
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

	float				time;				// ���`��ԗp
	int					tblNo;				// �s���f�[�^�̃e�[�u���ԍ�
	int					tblMax;				// ���̃e�[�u���̃f�[�^��

	int					moveCounter;		// �����ς��^�C�}�[

	//�G�l�~�[�����˂���Ƃ�
	float				fireCooldown;		//
	float				fireTimer;			//
};

private:
	int texID;  // ??ID
};



//*****************************************************************************
// �v���g�^�C�v�錾
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


