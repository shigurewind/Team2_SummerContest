#include "enemy_bug.h"

#define ENEMY_OFFSET_Y  (-100.0f)

extern std::vector<BaseEnemy*> g_enemies;
extern ID3D11Buffer* g_VertexBufferEnemy;


//重力
static float gravity = 0.5f;

static int frameCount = 0;


BugEnemy::BugEnemy() : texture(nullptr), width(40.0f), height(40.0f)
{
    material = new MATERIAL{};
    XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());
}

BugEnemy::~BugEnemy()
{
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
    delete material;
    material = nullptr;

}

void BugEnemy::Init()
{
    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/2Dpicture/enemy/bug.png",
        NULL, NULL, &texture, NULL);


    *material = {};
    material->Diffuse = XMFLOAT4(1, 1, 1, 1);

    pos = XMFLOAT3(0.0f, -200.0f, 0.0f);
    scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
    use = true;
    moveDir = XMFLOAT3(0.0f, 0.0f, 1.0f);       // 現在の動き方向
    moveChangeTimer = 2.0f;  // 向き変わるタイマー
    speed = 0.5f;			//エネミーのスピード
    currentFrame = 0;
    frameCounter = 0;
    frameInterval = 15;//change speed
    maxFrames = 1;

    minDistance = 50.0f;

    HP = 10;

}

void BugEnemy::Update()
{
    PLAYER* player = GetPlayer();
    BULLET* bullet = GetBullet();


    if (!use) return;

    BaseEnemy::Update();


    //if (isAttacking)    //攻撃のアニメーション処理
    //{
    //    currentFrame = 2;

    //}
    //else
    //{
    //    // 移動のアニメーション処理
    //    frameCounter++;
    //    if (frameCounter >= frameInterval) {
    //        frameCounter = 0;
    //        currentFrame = (currentFrame + 1) % 2;
    //    }
    //}



    // エネミーからプレイヤーまでのベクトル
    XMFLOAT3 dir;
    dir.x = player->pos.x - pos.x;
    dir.y = player->pos.y - pos.y;
    dir.z = player->pos.z - pos.z;



    //// プレイヤーの座標までの計算
    XMFLOAT3 toPlayer = { dir.x, dir.y, dir.z };

    float distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z;
    float range = 100.0f; // 発射範囲



    //攻撃行う範囲
    if (distSq < range * range)
    {
        ChasingPlayer(speed, range);

        if (!isAttacking && attackCooldownTimer <= 0.0f)
        {
            Attack();
        }

    }
    else
    {
        NormalMovement();

    }


    //弾と当たり判定？
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (!bullet[i].use) continue;

        XMFLOAT3 enemyHalfSize = { width / 2, height, 50.f }; //エネミーの当たり判定のサイズ

        if (CheckSphereAABBCollision(bullet[i].pos, bullet[i].size, pos, enemyHalfSize))
        {
            bullet[i].use = false;
            HP -= 1;
            if (HP <= 0)
            {
                use = false;
                DropItems(pos, GHOST);
            }
        }

    }

    if (isAttacking)
    {
        attackFrameTimer -= 1.0f / 60.0f;
        if (attackFrameTimer <= 0.0f)
        {
            isAttacking = false;
        }
    }

    if (attackCooldownTimer > 0.0f)
    {
        attackCooldownTimer -= 1.0f / 60.0f;
        if (attackCooldownTimer < 0.0f) attackCooldownTimer = 0.0f;
    }


}

void BugEnemy::Draw()
{
    if (!use || !texture || !g_VertexBufferEnemy) return;


    SetLightEnable(FALSE);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferEnemy, &stride, &offset);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CAMERA* cam = GetCamera();
    XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);

    XMMATRIX mtxWorld = XMMatrixIdentity();
    mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
    mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
    mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

    mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
    mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
    mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

    mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
    mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
    mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];

    XMMATRIX mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
    XMMATRIX mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);


    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBufferEnemy, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    VERTEX_3D* v = (VERTEX_3D*)msr.pData;

    float w = width, h = height;
    v[0].Position = XMFLOAT3(-w / 2, h, 0);
    v[1].Position = XMFLOAT3(w / 2, h, 0);
    v[2].Position = XMFLOAT3(-w / 2, 0, 0);
    v[3].Position = XMFLOAT3(w / 2, 0, 0);

    for (int i = 0; i < 4; ++i) {
        v[i].Normal = XMFLOAT3(0, 0, -1);
        v[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
    }

    float tw = 1.0f / maxFrames;
    float th = 1.0f;
    float tx = currentFrame * tw;
    float ty = 0.0f;

    v[0].TexCoord = XMFLOAT2(tx, ty);
    v[1].TexCoord = XMFLOAT2(tx + tw, ty);
    v[2].TexCoord = XMFLOAT2(tx, ty + th);
    v[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

    GetDeviceContext()->Unmap(g_VertexBufferEnemy, 0);

    SetAlphaTestEnable(FALSE);
    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetWorldMatrix(&mtxWorld);
    SetMaterial(*material);
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);


    GetDeviceContext()->Draw(4, 0);

}

void BugEnemy::NormalMovement()
{
}

void BugEnemy::Attack()
{
}
