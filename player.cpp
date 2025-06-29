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
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"
#include "FBXmodel.h"
#include "Octree.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/cone.obj"			// �ǂݍ��ރ��f����


#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// ��]��

#define PLAYER_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define PLAYER_OFFSET_Y		(7.0f*20.0f)							// �v���C���[�̑��������킹��

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


// �v���C���[�̊K�w�A�j���[�V�����f�[�^


// �v���C���[�̓������E�ɓ������Ă���A�j���f�[�^
static INTERPOLATION_DATA move_tbl_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};


static INTERPOLATION_DATA move_tbl_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(XM_PI/2, 0.0f, 0.0f),   XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },

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

	//FBXTEST
	//LoadFBXModel("data/MODEL/model.fbx", &g_Player.model);


	g_Player.pos = XMFLOAT3(-15.0f, PLAYER_OFFSET_Y+50.0f, -100.0f);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_Player.alive = TRUE;			// TRUE:�����Ă�
	g_Player.size = PLAYER_SIZE;	// �����蔻��̑傫��

	g_Player.ammo = 5;				//�����[�h�ł���e��
	g_Player.maxammo = 20;			//�����Ă�e��

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
	CAMERA *cam = GetCamera();


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
		if (GetKeyboardPress(DIK_D)) {
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
			wallBoxMin.y -= PLAYER_OFFSET_Y;
			wallBoxMin.z -= halfSize;

			wallBoxMax.x += halfSize;
			wallBoxMax.y += PLAYER_OFFSET_Y;
			wallBoxMax.z += halfSize;

			if (!AABBHitOctree(GetWallTree(), wallBoxMin, wallBoxMax)) {
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
		OctreeNode* floorTree = GetFloorTree();
		if (floorTree == nullptr) {
			OutputDebugStringA("cant find flooroctree\n");
			g_Player.pos.y = PLAYER_OFFSET_Y;
			g_Player.isGround = TRUE;
			g_Player.verticalSpeed = 0.0f;
			return;

		}
		XMFLOAT3 from = g_Player.pos;
		from.y += 1.0f;  
		XMFLOAT3 to = g_Player.pos;
		to.y -= 5.0f;    

		XMFLOAT3 dir = {
			to.x - from.x,
			to.y - from.y,
			to.z - from.z
		};

		XMFLOAT3 hitPos, hitNormal;
		float dist = 10.0f;

		if (RayHitOctree(GetFloorTree(), from, dir, &dist, &hitPos, &hitNormal)) {
			if (g_Player.verticalSpeed <= 0.0f && hitPos.y <= g_Player.pos.y) {
				newPos.y = hitPos.y + PLAYER_OFFSET_Y;
				g_Player.verticalSpeed = 0.0f;
				g_Player.isGround = TRUE;
			}
			else {
				g_Player.isGround = FALSE;
			}
		}
		else {
			g_Player.isGround = FALSE;

			if (newPos.y < -100.0f) {
				newPos = XMFLOAT3(-15.0f, PLAYER_OFFSET_Y + 50.0f, -100.0f);
				g_Player.verticalSpeed = 0.0f;
				g_Player.isGround = TRUE;
			}
		}

		{	// �����������Ƀv���C���[���ړ�������
		// �����������Ƀv���C���[���������Ă��鏊
			g_Player.rot.y = roty + cam->rot.y;

			newPos.x -= sinf(g_Player.rot.y) * g_Player.spd;
			newPos.z -= cosf(g_Player.rot.y) * g_Player.spd;
		}

		g_Player.pos = newPos;
	

		
		// �e���ˏ����i���ʊ֐��g�p�j 
		if (IsMouseLeftTriggered() && g_Player.ammo > 0)
		{
			XMFLOAT3 pos = isFirstPersonMode ? GetGunMuzzlePosition() : g_Player.pos;  
			XMFLOAT3 rot = isFirstPersonMode ? GetGunMuzzleRotation() : g_Player.rot;  
			/*SetRevolverBullet(pos, rot);*/
			SetShotgunBullet(pos, rot, *GetShotgun()->bulletData);
			g_Player.ammo--;
		}
		// R�L�[�Ń����[�h����
		if (GetKeyboardTrigger(DIK_R))
		{
			// �e���s�����Ă��āA���莝���ɒe������ꍇ�̂݃����[�h
			if (g_Player.ammo < 5 && g_Player.maxammo > 0)
			{

				int need = 5 - g_Player.ammo;
				int reload = Min(need, g_Player.maxammo);
				g_Player.ammo += reload;
				g_Player.maxammo -= reload;
			}
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
		LIGHT *light = GetLightData(1);
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
	PrintDebugProc("Player X:%f Y:%f Z:%f \n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	PrintDebugProc("Player Ground:%s VertSpeed:%f\n", g_Player.isGround ? "TRUE" : "FALSE", g_Player.verticalSpeed);
	PrintDebugProc("FloorTree:%p\n", GetFloorTree());
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
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

