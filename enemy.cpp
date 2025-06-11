#include "enemy.h"
#include "player.h"
#include "bullet.h"
#include "shadow.h"
#include "collision.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量


//エネミーの動き範囲
#define ENEMY_AREA_MIN_X	(-80.0f)
#define ENEMY_AREA_MAX_X	(80.0f)
#define ENEMY_AREA_MIN_Z	(-80.0f)
#define ENEMY_AREA_MAX_Z	(80.0f)
#define ENEMY_AREA_MIN_Y	(7.0f)
#define ENEMY_AREA_MAX_Y	(80.0f)

#define MOVECOUNTER			(300)		// 向き変わるタイマー




//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー
int g_Enemy_load = 0;


static INTERPOLATION_DATA g_MoveTbl0[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 5.0f },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 5.0f },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 5.0f },

};

static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	g_MoveTbl0,

};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, 7.0f, 20.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd  = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ
		g_Enemy[i].type = NONE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[i].model, &g_Enemy[i].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);
		

		g_Enemy[i].time = 0.0f;			// 線形補間用のタイマーをクリア
		g_Enemy[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Enemy[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット
	
		g_Enemy[i].fireCooldown = 5.0f;
		g_Enemy[i].fireTimer = 0.0f;


		g_Enemy[i].use = TRUE;			// TRUE:生きてる
	}

BaseEnemy::~BaseEnemy() {}

//----------------------------

	g_Enemy[1].type = SKELETON;
	// 1番だけ線形補間で動かしてみる
	g_Enemy[1].time = 0.0f;		// 線形補間用のタイマーをクリア
	g_Enemy[1].tblNo = 0;		// 再生するアニメデータの先頭アドレスをセット
	g_Enemy[1].tblMax = sizeof(g_MoveTbl0) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	
	g_Enemy[2].type = SPIDER;


	
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
}

ScarecrowEnemy::~ScarecrowEnemy() {}

void ScarecrowEnemy::Init()
{

	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{

		switch (g_Enemy[i].type)
		{
		case GHOST:
			GhostMovement(i);
			break;

		case SKELETON:
			SkeletonMovement(i);
			break;

		case SPIDER:
			SpiderMovement(i);
			break;

		default:
			break;
		}
	}

void ScarecrowEnemy::Update()
{
	// 不动，无需逻辑
}

	if (GetKeyboardTrigger(DIK_L))
	{
		// モデルの色を元に戻している
		for (int j = 0; j < g_Enemy[0].model.SubsetNum; j++)
		{
			SetModelDiffuse(&g_Enemy[0].model, j, g_Enemy[0].diffuse[j]);
		}
	}
#endif

}



//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);



	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);

			// モデル描画
			DrawModel(&g_Enemy[i].model);
		
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

	SetWorldMatrix(&this->mtxWorld);
	DrawSprite3D(texID);
}


//=============================================================================
// エネミーの動き
//=============================================================================

void ChangeEnemyDirection(int i) {
	float theta = (rand() % 360) * XM_PI / 180.0f;
	float phi = ((rand() % 90) + 45) * XM_PI / 180.0f;

	XMFLOAT3 dir;
	dir.x = sinf(phi) * cosf(theta);
	dir.y = cosf(phi);
	dir.z = sinf(phi) * sinf(theta);



static BaseEnemy* g_enemy = nullptr;

void InitEnemy()
{
	g_enemy = new ScarecrowEnemy();
	g_enemy->Init();
}

void UpdateEnemy()
{
	if (g_enemy && g_enemy->IsUsed()) g_enemy->Update();
}

void DrawEnemy()
{
	if (g_enemy && g_enemy->IsUsed()) g_enemy->Draw();
}

void UninitEnemy()
{
	if (g_enemy)
	{
		delete g_enemy;
		g_enemy = nullptr;
	}

	XMFLOAT3 pos = g_Enemy[i].pos;
	pos.y -= (ENEMY_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Enemy[i].shadowIdx, pos);
}

void SpiderMovement(int i)
{
	ChasingPlayer(i);

	PLAYER* player = GetPlayer();

	// エネミーからプレイヤーまでのベクトル
	XMFLOAT3 dir;
	dir.x = player->pos.x - g_Enemy[i].pos.x;
	dir.y = player->pos.y - g_Enemy[i].pos.y;
	dir.z = player->pos.z - g_Enemy[i].pos.z;

	if (g_Enemy[i].fireTimer > 0.0f)
	{
		g_Enemy[i].fireTimer -= 1.0f / 60.0f;  // 60fps
	}

	// プレイヤーの座標までの計算
	XMFLOAT3 toPlayer = {dir.x, dir.y, dir.z};

	float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
	float range = 100.0f; // 発射範囲

	if (distSq < range * range)
	{
		// 発射するとき
		if (g_Enemy[i].fireTimer <= 0.0f)
		{
			SetBullet(g_Enemy[i].pos, g_Enemy[i].rot);

			// Reset timer
			g_Enemy[i].fireTimer = g_Enemy[i].fireCooldown;
		}
	}
}