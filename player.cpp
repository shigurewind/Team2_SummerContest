//=============================================================================
//
// ���f������ [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "player.h"
#include "shadow.h"
#include "debugproc.h"
#include "meshfield.h"
#include "FBXmodel.h"
#include "Octree.h"
#include "collision.h"
#include "overlay2D.h"
#include "enemy.h"

//*****************************************************************************
// �}�N����`	
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/player.obj"			// �ǂݍ��ރ��f����


#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// ��]��

#define PLAYER_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define PLAYER_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��

#define PLAYER_PARTS_MAX	(2)								// �v���C���[�̃p�[�c�̐�



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static PLAYER		g_Player;						// �v���C���[

//static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// �v���C���[�̃p�[�c�p

static float		roty = 0.0f;

static LIGHT		g_Light;

//�d��
static float gravity = 0.5f;
//�ߐڍU���N�[���_�E��
static float meleeCooldown = 0.0f;
//�`���[�g���A������p
static bool tutorialTriggered = false;


//wepon��bullet�e�̏��
static WeaponType currentWeapon = WEAPON_REVOLVER;
static BulletType currentBullet = BULLET_NORMAL;


// �v���C���[�̊K�w�A�j���[�V�����f�[�^


// �v���C���[�̓������E�ɓ������Ă���A�j���f�[�^
static INTERPOLATION_DATA move_tbl_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};


static INTERPOLATION_DATA move_tbl_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, 0.0f),   XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },

};


static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	move_tbl_left,
	move_tbl_right,

};


int Min(int a, int b) {
	return (a < b) ? a : b;
}

