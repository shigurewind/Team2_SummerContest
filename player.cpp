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
PLAYER	g_Player;						// �v���C���[

//static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// �v���C���[�̃p�[�c�p

static float		roty = 0.0f;

static LIGHT		g_Light;

//�d��
//static float gravity = 0.5f;
//�ߐڍU���N�[���_�E��
static float meleeCooldown = 0.0f;
//�`���[�g���A������p
static bool tutorialTriggered = false;


//wepon��bullet�e�̏��
static WeaponType currentWeapon = WEAPON_REVOLVER;
static BulletType currentBullet = BULLET_NORMAL;








int Min(int a, int b) {
	return (a < b) ? a : b;
}

//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{

	g_Player.Init();





	return S_OK;
}

void PLAYER::Init()
{

	// ��{������
	pos = { 0, PLAYER_OFFSET_Y + 50.0f, 0 };
	rot = { 0, 0, 0 };
	scl = { 1, 1, 1 };
	velocity = { 0, 0, 0 };
	speed = 2.0f;
	size = PLAYER_SIZE;

	EnableGravity(true);
	SetMaxFallSpeed(6.0f);
	jumpPower = 8.0f;

	ammoNormal = 0;
	maxAmmoNormal = 30;
	ammoFire = 0;
	maxAmmoFire = 20;

	HP = HP_MAX = 5;
	alive = true;

	meleeCDTime = 0.8f;

	currentWeapon = WEAPON_REVOLVER;
	currentBullet = BULLET_NORMAL;

	load = TRUE;
	LoadModel(MODEL_PLAYER, &model);

	// �e
	XMFLOAT3 shadowPos = pos;
	shadowPos.y -= (PLAYER_OFFSET_Y - 0.1f);
	shadowIdx = CreateShadow(shadowPos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);

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


	if (meleeCooldown > 0.0f) {
		meleeCooldown -= 1.0f / 60.0f;
	}

	if (g_Player.alive)
	{


		/*g_Player.HandleInput();
		g_Player.HandleShooting();
		g_Player.HandleReload();
		g_Player.HandleJump();
		g_Player.HandleGroundCheck();*/

		g_Player.OnUpdate(); // �v���C���[�̍X�V����

		//g_Player.EventCheck(); // �C�x���g�`�F�b�N






		//HP����test
		if (GetKeyboardTrigger(DIK_H))
		{
			g_Player.HP = g_Player.HP - 1;
		}

	}



#ifdef _DEBUG

#endif









	// �e���v���C���[�̈ʒu�ɍ��킹��
	XMFLOAT3 pos = g_Player.GetPosition();
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);





	// �|�C���g���C�g�̃e�X�g
	{
		LIGHT* light = GetLightData(1);
		XMFLOAT3 pos = g_Player.GetPosition();
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
	//PrintDebugProc("Player X:%f Y:%f Z:%f \n\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);

	PrintDebugProc("R�L�[�Ń����[�h\n"
		"1�L�[�ŕ���؂�ւ�\n"
		"2�L�[�Œe�؂�ւ�");
#endif

}


void PLAYER::OnUpdate() {
	HandleInput();          // W/A/S/D�ړ� & ��������
	HandleJump();           // �X�y�[�X�L�[����

	ApplyCollision();      // �Փ˔���ƓK�p
	Object::Update();
	HandleGroundCheck();    // �n�ʐڒn����

	HandleShooting();       // �e����
	HandleReload();         // R�Ń����[�h

	EventCheck();          // �C�x���g�`�F�b�N
}

//�W�����v
void PLAYER::HandleJump() {
	if (GetKeyboardTrigger(DIK_SPACE) && isGround) {
		velocity.y = jumpPower;
		isGround = false;
	}
}

