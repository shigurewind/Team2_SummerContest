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
        float4 bloodEffect = CalculateBloodEffect(input.WorldPos.xyz);

        if (bloodEffect.a > 0.01f) 
        {
      
            //color.rgb = lerp(color.rgb, bloodEffect.rgb, bloodEffect.a);//アルファブレンドあり
            color.rgb = bloodEffect.rgb; //アルファブレンドなし

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
		float3 toP = input.WorldPos.xyz - Camera.xyz;

		float distXZ = length(toP.xz);
		float startXZ = Fog.Distance.x;
		float endXZ = Fog.Distance.y;
		float fogXZ = saturate((distXZ - startXZ) / max(endXZ - startXZ, 1e-5));

		float distY = abs(toP.y);
		float startY = Fog.Distance.z;
		float endY = Fog.Distance.w;
		float fogY = saturate((distY - startY) / max(endY - startY, 1e-5));

		float f = max(fogXZ, fogY);

		color.rgb = lerp(color.rgb, Fog.FogColor.rgb, f);
	}

    
    return color;
    
    
    

}
