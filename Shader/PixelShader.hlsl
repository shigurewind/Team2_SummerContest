struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer
{
    float4x4 World; // ワールド変換行列
    float4x4 View; // ビュー変換行列
    float4x4 Projection; // 透視射影変換行列
    float4 CameraPos; // カメラ座標
    float4 LightVector; // ライト方向
    float4 LightColor; // ライトカラー
    float4 MaterialAmbient; // アンビエント
    float4 MaterialDiffuse; // ディフューズ
    float4 MaterialSpecular; // スペキュラー
}

Texture2D Texture : register(t0[0]); // Textureをスロット0の0番目のテクスチャレジスタに設定
SamplerState Sampler : register(s0[0]); // Samplerをスロット0の0番目のサンプラレジスタに設定

float4 main(PS_IN input) : SV_Target
{
    float ambient_factor = MaterialAmbient[3];
    float diffuse_factor = MaterialDiffuse[3];

    float4 ambient_color = MaterialAmbient * ambient_factor;
    float4 diffuse_color = input.color * (LightColor * MaterialDiffuse) * diffuse_factor;

	// アンビエントカラー + ディフューズカラー
    return ambient_color + diffuse_color;
}
