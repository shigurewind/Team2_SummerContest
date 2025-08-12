

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
    float4 g_DissolveColor; // edge color

      // 血痕 (マップに)
    float3 g_BloodPositions[4]; // up to 4 blood positions
    float g_BloodRadii[4]; // corresponding radii
    float g_BloodIntensity; // overall blood intensity
    int g_BloodCount; // current blood stain count

      // Custom effect parameters
    float4 g_CustomParam1;
    float4 g_CustomParam2;

    float2 g_EffectPadding; // 16バイト用
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
SamplerState g_SamplerState : register(s0);

//*****************************************************************************
// ツール関数
//*****************************************************************************

  // Calculate dissolve effect
float CalculateDissolve(float2 uv, float dissolveAmount)
{
    if (!(g_EffectFlags & EFFECT_DISSOLVE))
        return 1.0f;

    float dissolveVal = g_DissolveMap.Sample(g_SamplerState, uv).r;
    return dissolveVal > dissolveAmount ? 1.0f : 0.0f;
}

  // Calculate blood stain effect for terrain
float CalculateBloodStain(float3 worldPos)
{
    if (!(g_EffectFlags & EFFECT_BLOOD_STAIN))
        return 0.0f;

    float totalBlood = 0.0f;

    for (int i = 0; i < g_BloodCount && i < 4; i++)
    {
        float distance = length(worldPos - g_BloodPositions[i]);
        float bloodFactor = saturate(1.0f - (distance / g_BloodRadii[i]));
        totalBlood += bloodFactor;
    }

    return saturate(totalBlood * g_BloodIntensity);
}

  // 光源計算関数
float4 CalculateLighting(float4 worldPos, float4 normal, float4 baseColor)
{
    if (Light.Enable == 0)
    {
        return baseColor * Material.Diffuse;
        
    }

    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 5; i++)
    {
        if (Light.Flags[i].y == 1) // light is enabled
        {
            float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
            
            

            if (Light.Flags[i].x == 1) // 平行光
            {
                float3 lightDir = normalize(-Light.Direction[i].xyz);
                float lightIntensity = max(0.0f, dot(lightDir, normalize(normal.xyz)));
                tempColor = baseColor * Light.Diffuse[i] * lightIntensity;
                
            }
            else if (Light.Flags[i].x == 2) // ポイントライト
            {
                float3 lightDir = normalize(Light.Position[i].xyz - worldPos.xyz);
                float lightIntensity = max(0.0f, dot(lightDir, normalize(normal.xyz)));

                  // attenuation
                float distance = length(Light.Position[i].xyz - worldPos.xyz);
                float attenuation = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);

                tempColor = baseColor * Light.Diffuse[i] * lightIntensity * attenuation;
            }

            finalColor += tempColor;
        }
    }
    
    finalColor = min(finalColor, 1.0f);

    finalColor.a = baseColor.a * Material.Diffuse.a;
    return finalColor;
}
