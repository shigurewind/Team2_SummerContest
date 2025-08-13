

#include "common.hlsl"

//=============================================================================
// ピクセルシェーダ
//=============================================================================
float4 PixelShaderPolygon(VertexOutput input) : SV_Target
{
    float4 color;

    if (Material.noTexSampling == 0)
    {
        color = g_Texture.Sample(g_SamplerState, input.TexCoord);
        
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
    
    
    
     //ディゾルブ処理
    if (g_EffectFlags & EFFECT_DISSOLVE)
    {
        float dissolveResult = CalculateDissolve(input.TexCoord, g_DissolveAmount);
        if (dissolveResult < 0.1f)
            discard; // Remove dissolved parts

          // Add dissolve edge glow
        if (dissolveResult < 0.3f && dissolveResult > 0.1f)
        {
            float edgeFactor = (0.3f - dissolveResult) / 0.2f;
            color.rgb = lerp(color.rgb, g_DissolveColor.rgb, edgeFactor * g_DissolveColor.a);
        }
    }
    
    //Glow処理
    if (g_EffectFlags & EFFECT_GLOW)
    {
        float glowIntensity = g_CustomParam1.x;
        float3 glowColor = g_CustomParam1.yzw;
        color.rgb += glowColor * glowIntensity;
    }
    
    //Damage処理
    if (g_EffectFlags & EFFECT_DAMAGE)
    {
        float damageIntensity = g_CustomParam2.x;
        float3 damageColor = float3(1.0f, 0.2f, 0.2f); // red flash
        color.rgb = lerp(color.rgb, damageColor, damageIntensity);
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

	//縁取り
    if (fuchi == 1)
    {
        float angle = dot(normalize(input.WorldPos.xyz - Camera.xyz), normalize(input.Normal.xyz));
        if (angle > -0.3f)
        {
            color.rb = 1.0f;
            color.g = 0.0f;
        }
    }
    
    
    return color;

}
