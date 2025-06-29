//=============================================================================
//
// モデル処理 [player.cpp]
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
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/cone.obj"			// 読み込むモデル名


#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(7.0f*20.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(2)								// プレイヤーのパーツの数

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player;						// プレイヤー

//static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static float		roty = 0.0f;

static LIGHT		g_Light;

//重力
static float gravity = 0.5f;


// プレイヤーの階層アニメーションデータ


// プレイヤーの頭を左右に動かしているアニメデータ
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
// 初期化処理
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

	g_Player.spd = 0.0f;			// 移動スピードクリア

	g_Player.alive = TRUE;			// TRUE:生きてる
	g_Player.size = PLAYER_SIZE;	// 当たり判定の大きさ

	g_Player.ammo = 5;				//リロードできる弾数
	g_Player.maxammo = 20;			//持ってる弾数

	// ここでプレイヤー用の影を作成している
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Player.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ↑
	//        このメンバー変数が生成した影のIndex番号

	// キーを押した時のプレイヤーの向き
	roty = 0.0f;

	


	g_Player.isGround = FALSE;
	g_Player.maxFallSpeed = 6.0f;
	g_Player.jumpPower = 8.0f;
	



	



	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	// モデルの解放処理
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}

	



}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	CAMERA *cam = GetCamera();


	if (g_Player.alive)
	{
		g_Player.spd *= 0.7f;

		// 移動処理
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

		//正しい向き方向処理
		//g_Player.rot.y = cam->rot.y + 3.14f;

		
		//Jump
		if (GetKeyboardTrigger(DIK_SPACE) && g_Player.isGround) {
			g_Player.verticalSpeed = g_Player.jumpPower;
			g_Player.isGround = FALSE;
		}

		//重力
		if (!g_Player.isGround) {
			g_Player.verticalSpeed -= gravity;
			if (g_Player.verticalSpeed < -g_Player.maxFallSpeed) {
				g_Player.verticalSpeed = -g_Player.maxFallSpeed;
			}
		}

		newPos.y += g_Player.verticalSpeed;
		//地面
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

		{	// 押した方向にプレイヤーを移動させる
		// 押した方向にプレイヤーを向かせている所
			g_Player.rot.y = roty + cam->rot.y;

			newPos.x -= sinf(g_Player.rot.y) * g_Player.spd;
			newPos.z -= cosf(g_Player.rot.y) * g_Player.spd;
		}

		g_Player.pos = newPos;
	

		
		// 弾発射処理（共通関数使用） 
		if (IsMouseLeftTriggered() && g_Player.ammo > 0)
		{
			XMFLOAT3 pos = isFirstPersonMode ? GetGunMuzzlePosition() : g_Player.pos;  
			XMFLOAT3 rot = isFirstPersonMode ? GetGunMuzzleRotation() : g_Player.rot;  
			/*SetRevolverBullet(pos, rot);*/
			SetShotgunBullet(pos, rot, *GetShotgun()->bulletData);
			g_Player.ammo--;
		}
		// Rキーでリロード処理
		if (GetKeyboardTrigger(DIK_R))
		{
			// 弾が不足していて、かつ手持ちに弾がある場合のみリロード
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


	


	// レイキャストして足元の高さを求める
	//XMFLOAT3 HitPosition;		// 交点
	//XMFLOAT3 Normal;			// ぶつかったポリゴンの法線ベクトル（向き）
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


	//// 弾発射処理
	//if (GetKeyboardTrigger(DIK_SPACE))
	//{
	//	SetBullet(g_Player.pos, g_Player.rot);
	//}


	// 影もプレイヤーの位置に合わせる
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);


	


	// ポイントライトのテスト
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
	// デバッグ表示
	PrintDebugProc("Player X:%f Y:%f Z:%f \n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	PrintDebugProc("Player Ground:%s VertSpeed:%f\n", g_Player.isGround ? "TRUE" : "FALSE", g_Player.verticalSpeed);
	PrintDebugProc("FloorTree:%p\n", GetFloorTree());
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// クォータニオンを反映
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMFLOAT4X4 temp;
	DirectX::XMStoreFloat4x4(&temp, mtxWorld);
	g_Player.mtxWorld = temp;


	// 縁取りの設定
	SetFuchi(1);

	// モデル描画
	DrawModel(&g_Player.model);



	SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

