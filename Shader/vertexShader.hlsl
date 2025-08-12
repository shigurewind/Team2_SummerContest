
#include "Common.hlsl"

//=============================================================================
// 頂点シェーダ
//=============================================================================
VertexOutput VertexShaderPolygon(VertexInput input)
{
    VertexOutput output;
    
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    output.Position = mul(input.Position, wvp);

    output.Normal = normalize(mul(float4(input.Normal.xyz, 0.0f), World));

    output.TexCoord = input.TexCoord;

    output.WorldPos = mul(input.Position, World);

    output.Color = input.Color;
    
    return output;
    
}