//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{
	g_Player.load = TRUE;
	LoadModel(MODEL_PLAYER, &g_Player.model);

	


	g_Player.pos = XMFLOAT3(0.0f, PLAYER_OFFSET_Y+50.0f, 0.0f);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_Player.alive = TRUE;			// TRUE:�����Ă�
	g_Player.size = PLAYER_SIZE;	// �����蔻��̑傫��

	g_Player.ammoNormal    = 0;		//�ŏ��ɑ��U����Ă�e��
	g_Player.maxAmmoNormal = 30;	//�������Ă�e���S��

	g_Player.ammoFire      = 0;		//�ŏ��ɑ��U����Ă�e��
	g_Player.maxAmmoFire   = 20;	//�������Ă�e���S��

	// �����Ńv���C���[�p�̉e���쐬���Ă���
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Player.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ��
	//        ���̃����o�[�ϐ������������e��Index�ԍ�

	// �L�[�����������̃v���C���[�̌���
	roty = 0.0f;




	g_Player.isGround = FALSE;
	g_Player.maxFallSpeed = 6.0f;
	g_Player.jumpPower = 8.0f;

	g_Player.HP = g_Player.HP_MAX = 5;






	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
{
	// ���f���̉������
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}





}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePlayer(void)
{
	CAMERA* cam = GetCamera();

	if (meleeCooldown > 0.0f) {
		meleeCooldown -= 1.0f / 60.0f;  
	}

	if (g_Player.alive)
	{
		g_Player.spd *= 0.7f;

		// �ړ�����
		XMFLOAT3 move = {};
		bool isMoving = false;

		if (GetKeyboardPress(DIK_W)) {
			move.x += sinf(cam->rot.y);
			move.z += cosf(cam->rot.y);
			isMoving = true;
		}
		if (GetKeyboardPress(DIK_S)) {
			move.x -= sinf(cam->rot.y);
			move.z -= cosf(cam->rot.y);
			isMoving = true;
		}
		if (GetKeyboardPress(DIK_A)) {
			move.x -= cosf(cam->rot.y);
			move.z += sinf(cam->rot.y);
			isMoving = true;
		}
		if(GetKeyboardPress(DIK_D)) {
			move.x += cosf(cam->rot.y);
			move.z -= sinf(cam->rot.y);
			isMoving = true;
		}

		XMFLOAT3 newPos = g_Player.pos;
		if (isMoving) {
			XMVECTOR moveVec = XMVector3Normalize(XMLoadFloat3(&move));
			XMFLOAT3 testPos = g_Player.pos;
			testPos.x += XMVectorGetX(moveVec) * VALUE_MOVE;
			testPos.z += XMVectorGetZ(moveVec) * VALUE_MOVE;

			XMFLOAT3 wallBoxMin = testPos;
			XMFLOAT3 wallBoxMax = testPos;
			float halfSize = g_Player.size;

			wallBoxMin.x -= halfSize;
			wallBoxMin.y -= 0.1f;
			wallBoxMin.z -= halfSize;

			wallBoxMax.x += halfSize;
			wallBoxMax.y += 0.1f;
			wallBoxMax.z += halfSize;

			if (!AABBHitOctree(GetWallTree(), GetWallTriangles(), wallBoxMin, wallBoxMax, 0, 5, 5)) {
				newPos.x = testPos.x;
				newPos.z = testPos.z;
			}
		}

		//������������������
		//g_Player.rot.y = cam->rot.y + 3.14f;

		
		//Jump
		if (GetKeyboardTrigger(DIK_SPACE) && g_Player.isGround) {
			g_Player.verticalSpeed = g_Player.jumpPower;
			g_Player.isGround = FALSE;
		}

		//�d��
		if (!g_Player.isGround) {
			g_Player.verticalSpeed -= gravity;
			if (g_Player.verticalSpeed < -g_Player.maxFallSpeed) {
				g_Player.verticalSpeed = -g_Player.maxFallSpeed;
			}
		}


		newPos.y += g_Player.verticalSpeed;

		//�n��
		const float groundThreshold = 0.2f;
		float groundY;
		if (CheckPlayerGroundSimple(newPos, PLAYER_OFFSET_Y, groundY) && g_Player.verticalSpeed <= 0.0f)
		{
			float targetY = groundY;
			float distanceToGround = newPos.y - targetY;
			if (distanceToGround <= groundThreshold)
			{
				newPos.y = targetY;
				g_Player.verticalSpeed = 0.0f;
				g_Player.isGround = TRUE;
			}
			else
			{
				g_Player.isGround = FALSE;
			}
		}
		else
		{
			g_Player.isGround = FALSE;
		}
		g_Player.pos = newPos;
		


		//�ߐڍU��

		if (IsMouseRightTriggered() && meleeCooldown <= 0.0f)
		{
			meleeCooldown = 0.8f;
			PlayMeleeAnimation();
			//enemy 

			auto& enemies = GetEnemies();
			for (auto enemy : enemies) {
				if (!enemy->IsUsed()) continue;

				XMFLOAT3 ePos = enemy->GetPosition();
				float dx = g_Player.pos.x - ePos.x;
				float dz = g_Player.pos.z - ePos.z;
				float distance = sqrtf(dx * dx + dz * dz);

				if (distance > 100.0f) continue;

				enemy->SetUsed(false);  
			}
		}

		//����̒n�����ƃQ�[�����~
		if (!tutorialTriggered &&
			g_Player.pos.x > 50.0f && g_Player.pos.x < 100.0f &&
			g_Player.pos.z > 50.0f && g_Player.pos.z < 100.0f)
		{
			SetTutorialShowing(true);
			tutorialTriggered = true;
		}




		//�L�[�{�[�h��1�@����̐؂�ւ�
		if (GetKeyboardTrigger(DIK_1))
		{
			switch (currentWeapon)
			{
			case WEAPON_REVOLVER:
				currentWeapon = WEAPON_SHOTGUN;
				break;
			case WEAPON_SHOTGUN:
				currentWeapon = WEAPON_ROCKET_LAUNCHER;
				break;
			case WEAPON_ROCKET_LAUNCHER:
				currentWeapon = WEAPON_REVOLVER;
				break;
			}
		}
		//�L�[�{�[�h��2�@�e�̐؂�ւ�
		if (GetKeyboardTrigger(DIK_2))
		{
			if (currentBullet == BULLET_NORMAL)
			{
				currentBullet = BULLET_FIRE;
			}
			else
			{
				currentBullet = BULLET_NORMAL;
			}
		}
		PLAYER* p = GetPlayer();


		// �e���ˏ���
		int* currentAmmo = (currentBullet == BULLET_NORMAL) ? &p->ammoNormal : &p->ammoFire;
		if (IsMouseLeftTriggered() && *currentAmmo > 0)
		{
			XMFLOAT3 pos = GetGunMuzzlePosition();
			XMFLOAT3 rot = GetGunMuzzleRotation();
			if (currentWeapon == WEAPON_REVOLVER)
			{
				SetRevolverBullet(currentBullet, pos, rot);
			}
			else if(currentWeapon == WEAPON_SHOTGUN)
			{
				SetShotgunBullet(currentBullet, pos, rot);
			}
			else if (currentWeapon == WEAPON_ROCKET_LAUNCHER)
			{
				SetRocketLauncherBullet(currentBullet, pos, rot);
			}
			(*currentAmmo)--;
		}
		
		
		

		// R�L�[�Ń����[�h����
		if (GetKeyboardTrigger(DIK_R))
		{
			Weapon* weapon = nullptr;
			switch (currentWeapon)
			{
			case WEAPON_REVOLVER:
				weapon = GetRevolver();
				break;
			case WEAPON_SHOTGUN:
				weapon = GetShotgun();
				break;
			case WEAPON_ROCKET_LAUNCHER:
				weapon = GetRocket_Launcher();
				break;
			}

			int clipSize = weapon->clipSize;

			int* ammo = (currentBullet == BULLET_NORMAL) ? &g_Player.ammoNormal : &g_Player.ammoFire;
			int* maxAmmo = (currentBullet == BULLET_NORMAL) ? &g_Player.maxAmmoNormal : &g_Player.maxAmmoFire;

			if (*ammo < clipSize && *maxAmmo > 0)
			{
				int need = clipSize - *ammo;
				int reload = Min(need, *maxAmmo);
				*ammo += reload;
				*maxAmmo -= reload;
			}
		}

		//test
		if (GetKeyboardTrigger(DIK_H))
		{
			g_Player.HP = g_Player.HP - 1;
		}

	}



#ifdef _DEBUG
	/*if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.spd = 0.0f;
		roty = 0.0f;
	}*/
#endif


	{	// �����������Ƀv���C���[���ړ�������
		// �����������Ƀv���C���[���������Ă��鏊
		g_Player.rot.y = roty + cam->rot.y;

		g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;
	}



	// ���C�L���X�g���đ����̍��������߂�
	//XMFLOAT3 HitPosition;		// ��_
	//XMFLOAT3 Normal;			// �Ԃ������|���S���̖@���x�N�g���i�����j
	//BOOL ans = RayHitField(g_Player.pos, &HitPosition, &Normal);
	//if (ans)
	//{
	//	g_Player.pos.y = HitPosition.y + PLAYER_OFFSET_Y;
	//}
	//else
	//{
	//	g_Player.pos.y = PLAYER_OFFSET_Y;
	//	Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//}


	//// �e���ˏ���
	//if (GetKeyboardTrigger(DIK_SPACE))
	//{
	//	SetBullet(g_Player.pos, g_Player.rot);
	//}


	// �e���v���C���[�̈ʒu�ɍ��킹��
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);





	// �|�C���g���C�g�̃e�X�g
	{
		LIGHT* light = GetLightData(1);
		XMFLOAT3 pos = g_Player.pos;
		pos.y += 20.0f;

		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(1, light);
	}






