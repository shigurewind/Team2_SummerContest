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



//*****************************************************************************
// �\���̒�`
//*****************************************************************************
//struct PLAYER
//{
//	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
//	XMFLOAT3			pos;				// ���f���̈ʒu
//	XMFLOAT3			rot;				// ���f���̌���(��])
//	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)
//
//	float				spd;				// �ړ��X�s�[�h
//	
//	BOOL				load;
//	DX11_MODEL			model;				// ���f�����
//
//	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
//
//	BOOL				alive;
//
//	float				size;
//
//	// �K�w�A�j���[�V�����p�̃����o�[�ϐ�
//	float				time;				// ���`��ԗp
//	int					tblNo;				// �s���f�[�^�̃e�[�u���ԍ�
//	int					tblMax;				// ���̃e�[�u���̃f�[�^��
//
//	// �e�́ANULL�A�q���͐e�̃A�h���X������
//	PLAYER				*parent;			// �������e�Ȃ�NULL�A�������q���Ȃ�e��player�A�h���X
//
//	// �N�H�[�^�j�I��
//	XMFLOAT4			Quaternion;
//
//	XMFLOAT3			UpVector;			// �����������Ă��鏊
//
//
//	BOOL			isGround;	//�n�ʃ`�F�b�N
//	float			verticalSpeed;	//���Ƃ�Speed
//	float			maxFallSpeed;//�ő嗎�Ƃ�Speed
//	float			jumpPower;	//jump�̃p���[
//
//
//	// �e���Ǘ��i��ނ��Ɓj
//	int ammoNormal;
//	int maxAmmoNormal;
//
//	int ammoFire;
//	int maxAmmoFire;
//	float				HP;
//	float				HP_MAX;
//};



class PLAYER : public Object
{
public:
	void Init();
	void Update();
	void Draw();

	void HandleInput();
	void HandleShooting();
	void HandleReload();
	void HandleJump();
	void HandleGroundCheck();

	void EventCheck();



	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			newPos;				// ���f���̎��̃t���[���\���ʒu
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


//extern PLAYER g_Player;


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