//�ړ�����
void PLAYER::HandleInput()
{
	//�ړ�����TODO�F�ύX�K�v
	CAMERA* cam = GetCamera();

	//g_Player.speed *= 0.7f;

	// �ړ�����
	XMFLOAT3 move = {};
	//bool isMoving = false;

	if (GetKeyboardPress(DIK_W)) {
		move.x += sinf(cam->rot.y);
		move.z += cosf(cam->rot.y);
		//isMoving = true;
	}
	if (GetKeyboardPress(DIK_S)) {
		move.x -= sinf(cam->rot.y);
		move.z -= cosf(cam->rot.y);
		//isMoving = true;
	}
	if (GetKeyboardPress(DIK_A)) {
		move.x -= cosf(cam->rot.y);
		move.z += sinf(cam->rot.y);
		//isMoving = true;
	}
	if (GetKeyboardPress(DIK_D)) {
		move.x += cosf(cam->rot.y);
		move.z -= sinf(cam->rot.y);
		//isMoving = true;
	}
	
	velocity.x = move.x * speed;
	velocity.z = move.z * speed;

	

	//�ߐڍU��
	if (IsMouseRightTriggered() && meleeCooldown <= 0.0f)
	{
		meleeCooldown = meleeCDTime;
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


	//����؂�ւ�
	//�L�[�{�[�h��1�@����̐؂�ւ�
	if (GetKeyboardTrigger(DIK_1))
	{
		currentWeapon = (currentWeapon == WEAPON_REVOLVER) ? WEAPON_SHOTGUN : WEAPON_REVOLVER;
	}
	//�L�[�{�[�h��2�@�e�̐؂�ւ�
	if (GetKeyboardTrigger(DIK_2))
	{
		currentBullet = (currentBullet == BULLET_NORMAL) ? BULLET_FIRE : BULLET_NORMAL;
	}


}


void PLAYER::ApplyCollision()
{
	//���̈ʒu��\��
	XMFLOAT3 nextPos = pos;
	nextPos.x += velocity.x;
	nextPos.z += velocity.z;

	//BOX�̌v�Z
	float halfSize = size;
	XMFLOAT3 min = { nextPos.x - halfSize, pos.y - 0.1f, nextPos.z - halfSize };
	XMFLOAT3 max = { nextPos.x + halfSize, pos.y + 0.1f, nextPos.z + halfSize };

	if (AABBHitOctree(GetWallTree(), GetWallTriangles(), min, max, 0, 5, 5))
	{
		//Slide����
		XMFLOAT3 originalVelocity = { velocity.x, 0.0f, velocity.z };

		//�ǂ̖@��
		XMFLOAT3 wallNormal = GetWallCollisionNormal(pos, originalVelocity, halfSize);

		if (wallNormal.x != 0.0f || wallNormal.z != 0.0f)
		{
			// �ǂɉ����ăX���C�h
			//velocity-(velocity�Enormal)*normal
			XMVECTOR vel = XMLoadFloat3(&originalVelocity);
			XMVECTOR normal = XMLoadFloat3(&wallNormal);

			float dotProduct = XMVectorGetX(XMVector3Dot(vel, normal));
			XMVECTOR slide = XMVectorSubtract(vel, XMVectorScale(normal, dotProduct));

			XMFLOAT3 slideVelocity;
			XMStoreFloat3(&slideVelocity, slide);

			// �X���C�h��̈ʒu���v�Z
			XMFLOAT3 testPos = pos;
			testPos.x += slideVelocity.x;
			testPos.z += slideVelocity.z;

			XMFLOAT3 testMin = { testPos.x - halfSize, pos.y - 0.1f, testPos.z - halfSize };
			XMFLOAT3 testMax = { testPos.x + halfSize, pos.y + 0.1f, testPos.z + halfSize };

			if (!AABBHitOctree(GetWallTree(), GetWallTriangles(), testMin, testMax, 0, 5, 5))
			{
				//�X���C�h���p
				velocity.x = slideVelocity.x;
				velocity.z = slideVelocity.z;
			}
			else
			{
				//�~�܂�
				velocity.x = 0;
				velocity.z = 0;
			}

		}
		else
		{
			//�ǂɂԂ�������~�܂�
			velocity.x = 0;
			velocity.z = 0;
		}
		

	}
}

//�ڒn����
void PLAYER::HandleGroundCheck()
{


	const float groundThreshold = 0.2f;
	float groundY;
	if (CheckPlayerGroundSimple(pos, PLAYER_OFFSET_Y, groundY) && GetVelocity().y <= 0.0f)
	{
		float targetY = groundY;
		float distanceToGround = pos.y - targetY;
		if (distanceToGround <= groundThreshold)
		{
			pos.y = targetY;
			SetVelocity(XMFLOAT3(GetVelocity().x, 0.0f, GetVelocity().z));
			isGround = TRUE;
		}
		else
		{
			isGround = FALSE;
		}
	}
	else
	{
		isGround = FALSE;
	}


	

}

void PLAYER::EventCheck()
{
	//����̒n�����ƃQ�[�����~
	if (!tutorialTriggered &&
		pos.x > 50.0f && pos.x < 100.0f &&
		pos.z > 50.0f && pos.z < 100.0f)
	{
		SetTutorialShowing(true);
		tutorialTriggered = true;
	}

}


void PLAYER::HandleShooting()
{
	// �e���ˏ���
	int* currentAmmo = (currentBullet == BULLET_NORMAL) ? &ammoNormal : &ammoFire;
	if (IsMouseLeftTriggered() && currentAmmo > 0)
	{
		XMFLOAT3 pos = GetGunMuzzlePosition();
		XMFLOAT3 rot = GetGunMuzzleRotation();
		if (currentWeapon == WEAPON_REVOLVER)
		{
			SetRevolverBullet(currentBullet, pos, rot);
		}
		else {
			SetShotgunBullet(currentBullet, pos, rot);
		}
		(currentAmmo)--;
	}
}


void PLAYER::HandleReload()
{
	// R�L�[�Ń����[�h����
	if (GetKeyboardTrigger(DIK_R))
	{
		Weapon* weapon = (currentWeapon == WEAPON_REVOLVER) ? GetRevolver() : GetShotgun();
		int clipSize = weapon->clipSize;

		int* ammo = (currentBullet == BULLET_NORMAL) ? &ammoNormal : &ammoFire;
		int* maxAmmo = (currentBullet == BULLET_NORMAL) ? &maxAmmoNormal : &maxAmmoFire;

		if (*ammo < clipSize && *maxAmmo > 0)
		{
			int need = clipSize - *ammo;
			int reload = Min(need, *maxAmmo);
			*ammo += reload;
			*maxAmmo -= reload;
		}
	}
}


XMFLOAT3 PLAYER::GetWallCollisionNormal(XMFLOAT3 currentPos, XMFLOAT3 moveVector, float halfSize)
{
	//ray�̊J�n�ʒu
	XMFLOAT3 rayStart = currentPos;
	rayStart.y += 1.0f;

	XMFLOAT3 rayEnd = rayStart;
	rayEnd.x += moveVector.x * 2.0f;
	rayEnd.z += moveVector.z * 2.0f;

	XMFLOAT3 rayDir = { rayEnd.x - rayStart.x, rayEnd.y - rayStart.y, rayEnd.z - rayStart.z };

	//��ԋ߂��̕�
	float closestDist = 100.0f;
	XMFLOAT3 hitPos, hitNormal = { 0.0f, 0.0f, 0.0f };

	if (RayHitOctree(GetWallTree(), GetWallTriangles(), rayStart, rayDir, &closestDist, &hitPos, &hitNormal, 0, 5, 5))
	{
		return hitNormal;
	}

	return { 0.0f, 0.0f, 0.0f };
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
	mtxTranslate = XMMatrixTranslation(g_Player.GetPosition().x, g_Player.GetPosition().y, g_Player.GetPosition().z);
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