#ifdef _DEBUG
	// �f�o�b�O�\��
	PrintDebugProc("Player X:%f Y:%f Z:%f \n\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	
	PrintDebugProc("R�L�[�Ń����[�h\n"
				   "1�L�[�ŕ���؂�ւ�\n"
				   "2�L�[�Œe�؂�ւ�");
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	XMMATRIX mtxFootOffset = XMMatrixTranslation(0.0f, -PLAYER_OFFSET_Y, 0.0f);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxFootOffset);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �N�H�[�^�j�I���𔽉f
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMFLOAT4X4 temp;
	DirectX::XMStoreFloat4x4(&temp, mtxWorld);
	g_Player.mtxWorld = temp;


	// �����̐ݒ�
	SetFuchi(1);

	// ���f���`��
	DrawModel(&g_Player.model);



	SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
PLAYER* GetPlayer(void)
{
	return &g_Player;
}

WeaponType GetCurrentWeaponType(void)
{
	return currentWeapon;
}

BulletType GetCurrentBulletType(void)
{
	return currentBullet;
}
bool CheckPlayerGroundSimple(XMFLOAT3 pos, float offsetY, float& groundY)
{
	const auto& tris = GetFloorTriangles();

	XMFLOAT3 rayStart = pos;
	rayStart.y += 50.0f;
	XMFLOAT3 rayEnd = pos;
	rayEnd.y -= 100.0f;

	XMFLOAT3 hit, normal;
	for (const auto& tri : tris)
	{
		if (RayCast(tri.v0, tri.v1, tri.v2, rayStart, rayEnd, &hit, &normal))
		{
			groundY = hit.y;
			return true;
		}
	}
	return false;
}