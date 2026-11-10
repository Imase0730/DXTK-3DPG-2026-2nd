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

// 頂点シェーダーの入力用
struct VSInput
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR;
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Position     : SV_POSITION;  // 位置
    float4 Diffuse      : COLOR0;       // ディフューズ色
    float3 Specular     : COLOR1;       // スペキュラー色
};

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換    
    vout.Position = mul(vin.Position, WorldViewProj);

    // 平行光源のディフューズ色
    float3 N = normalize(mul(vin.Normal, WorldInverseTranspose));
    float3 L = normalize(-LightDirection[0]);
    float dotL = dot(N, L);
    float zeroL = step(0, dotL);
    float3 diffuse = zeroL * dotL * LightDiffuseColor[0] * DiffuseColor.rgb + EmissiveColor;
    vout.Diffuse = float4(diffuse, DiffuseColor.a);
    
    // スペキュラー色
    float3 position_ws = mul(vin.Position, World).xyz;      // ワールド空間の頂点の位置
    float3 eyeVector = normalize(EyePosition - position_ws);
    float3 H = normalize(eyeVector - LightDirection[0]);    // ハーフベクトル
    float dotH = dot(N, H);
    float specularIntensity = pow(max(dotH, 0) * zeroL, SpecularPower) * dotL;
    vout.Specular = specularIntensity * LightSpecularColor[0] * SpecularColor;
    
    return vout;
}

