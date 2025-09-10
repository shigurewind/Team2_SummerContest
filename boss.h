#pragma once
#include <vector>
#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include "enemy.h"

using namespace DirectX;

//*****************************************************************************
// BOSSスキルクラス
//*****************************************************************************
class BossSkill
{
public:
	BossSkill() : cooldownTimer(0.0f), cooldownTime(1.0f) {}
	virtual ~BossSkill() = default;

	virtual void Execute(class Boss* boss) = 0;
	virtual bool CanUse() const { return cooldownTimer <= 0.0f; }
	virtual void Update(float deltaTime) {
		if (cooldownTimer > 0.0f) cooldownTimer -= deltaTime;
	}

protected:
	float cooldownTimer;
	float cooldownTime;
	void StartCooldown() { cooldownTimer = cooldownTime; }
};

//*****************************************************************************
// BOSS段階
//*****************************************************************************
enum class BossPhase
{
	INACTIVE,   // 非活性
	PHASE_1,    // 段階1
	PHASE_2,    // 段階2
	PHASE_3,    // 段階3
	DYING       // 死亡
};

//*****************************************************************************
// BOSSクラス
//*****************************************************************************
class Boss : public BaseEnemy
{
public:
	Boss();
	virtual ~Boss();

	void Init() override;
	void Update() override;
	void Draw() override;

	// BOSS専用
	void SetTriggerZone(const XMFLOAT3& center, float radius);
	void SetFixedPosition(const XMFLOAT3& position);
	void SetFixedRotation(const XMFLOAT3& rotation);

	// 段階管理
	BossPhase GetCurrentPhase() const { return currentPhase; }
	void ForcePhaseChange(BossPhase newPhase);

	// スキル管理
	void AddSkill(std::unique_ptr<BossSkill> skill);
	void ExecuteRandomSkill();

	// トリガー管理
	bool IsPlayerInTriggerZone() const;
	void ActivateBoss();

	// 受撃管理
	void TriggerHitEffect();       // 受撃効果
	void UpdateHitEffect(float deltaTime);  // 更新

private:
	// 段階
	BossPhase currentPhase;
	float phaseChangeThresholds[3];  // 切り替え閾値
	bool isActivated;

	// 位置と回転
	XMFLOAT3 fixedPosition;
	XMFLOAT3 fixedRotation;
	bool useFixedPosition;

	// トリガー領域
	XMFLOAT3 triggerZoneCenter;
	float triggerZoneRadius;

	// スキルシステム
	std::vector<std::unique_ptr<BossSkill>> skills;
	std::vector<std::unique_ptr<BossSkill>> currentPhaseSkills;
	float skillTimer;
	float skillInterval;

	//スキルアニメーション
	bool isPlayingSkillAnimation;     // スキルアニメーション再生中フラグ
	float skillAnimationTimer;        // スキルアニメーションタイマー
	float skillAnimationDuration;     // スキルアニメーション時間
	int skillAnimationFrame;          // スキルアニメーションフレーム数


	// レンダリング
	ID3D11ShaderResourceView* phaseTextures[3];  // 段階ごとのテクスチャ
	struct MATERIAL* material;
	float width, height;

	// アニメーション
	int currentFrame;
	int frameCounter;
	int frameInterval;
	int maxFrames;

	// 内部関数
	void CheckPhaseTransition();
	void SetupPhaseSkills(BossPhase phase);
	void UpdateSkills(float deltaTime);
	XMMATRIX CreateFixedWorldMatrix();

	void PlaySkillAnimation(int frame, float duration);// スキルアニメーション再生開始(スキル実行関数に入れる)


	//攻撃される用
	bool isHit;
	float hitEffectTimer;          // 計算用
	float hitEffectDuration;       // エフェクト時間
	XMFLOAT3 originalPosition;     // 元々の位置、揺れ用
	float shakeIntensity;	       // 揺れの強さ
};

//*****************************************************************************
// スキル実装
//*****************************************************************************



//*****************************************************************************
// グローバル関数
//*****************************************************************************
void InitBoss();
void UpdateBoss();
void DrawBoss();
void UninitBoss();

Boss* GetBoss();
void SpawnBoss(const XMFLOAT3& position, const XMFLOAT3& triggerCenter, float triggerRadius);
