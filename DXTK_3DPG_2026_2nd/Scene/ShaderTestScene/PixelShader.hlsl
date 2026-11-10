// テクスチャ
Texture2D Texture : register(t0);

// サンプラー
SamplerState Sampler : register(s0);

// 定数バッファ
cbuffer Parameters : register(b0)
{
    float4 DiffuseColor             : packoffset(c0);
    float3 EmissiveColor            : packoffset(c1);
    float3 SpecularColor            : packoffset(c2);
    float SpecularPower             : packoffset(c2.w);

    float3 LightDirection[3]        : packoffset(c3);
    float3 LightDiffuseColor[3]     : packoffset(c6);
    float3 LightSpecularColor[3]    : packoffset(c9);

    float3 EyePosition              : packoffset(c12);

    float3 FogColor                 : packoffset(c13);
    float4 FogVector                : packoffset(c14);

    float4x4 World                  : packoffset(c15);
    float3x3 WorldInverseTranspose  : packoffset(c19);
    float4x4 WorldViewProj          : packoffset(c22);
};

// ピクセルシェーダーの入力用
struct PSInput
{
    float4 Position     : SV_POSITION;  // 位置
    float3 PositionWS   : TEXCOORD0;    // ワールド空間の位置
    float3 NormalWS     : TEXCOORD1;    // ワールド空間の法線
};

float4 main(PSInput pin) : SV_TARGET
{
    // 平行光源のディフューズ色
    float3 N = normalize(pin.NormalWS);
    float3 L = normalize(-LightDirection[0]);
    float dotL = dot(N, L);
    float zeroL = step(0, dotL);
    float3 diffuse = zeroL * dotL * LightDiffuseColor[0] * DiffuseColor.rgb + EmissiveColor;
    
    // スペキュラー色
    float3 eyeVector = normalize(EyePosition - pin.PositionWS);
    float3 H = normalize(eyeVector - LightDirection[0]); // ハーフベクトル
    float dotH = dot(N, H);
    float specularIntensity = pow(max(dotH, 0) * zeroL, SpecularPower) * dotL;
    float3 specular = specularIntensity * LightSpecularColor[0] * SpecularColor;

    float4 color = float4(diffuse, DiffuseColor.a);
    // スペキュラー色を加える
    color.rgb += specular * DiffuseColor.a;

    return color;
}
