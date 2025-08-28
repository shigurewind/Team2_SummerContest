

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

// マトリクスバッファ
cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

// マテリアルバッファ
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    int noTexSampling;
    float Dummy[2]; //16byte境界用
};

cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// ライト用バッファ
struct LIGHT
{
    float4 Direction[5];
    float4 Position[5];
    float4 Diffuse[5];
    float4 Ambient[5];
    float4 Attenuation[5];
    int4 Flags[5];
    int Enable;
    int Dummy[3]; //16byte境界用
};

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}

struct FOG
{
    float4 Distance;
    float4 FogColor;
    int Enable;
    float Dummy[3]; //16byte境界用
};

// フォグ用バッファ
cbuffer FogBuffer : register(b5)
{
    FOG Fog;
};

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
    int fuchi;
    int fill[3];
};


cbuffer CameraBuffer : register(b7)
{
    float4 Camera;
}

//エフェクト用のバッファ
cbuffer EffectBuffer : register(b8)
{
      // Effect control flags (bitwise)
    uint g_EffectFlags;
    

      // ディゾルブ (敵)
    float g_DissolveAmount; // 0.0 - 1.0
    float2 padding1;
    float4 g_DissolveColor; // edge color

      // 血痕 (マップに)
    float4 g_BloodPositions[8]; // up to 4 blood positions
    float4 g_BloodRadii[2]; // corresponding radii
    float g_BloodIntensity; // overall blood intensity
    int g_BloodCount; // current blood stain count
    float2 padding2;
    
    float4 g_BloodProjections[8];

      // Custom effect parameters
    float4 g_CustomParam1;
    float4 g_CustomParam2;

    float4 padding3; // 16バイト用
};

//*****************************************************************************
  // Effect Flag Definitions
  //*****************************************************************************
#define EFFECT_DISSOLVE     0x01
#define EFFECT_BLOOD_STAIN  0x02
#define EFFECT_GLOW         0x04
#define EFFECT_DAMAGE       0x08

  //*****************************************************************************
  // Common Structures
  //*****************************************************************************

  // Vertex input structure
struct VertexInput
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

  // Vertex output structure
struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    float4 WorldPos : POSITION0;
};

//*****************************************************************************
// Common Textures and Samplers
//*****************************************************************************
Texture2D g_Texture : register(t0);
Texture2D g_DissolveMap : register(t1); // ノイズテクスチャ
Texture2D g_BloodTexture : register(t2); // 血痕テクスチャ
SamplerState g_SamplerState : register(s0);

//*****************************************************************************
// ツール関数
//*****************************************************************************

  // ディゾルブエフェクト計算
float CalculateDissolve(float2 uv, float dissolveAmount)
{
    if (!(g_EffectFlags & EFFECT_DISSOLVE))
        return 1.0f;

    float dissolveVal = g_DissolveMap.Sample(g_SamplerState, uv).r;
    //return dissolveVal > dissolveAmount ? 1.0f : 0.0f;
    float edge = 0.1f; // 
    return saturate((dissolveVal - dissolveAmount + edge) / edge);
    
    
    
}

  // マップの血痕エフェクト計算
float CalculateBloodStain(float3 worldPos)
{
    if (!(g_EffectFlags & EFFECT_BLOOD_STAIN))
        return 0.0f;

    float totalBlood = 0.0f;

    for (int i = 0; i < g_BloodCount && i < 8; i++)
    {
        float3 bloodCenter = g_BloodPositions[i].xyz;
        float3 projDir = g_BloodProjections[i].xyz;
        float intensity = g_BloodProjections[i].w;

          // 半径の取得
        float radius;
        if (i < 4)
        {
            if (i == 0)
                radius = g_BloodRadii[0].x;
            else if (i == 1)
                radius = g_BloodRadii[0].y;
            else if (i == 2)
                radius = g_BloodRadii[0].z;
            else if (i == 3)
                radius = g_BloodRadii[0].w;
        }
        else
        {
            if (i == 4)
                radius = g_BloodRadii[1].x;
            else if (i == 5)
                radius = g_BloodRadii[1].y;
            else if (i == 6)
                radius = g_BloodRadii[1].z;
            else if (i == 7)
                radius = g_BloodRadii[1].w;
        }

          // 血痕の中心からピクセルまでの距離
        float3 toPixel = worldPos - bloodCenter;
        float distance = length(toPixel);
        
        if (distance > radius)
            continue;
        
        // 血痕テクスチャのUV座標（中心が(0.5,0.5)になるように調整）
        float2 bloodUV = (toPixel.xz / radius) * 0.5f + 0.5f;
        float bloodTexSample = g_BloodTexture.Sample(g_SamplerState, bloodUV).r;
        

          // 距離に基づく血痕の強度（半径内で最大、外で0）
        float bloodFactor = saturate(1.0f - (distance / radius));
        
        bloodFactor *= bloodTexSample; // テクスチャ応用

          // 投影方向が指定されている場合、その方向に基づいて血痕を強調
        if (length(projDir) > 0.1f) // ある
        {
            float3 normalizedProjDir = normalize(projDir);
            float3 normalizedToPixel = normalize(toPixel);

              
            float projectionFactor = saturate(1.0f + dot(normalizedToPixel, normalizedProjDir) * 0.5f);
            bloodFactor *= projectionFactor;
        }

          // 強度を調整
        bloodFactor *= intensity;

          // エッジを滑らかにフェードアウト
        float edgeFade = smoothstep(0.9f, 0.3f, distance / radius);
        bloodFactor *= edgeFade;

        totalBlood += bloodFactor;
    }

    return saturate(totalBlood * g_BloodIntensity);
}


