// 定数バッファ
cbuffer PerFrameCB : register(b0)
{
    float4x4 WorldViewProjection;   // ワールド行列×ビュー行列×プロジェクション行列
    float4x4 WorldInverseTranspose; // ワールド行列の逆転置行列
    float4 LightDirection;          // ライトの方向
};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position : POSITION; // 位置
    float3 Normal   : NORMAL;   // 法線
    float2 TexCoord : TEXCOORD; // テクスチャ座標
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Position     : SV_POSITION;  // 位置
    float4 LightColor   : LIGHTCOLOR;   // ライトの色
    float2 TexCoord     : TEXCOORD;     // テクスチャ座標
};

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換    
    vout.Position = mul(WorldViewProjection, float4(vin.Position, 1.0f));

    // ライトの色
    float3 N = normalize(mul((float3x3)WorldInverseTranspose, vin.Normal));
    float3 L = normalize(-LightDirection.xyz);
    vout.LightColor = float4(1.0f, 1.0f, 1.0f, 1.0f) * saturate(dot(N, L));
    
    // テクスチャ座標
    vout.TexCoord = vin.TexCoord;
    
    return vout;
}

