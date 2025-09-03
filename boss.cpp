#include "boss.h"
#include "player.h"
#include "bullet.h"
#include "debugproc.h"
#include "camera.h"
#include "main.h"
#include "renderer.h"
#include "sprite.h"
#include "input.h"
#include "collision.h"
#include "GameUI.h"
#include "blood.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static Boss* g_Boss = nullptr;
static ID3D11Buffer* g_VertexBufferBoss = nullptr;

//*****************************************************************************
// Bossクラス
//*****************************************************************************
Boss::Boss() 
    : currentPhase(BossPhase::INACTIVE)
    , isActivated(false)
    , useFixedPosition(true)
    , fixedPosition({0.0f, 0.0f, 100.0f})
    , fixedRotation({0.0f, 0.0f, 0.0f})
    , triggerZoneCenter({0.0f, 0.0f, 80.0f})
    , triggerZoneRadius(50.0f)
    , skillTimer(0.0f)
    , skillInterval(3.0f)
    , width(200.0f)
    , height(200.0f)
    , currentFrame(0)
    , frameCounter(0)
    , frameInterval(10)
    , maxFrames(2)
    , isPlayingSkillAnimation(false)
    , skillAnimationTimer(0.0f)
    , skillAnimationDuration(1.0f)
    , skillAnimationFrame(0)
{
	// 切り替え閾値初期化
    phaseChangeThresholds[0] = 0.7f;  // 70％段階2に入る
	phaseChangeThresholds[1] = 0.33f;  // 10％段階3に入る
    phaseChangeThresholds[2] = 0.0f;   // 0死亡

	// テクスチャ初期化
    for (int i = 0; i < 3; ++i) {
        phaseTextures[i] = nullptr;
    }

    material = new MATERIAL{};
    XMStoreFloat4x4(&mtxWorld, XMMatrixIdentity());

	immuneToKnockback = true; // ノックバック無効
}

Boss::~Boss() 
{
	// テクスチャ解放
    for (int i = 0; i < 3; ++i) {
        if (phaseTextures[i]) {
            phaseTextures[i]->Release();
            phaseTextures[i] = nullptr;
        }
    }

    if (material) {
        delete material;
        material = nullptr;
    }
}

void Boss::Init() 
{
	// テクスチャ読み込み
    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/2Dpicture/boss/boss_phase1.png",  
        NULL, NULL, &phaseTextures[0], NULL);

    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/2Dpicture/boss/boss_phase2.png",
        NULL, NULL, &phaseTextures[1], NULL);

    D3DX11CreateShaderResourceViewFromFile(
        GetDevice(),
        "data/2Dpicture/boss/boss_phase3.png",
        NULL, NULL, &phaseTextures[2], NULL);

    if (!phaseTextures[0]) 
    {
        D3DX11CreateShaderResourceViewFromFile(
            GetDevice(),
            "data/2Dpicture/enemy/enemy001.png",
            NULL, NULL, &phaseTextures[0], NULL);
	}
    

	// マテリアル初期化
    *material = {};
    material->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// 属性初期化
    pos = fixedPosition;
    scl = XMFLOAT3(2.0f, 2.0f, 2.0f);  // 大きい
    use = true;
    
    // HP
    maxHP = 300;  
    HP = maxHP;
    
    // 位置固定	
    EnableGravity(false);
    
    currentPhase = BossPhase::INACTIVE;
    
	// スキル初期化
    SetupPhaseSkills(BossPhase::PHASE_1);
}

