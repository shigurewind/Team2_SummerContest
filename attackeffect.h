#pragma once
#include <vector>
#include <cmath>
#include <DirectXMath.h>

using namespace DirectX;

class BaseEnemy; 
struct AttackContext {
    XMFLOAT3 origin;    
    XMFLOAT3 forward;   
    float    radius;    
    float    coneDeg;   
    float    strength;  
    float    duration;  
};

namespace atkutil {
    inline XMFLOAT3 Sub(const XMFLOAT3& a, const XMFLOAT3& b) {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }
    inline float DotXZ(const XMFLOAT3& a, const XMFLOAT3& b) {
        return a.x * b.x + a.z * b.z; 
    }
    inline float LenXZ(const XMFLOAT3& v) {
        return sqrtf(v.x * v.x + v.z * v.z);
    }
    inline XMFLOAT3 NormXZ(const XMFLOAT3& v) {
        float l = LenXZ(v);
        if (l < 1e-5f) return { 0,0,0 };
        return { v.x / l, 0.0f, v.z / l };
    }
}



void ApplyMeleeKnockback(const AttackContext& ctx);