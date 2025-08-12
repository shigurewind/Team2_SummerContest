#include "Common.hlsl"


//=============================================================================
// ピクセルシェーダ
//=============================================================================
float4 PixelShaderPolygon(VertexOutput input) : SV_Target
{
    float4 color;

    if (Material.noTexSampling == 0)
    {
        color = g_Texture.Sample(g_SamplerState, input.TexCoord);

          // Alpha test
        if (color.a < 0.1f)
            clip(-1);

        color *= input.Color;
    }
    else
    {
        color = input.Color;
    }
    
    
    //光源計算
    color = CalculateLighting(input.WorldPos, input.Normal, color);
    
    //血痕
    if (g_EffectFlags & EFFECT_BLOOD_STAIN)
    {
        float bloodFactor = CalculateBloodStain(input.WorldPos.xyz);
        if (bloodFactor > 0.0f)
        {
              // Mix in blood color
            float3 bloodColor = float3(0.4f, 0.1f, 0.1f); // dark red
            color.rgb = lerp(color.rgb, bloodColor, bloodFactor);

              // Add some glossiness to blood areas
            color.rgb += float3(0.1f, 0.0f, 0.0f) * bloodFactor;
        }
    }
    
    //弾痕
    //if (g_EffectFlags & EFFECT_DAMAGE)
    //{
    //      // g_CustomParam1.xyz = bullet hole center
    //      // g_CustomParam1.w = bullet hole radius
    //    float3 holeCenter = g_CustomParam1.xyz;
    //    float holeRadius = g_CustomParam1.w;

    //    float distanceToHole = length(input.WorldPos.xyz - holeCenter);
    //    if (distanceToHole < holeRadius)
    //    {
    //        float holeFactor = 1.0f - (distanceToHole / holeRadius);
    //        color.rgb *= (1.0f - holeFactor * 0.7f); // darken bullet hole area
    //    }
    //}
    
    //Glow
    if (g_EffectFlags & EFFECT_GLOW) // reuse glow flag for wear effect
    {
        float wearIntensity = g_CustomParam2.x;
        float3 wearColor = float3(0.3f, 0.25f, 0.2f); // brownish wear

          // Add noise-based wear pattern using texture coordinates
        float wearNoise = frac(sin(dot(input.TexCoord.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
        if (wearNoise > (1.0f - wearIntensity))
        {
            color.rgb = lerp(color.rgb, wearColor, 0.5f);
        }
    }

	//フォグ
    if (Fog.Enable == 1)
    {
        float z = input.Position.z * input.Position.w;
        float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
        f = saturate(f);
        color = f * color + (1 - f) * Fog.FogColor;
        color.a = color.a;
    }

    
    return color;
    
    
    

}