// スポットライト寄与計算
float3 SpotContribution(int i, float3 worldPos, float3 N, float4 baseColor)
{
    // L: 表面→光
    float3 Lvec = Light.Position[i].xyz - worldPos;
    float dist = length(Lvec);
    float3 L = Lvec / max(dist, 1e-5);

    // 距離減衰（range = Attenuation.x）
    float range = Light.Attenuation[i].x;
    float atten = saturate((range - dist) / max(range, 1e-5));

    // 角度減衰（内外コーン）
    float3 spotDir = normalize(Light.Direction[i].xyz); // ライトが向いている方向
    float c = dot(-L, spotDir); // 光軸とのcos角
    float inner = Light.Attenuation[i].y; // 内側コーンのcos
    float outer = Light.Attenuation[i].z; // 外側コーンのcos
    float t = saturate((c - outer) / max(inner - outer, 1e-5));
    float expo = Light.Attenuation[i].w; // フェードの鋭さ
    float spot = pow(t, expo);

    // ランバート
    float ndotl = saturate(dot(N, L));

    return (baseColor.rgb * Light.Diffuse[i].rgb) * (ndotl * atten * spot);
}


  // 光源計算関数
float4 CalculateLighting(float4 worldPos, float4 normal, float4 baseColor)
{
    if (Light.Enable == 0)
    {
        return baseColor * Material.Diffuse;
    }

    // 正規化は1回だけやっておくと軽い
    float3 N = normalize(normal.xyz);

    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // NOTE: ループ上限 5 は環境に合わせて（いまのコードに揃えました）
    for (int i = 0; i < 5; i++)
    {
        if (Light.Flags[i].y == 1) // enabled
        {
            float4 tempColor = float4(0, 0, 0, 0);

            if (Light.Flags[i].x == 1) // 1: 平行光
            {
                float3 L = normalize(-Light.Direction[i].xyz);
                float ndotl = max(0.0f, dot(L, N));

                float4 diffuse = baseColor * Light.Diffuse[i] * ndotl;
                float4 ambient = baseColor * Light.Ambient[i]; // 既存仕様どおり

                tempColor = diffuse + ambient;
            }
            else if (Light.Flags[i].x == 2)      // 2: ポイント
            {
                float3 Lvec = Light.Position[i].xyz - worldPos.xyz;
                float dist = length(Lvec);
                float3 L = Lvec / max(dist, 1e-5);

                float ndotl = max(0.0f, dot(L, N));

                // 距離減衰：Attenuation[i].x を「到達距離(range)」として線形減衰
                float range = Light.Attenuation[i].x;
                float atten = saturate((range - dist) / max(range, 1e-5));

                tempColor = baseColor * Light.Diffuse[i] * (ndotl * atten);
            }
            else if (Light.Flags[i].x == 3)       // 3: スポット（★追加）
            {
                // L: 表面→光
                float3 Lvec = Light.Position[i].xyz - worldPos.xyz;
                float dist = length(Lvec);
                float3 L = Lvec / max(dist, 1e-5);

                // 距離減衰（ポイントと同じ式を再利用）
                float range = Light.Attenuation[i].x; // x = 距離（到達範囲）
                float atten = saturate((range - dist) / max(range, 1e-5));

                // 角度減衰：Attenuation[i].y/z/w を使用
                //   y = 内側コーンの cosθ, z = 外側コーンの cosθ, w = 縁の鋭さ(指数)
                float3 spotDir = normalize(Light.Direction[i].xyz); // ライトの向き（軸）
                float c = dot(-L, spotDir); // 光軸との cosθ
                float inner = Light.Attenuation[i].y;
                float outer = Light.Attenuation[i].z;
                float expo = Light.Attenuation[i].w;

                // 内外コーンの間をスムーズに補間（外側→0, 内側→1）
                float t = saturate((c - outer) / max(inner - outer, 1e-5));
                float spot = pow(t, expo);

                float ndotl = max(0.0f, dot(L, N));

                // 懐中電灯らしく環境光は加算しない（必要なら Ambient を足してもOK）
                tempColor = baseColor * Light.Diffuse[i] * (ndotl * atten * spot);
            }

            finalColor += tempColor;
        }
    }

    finalColor = min(finalColor, 1.0f);
    finalColor.a = baseColor.a * Material.Diffuse.a;
    return finalColor;

}
