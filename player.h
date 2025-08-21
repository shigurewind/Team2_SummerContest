//=============================================================================
//
// ���f������ [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"
#include "bullet.h"
#include "object.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_PLAYER		(1)					// �v���C���[�̐�

#define	PLAYER_SIZE		(5.0f)				// �����蔻��̑傫��





class PLAYER : public Object
{
public:
	void Init();
	void OnUpdate();
	//void Draw();

	void HandleInput();//�ړ��Ƒ���Input����
	void ApplyCollision();//�}�b�v�����蔻�菈��
	void HandleGroundCheck();//�n�ʃ`�F�b�N

	void HandleShooting();
	void HandleReload();
	void HandleJump();

	void EventCheck();

	//�ǂ̃m�[�}���擾�iSlide�@�\���߂Ɂj
	XMFLOAT3 GetWallCollisionNormal(XMFLOAT3 currentPos, XMFLOAT3 moveVector, float halfSize);



	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	BOOL				load;
	DX11_MODEL			model;				// ���f�����

	// �N�H�[�^�j�I��
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// �����������Ă��鏊


	float HP, HP_MAX;
	int ammoNormal, maxAmmoNormal;
	int ammoFire, maxAmmoFire;

	//�ړ��֘A
	float			size;				// �����蔻��̑傫��
	float			speed;				// �ړ��X�s�[�h
	float			jumpPower;	//jump�̃p���[

	//�U��
	float meleeCDTime; // �ߐڍU���̃N�[���_�E������


	int shadowIdx;
	bool alive;

	//����֘A
	WeaponType currentWeapon;
	BulletType currentBullet;
};





//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

WeaponType GetCurrentWeaponType(void);
BulletType GetCurrentBulletType(void);


PLAYER* GetPlayer(void);
bool CheckPlayerGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY);
