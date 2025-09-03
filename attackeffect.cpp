#include "attackeffect.h"
#include "enemy.h" 

void ApplyMeleeKnockback(const AttackContext& ctx)
{
    float halfCone = ctx.coneDeg * 0.5f * (3.14159265f / 180.0f);
    float cosLimit = cosf(halfCone);

    XMFLOAT3 fwd = atkutil::NormXZ(ctx.forward);

    auto& enemies = GetEnemies();
    for (auto* e : enemies) {
        if (!e || !e->IsUsed()) continue;

        XMFLOAT3 toE = atkutil::Sub(e->GetPosition(), ctx.origin);
        float dist = atkutil::LenXZ(toE);
        if (dist > ctx.radius) continue;

        XMFLOAT3 dir = atkutil::NormXZ(toE);
        if (ctx.coneDeg < 359.5f) {
            float dot = atkutil::DotXZ(dir, fwd);
            if (dot < cosLimit) continue; 
        }

        float falloff = 1.0f - (dist / ctx.radius);
        float power = ctx.strength * (falloff * 0.5f + 0.5f); 

        e->ApplyKnockback(dir, power, ctx.duration);
    }
}