void Boss::Update() 
{
    if (!use) return;

    const float deltaTime = 1.0f / 60.0f;

	// BOSS戦闘開始判定
    if (!isActivated && IsPlayerInTriggerZone()) {
        ActivateBoss();
    }

    if (!isActivated) return;


	// アニメーション更新
    if (isPlayingSkillAnimation) {
        // スキル
        skillAnimationTimer -= deltaTime;
        if (skillAnimationTimer <= 0.0f) {
            // 終わり
            isPlayingSkillAnimation = false;
            frameCounter = 0;
            currentFrame = 0;
        }
        else {
            // 維持する
            currentFrame = skillAnimationFrame;
        }
    }
    else {
		// 段階通常アニメーション
        frameCounter++;
        if (frameCounter >= frameInterval) {
            frameCounter = 0;
            currentFrame = (currentFrame + 1) % maxFrames;
        }
    }

    // 段階チェック
    CheckPhaseTransition();
    
    // 
    //UpdateSkills(deltaTime);

    // 当たり判定
    BULLET* bullet = GetBullet();
    for (int i = 0; i < MAX_BULLET; i++) {
        if (!bullet[i].use) continue;

        XMFLOAT3 bossHalfSize = { width/2, height/2, 50.0f };
        
        if (CheckSphereAABBCollision(bullet[i].pos, bullet[i].size, pos, bossHalfSize)) {
            bullet[i].use = false;
			HP -= 10;  // ダメージ量
            
            // エフェクト
            XMFLOAT3 closestPoint;
            closestPoint.x = max(pos.x - bossHalfSize.x, min(bullet[i].pos.x, pos.x + bossHalfSize.x));
            closestPoint.y = max(pos.y - bossHalfSize.y, min(bullet[i].pos.y, pos.y + bossHalfSize.y));
            closestPoint.z = max(pos.z - bossHalfSize.z, min(bullet[i].pos.z, pos.z + bossHalfSize.z));

            XMVECTOR v = XMVector3Normalize(XMLoadFloat3(&bullet[i].vel));
            XMFLOAT3 hitNormal;
            XMStoreFloat3(&hitNormal, v);
            SpawnBlood(closestPoint, 12, hitNormal);

            //死亡処理
            if (HP <= 0) {
                currentPhase = BossPhase::DYING;
				//TODO: 死亡エフェクト
               
            }
        }
    }

#ifdef _DEBUG
    float healthPercent = (maxHP > 0) ? (float)HP / (float)maxHP : 0.0f;
    PrintDebugProc("Boss HP: %d/%d (%.1f%%)\n", HP, maxHP, healthPercent * 100.0f);
    PrintDebugProc("Boss Phase: %d\n", (int)currentPhase);
    PrintDebugProc("Boss Activated: %s\n", isActivated ? "YES" : "NO");
#endif
}

void Boss::Draw() 
{
    if (!use || currentPhase == BossPhase::INACTIVE) return;

    SetLightEnable(FALSE);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBufferBoss, &stride, &offset);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//固定ワールド行列作成
    XMMATRIX mtxWorld = CreateFixedWorldMatrix();

	// 頂点バッファ更新
    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBufferBoss, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
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

    // UV	
    float tw = 1.0f / maxFrames;
    float th = 1.0f;
    float tx = currentFrame * tw;
    float ty = 0.0f;

    v[0].TexCoord = XMFLOAT2(tx, ty);
    v[1].TexCoord = XMFLOAT2(tx + tw, ty);
    v[2].TexCoord = XMFLOAT2(tx, ty + th);
    v[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

    GetDeviceContext()->Unmap(g_VertexBufferBoss, 0);

    // レンダリング
    SetAlphaTestEnable(FALSE);
    SetBlendState(BLEND_MODE_ALPHABLEND);
    SetWorldMatrix(&mtxWorld);
    SetMaterial(*material);

	// 段階に応じたテクスチャセット
    ID3D11ShaderResourceView* currentTexture = nullptr;
    switch (currentPhase) {
        case BossPhase::PHASE_1: currentTexture = phaseTextures[0]; break;
        case BossPhase::PHASE_2: currentTexture = phaseTextures[1]; break;
        case BossPhase::PHASE_3: currentTexture = phaseTextures[2]; break;
        default: currentTexture = phaseTextures[0]; break;
    }

    if (currentTexture) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &currentTexture);
        GetDeviceContext()->Draw(4, 0);
    }
}

// トリガー領域
void Boss::SetTriggerZone(const XMFLOAT3& center, float radius) 
{
    triggerZoneCenter = center;
    triggerZoneRadius = radius;
}

// 固定位置設定
void Boss::SetFixedPosition(const XMFLOAT3& position) 
{
    fixedPosition = position;
    pos = position;
    useFixedPosition = true;
}

// 固定回転設定
void Boss::SetFixedRotation(const XMFLOAT3& rotation) 
{
    fixedRotation = rotation;
}

// プレイヤーがトリガー領域内にいるかチェック
bool Boss::IsPlayerInTriggerZone() const 
{
    XMFLOAT3 playerPos = GetPlayer()->GetPosition();
    
    float dx = playerPos.x - triggerZoneCenter.x;
    float dy = playerPos.y - triggerZoneCenter.y;
    float dz = playerPos.z - triggerZoneCenter.z;
    
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    return distanceSquared <= (triggerZoneRadius * triggerZoneRadius);
}

// BOSS戦闘開始
void Boss::ActivateBoss() 
{
    if (isActivated) return;
    
    isActivated = true;
    currentPhase = BossPhase::PHASE_1;
    SetupPhaseSkills(currentPhase);
    
#ifdef _DEBUG
    PrintDebugProc("BOSS ACTIVATED!\n");
#endif
}



// 段階チェック
void Boss::CheckPhaseTransition() 
{
    if (HP <= 0 && currentPhase != BossPhase::DYING) {
        currentPhase = BossPhase::DYING;
        return;
    }
    
    float healthPercent = (float)HP / (float)maxHP;
    
	// 段階1から2へ
    if (currentPhase == BossPhase::PHASE_1 && healthPercent <= phaseChangeThresholds[0]) {
        ForcePhaseChange(BossPhase::PHASE_2);
    }
	// 段階2から3へ
    else if (currentPhase == BossPhase::PHASE_2 && healthPercent <= phaseChangeThresholds[1]) {
        ForcePhaseChange(BossPhase::PHASE_3);
    }
}

