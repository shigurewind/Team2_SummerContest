

//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************

// �}�g���N�X�o�b�t�@
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

// �}�e���A���o�b�t�@
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    int noTexSampling;
    float Dummy[2]; //16byte���E�p
};

cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// ���C�g�p�o�b�t�@
struct LIGHT
{
    float4 Direction[5];
    float4 Position[5];
    float4 Diffuse[5];
    float4 Ambient[5];
    float4 Attenuation[5];
    int4 Flags[5];
    int Enable;
    int Dummy[3]; //16byte���E�p
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
    float Dummy[3]; //16byte���E�p
};

// �t�H�O�p�o�b�t�@
cbuffer FogBuffer : register(b5)
{
    FOG Fog;
};

// �����p�o�b�t�@
cbuffer Fuchi : register(b6)
{
    int fuchi;
    int fill[3];
};


cbuffer CameraBuffer : register(b7)
{
    float4 Camera;
}



//=============================================================================
// ���_�V�F�[�_
//=============================================================================
void VertexShaderPolygon(in float4 inPosition : POSITION0,
						  in float4 inNormal : NORMAL0,
						  in float4 inDiffuse : COLOR0,
						  in float2 inTexCoord : TEXCOORD0,

						  out float4 outPosition : SV_POSITION,
						  out float4 outNormal : NORMAL0,
						  out float2 outTexCoord : TEXCOORD0,
						  out float4 outDiffuse : COLOR0,
						  out float4 outWorldPos : POSITION0)
{
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    outPosition = mul(inPosition, wvp);

    outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), World));

    outTexCoord = inTexCoord;

    outWorldPos = mul(inPosition, World);

    outDiffuse = inDiffuse;
}



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);


//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
void PixelShaderPolygon(in float4 inPosition : SV_POSITION,
                         in float4 inNormal : NORMAL0,
                         in float2 inTexCoord : TEXCOORD0,
                         in float4 inDiffuse : COLOR0,
                         in float4 inWorldPos : POSITION0,
                         out float4 outDiffuse : SV_Target)
{
    float4 color;
    if (Material.noTexSampling == 0)
    {
        color = g_Texture.Sample(g_SamplerState, inTexCoord);
        color *= inDiffuse;
    }
    else
    {
        color = inDiffuse;
    }
    
    if (Light.Enable == 0)
    {
        color = color * Material.Diffuse;
    }
    else
    {
        //Material.Diffuse���g�킸�iBug����j
        float3 finalColor = color.rgb * 0.7f; // ���̌�
        
        
        for (int i = 0; i < 5; i++)
        {
            if (Light.Flags[i].y == 1)
            {
                if (Light.Flags[i].x == 1) // ���s����
                {
                    // ����
                    float3 lightDir = normalize(-Light.Direction[i].xyz); //����
                    float lightIntensity = max(0.0f, dot(lightDir, normalize(inNormal.xyz)));
                    
                    
                    float3 diffuse = color.rgb * Light.Diffuse[i].rgb * lightIntensity;
                    finalColor += diffuse;
                }
                else if (Light.Flags[i].x == 2) // �_����
                {
                    float3 lightDir = normalize(Light.Position[i].xyz - inWorldPos.xyz);
                    float lightIntensity = max(0.0f, dot(lightDir, normalize(inNormal.xyz)));
                    
                    // ��������
                    float distance = length(Light.Position[i].xyz - inWorldPos.xyz);
                    float attenuation = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
                    
                    float3 diffuse = color.rgb * Light.Diffuse[i].rgb * lightIntensity * attenuation;
                    finalColor += diffuse;
                }
            }
        }
        
        
        finalColor = min(finalColor, 1.0f);
        
        
        color = float4(finalColor, color.a);
    }
    
    outDiffuse = color;
}