// 定数バッファ
cbuffer PerFrameCB : register(b1)
{
    float4 color;
};

// テクスチャ
Texture2D Texture : register(t0);

// サンプラー
SamplerState Sampler : register(s0);

// ピクセルシェーダーの入力用
struct PSInput
{
    float4 Position : SV_POSITION;  // 位置
    float2 TexCoord : TEXCOORD;     // テクスチャ座標
};

float4 main(PSInput pin) : SV_TARGET
{
    return Texture.Sample(Sampler, pin.TexCoord) * color;
}