// 強制段階変更
void Boss::ForcePhaseChange(BossPhase newPhase) 
{
    if (currentPhase == newPhase) return;
    
    currentPhase = newPhase;

    switch (newPhase) {
    case BossPhase::PHASE_1:
        maxFrames = 1;  // 通常アニメーションフレーム数
        break;
    case BossPhase::PHASE_2:
        maxFrames = 1;  
        break;
    case BossPhase::PHASE_3:
        maxFrames = 1;  
        break;
    }

    SetupPhaseSkills(newPhase);
    
	// アニメーションリセット
    isPlayingSkillAnimation = false;
    frameCounter = 0;
    currentFrame = 0;
    
#ifdef _DEBUG
    PrintDebugProc("BOSS Phase changed to: %d\n", (int)newPhase);
#endif
}



// 固定ワールド行列作成
XMMATRIX Boss::CreateFixedWorldMatrix() 
{
    
    XMMATRIX mtxRotX = XMMatrixRotationX(fixedRotation.x);
    XMMATRIX mtxRotY = XMMatrixRotationY(fixedRotation.y);
    XMMATRIX mtxRotZ = XMMatrixRotationZ(fixedRotation.z);
    XMMATRIX mtxRot = XMMatrixMultiply(XMMatrixMultiply(mtxRotX, mtxRotY), mtxRotZ);
    
    XMMATRIX mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
    XMMATRIX mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
    
    return XMMatrixMultiply(XMMatrixMultiply(mtxScl, mtxRot), mtxTranslate);
}


//*****************************************************************************
// スキル基本実装
//*****************************************************************************

// スキルアニメーション
void Boss::PlaySkillAnimation(int frame, float duration)
{
    isPlayingSkillAnimation = true;
    skillAnimationFrame = frame;
    skillAnimationTimer = duration;
    skillAnimationDuration = duration;
	currentFrame = frame;  // そのフレームに切り替え
}

// スキル追加
void Boss::AddSkill(std::unique_ptr<BossSkill> skill)
{
    
}


// 段階に応じたスキル設定
void Boss::SetupPhaseSkills(BossPhase phase)
{
    skills.clear();

    switch (phase) {
    case BossPhase::PHASE_1:

        skillInterval = 3.0f;
        break;

    case BossPhase::PHASE_2:

        skillInterval = 2.0f;
        break;

    case BossPhase::PHASE_3:

        skillInterval = 1.5f;
        break;
    }
}

// スキル更新
void Boss::UpdateSkills(float deltaTime)
{
    // スキルクールダウン更新
    for (auto& skill : skills) {
        skill->Update(deltaTime);
    }

    // スキル発動タイマー更新
    if (skillTimer <= 0.0f && isActivated) {
        ExecuteRandomSkill();
        skillTimer = skillInterval;
    }
}

// ランダムスキル実行
void Boss::ExecuteRandomSkill()
{
    // 使用可能なスキルを収集
    std::vector<BossSkill*> availableSkills;
    for (auto& skill : skills) {
        if (skill->CanUse()) {
            availableSkills.push_back(skill.get());
        }
    }

    if (!availableSkills.empty()) {
        int randomIndex = rand() % availableSkills.size();
        availableSkills[randomIndex]->Execute(this);
    }
}




//*****************************************************************************
// グローバル関数
//*****************************************************************************

void InitBoss() 
{
	// 頂点バッファ作成
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    GetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBufferBoss);
    
    g_Boss = nullptr;
}

void UpdateBoss() 
{
    if (g_Boss && g_Boss->IsUsed()) {
        g_Boss->Update();
    }
}

void DrawBoss() 
{
    if (g_Boss && g_Boss->IsUsed()) {
        g_Boss->Draw();
    }
}

void UninitBoss() 
{
    if (g_Boss) {
        delete g_Boss;
        g_Boss = nullptr;
    }
    
    if (g_VertexBufferBoss) {
        g_VertexBufferBoss->Release();
        g_VertexBufferBoss = nullptr;
    }
}

Boss* GetBoss() 
{
    return g_Boss;
}

void SpawnBoss(const XMFLOAT3& position, const XMFLOAT3& triggerCenter, float triggerRadius) 
{
    if (g_Boss) {
        delete g_Boss;
    }
    
    g_Boss = new Boss();
    g_Boss->SetFixedPosition(position);
    g_Boss->SetTriggerZone(triggerCenter, triggerRadius);
    g_Boss->Init();
    g_Boss->SetUsed(true);
}