// テクスチャ
Texture2D Texture : register(t0);

// サンプラー
SamplerState Sampler : register(s0);

// ピクセルシェーダーの入力用
struct PSInput
{
    float4 Position     : SV_POSITION;  // 位置
    float4 Diffuse      : COLOR0;       // ディフューズ色
    float3 Specular     : COLOR1;       // スペキュラー色
};

float4 main(PSInput pin) : SV_TARGET
{
    pin.Diffuse.rgb += pin.Specular * pin.Diffuse.a;

    return pin.Diffuse;
}
