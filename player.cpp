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
#include "wallandfloor.h"
#include "collision.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/cone.obj"			// �ǂݍ��ރ��f����


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






//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{
	g_Player.load = TRUE;
	LoadModel(MODEL_PLAYER, &g_Player.model);

	g_Player.pos = XMFLOAT3(-10.0f, PLAYER_OFFSET_Y+100.0f, -50.0f);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_Player.alive = TRUE;			// TRUE:�����Ă�
	g_Player.size = PLAYER_SIZE;	// �����蔻��̑傫��

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

	XMFLOAT3 nextPos = g_Player.pos;

	if (g_Player.alive)
	{
		g_Player.spd *= 0.7f;

		// �ړ�����
		if (GetKeyboardPress(DIK_W)) {
			nextPos.x += sinf(cam->rot.y) * VALUE_MOVE;
			nextPos.z += cosf(cam->rot.y) * VALUE_MOVE;
		}
		if (GetKeyboardPress(DIK_S)) {
			nextPos.x -= sinf(cam->rot.y) * VALUE_MOVE;
			nextPos.z -= cosf(cam->rot.y) * VALUE_MOVE;
		}
		if (GetKeyboardPress(DIK_A)) {
			nextPos.x -= cosf(cam->rot.y) * VALUE_MOVE;
			nextPos.z += sinf(cam->rot.y) * VALUE_MOVE;
		}
		if (GetKeyboardPress(DIK_D)) {

			nextPos.x += cosf(cam->rot.y) * VALUE_MOVE;
			nextPos.z -= sinf(cam->rot.y) * VALUE_MOVE;
		}
		//������������������
		//g_Player.rot.y = cam->rot.y + 3.14f;


		//Jump
		if (GetKeyboardTrigger(DIK_SPACE) && g_Player.isGround)
		{
			g_Player.verticalSpeed = g_Player.jumpPower;
			g_Player.isGround = FALSE;
		}

		//�d��
		g_Player.verticalSpeed -= gravity;
		if (g_Player.verticalSpeed < (g_Player.maxFallSpeed * -1.0f))
		{
			g_Player.verticalSpeed = g_Player.maxFallSpeed * -1.0f;
		}

		//�n��
		nextPos.y += g_Player.verticalSpeed;
		if (g_Player.pos.y < 0)
		{
			g_Player.pos.y = 0;
			g_Player.isGround = TRUE;
		}


		//�Ȃ񂿂����Bullet����
		if ( IsMouseLeftTriggered())
		{
			
			XMFLOAT3 pos = cam->pos; 

			XMFLOAT3 direction;
			direction = cam->rot;
			direction.y += 3.14f;
			// 
			SetBullet(pos, direction);
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


	//�Ǐ�����
	bool isHit = false;



	WALLANDFLOOR* walls = GetWallAndFloor();
	XMFLOAT3 from = g_Player.pos;
	XMFLOAT3 to = nextPos;


	for (int i = 0; i < MAX_WALLANDFLOOR; i++)
	{
		if (!walls[i].use) continue;
		VERTEX_3D* verts = walls[i].model.VertexArrayCPU;
		unsigned short* indices = walls[i].model.IndexArrayCPU;
		unsigned int numIndices = walls[i].model.IndexNumCPU;
		XMMATRIX world = XMLoadFloat4x4(&walls[i].mtxWorld);

		for (unsigned int j = 0; j < numIndices; j += 3)
		{
			XMFLOAT3 p0, p1, p2;
			XMStoreFloat3(&p0, XMVector3TransformCoord(XMLoadFloat3(&verts[indices[j]].Position), world));
			XMStoreFloat3(&p1, XMVector3TransformCoord(XMLoadFloat3(&verts[indices[j + 1]].Position), world));
			XMStoreFloat3(&p2, XMVector3TransformCoord(XMLoadFloat3(&verts[indices[j + 2]].Position), world));

			//wallhit
			{
				XMVECTOR v0 = XMLoadFloat3(&p0);
				XMVECTOR v1 = XMLoadFloat3(&p1);
				XMVECTOR v2 = XMLoadFloat3(&p2);
				XMVECTOR edge1 = v1 - v0;
				XMVECTOR edge2 = v2 - v0;
				XMVECTOR faceNormal = XMVector3Normalize(XMVector3Cross(edge1, edge2));

				XMVECTOR rayDir = XMVector3Normalize(XMLoadFloat3(&to) - XMLoadFloat3(&from));
				
				bool isWall = true;
				if (fabs(XMVectorGetY(faceNormal)) > 0.7f)
				{
					isWall = false;
				};

				float dot = XMVectorGetX(XMVector3Dot(faceNormal, rayDir));
				if (dot > 0.0f) { isWall = false; };

				if (isWall)
				{
					XMFLOAT3 hit, normal;
					if (RayCast(p0, p1, p2, from, to, &hit, &normal))
					{
						g_Player.pos.x += sinf(g_Player.rot.y) * g_Player.spd;
						g_Player.pos.z += cosf(g_Player.rot.y) * g_Player.spd;

						isHit = true;
						break;
					}
				}
			}
		}



	}

	if (!isHit)
	{
		g_Player.pos = nextPos;
	}
	// ���C�L���X�g���đ����̍��������߂�
	XMFLOAT3 HitPosition;		// ��_
	XMFLOAT3 Normal;			// �Ԃ������|���S���̖@���x�N�g���i�����j
	BOOL ans = RayHitWallAndFloor(g_Player.pos, &HitPosition, &Normal);
	if (ans)
	{
		g_Player.pos.y = HitPosition.y + PLAYER_OFFSET_Y;
	}
	else
	{
		Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}
	

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

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);


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

