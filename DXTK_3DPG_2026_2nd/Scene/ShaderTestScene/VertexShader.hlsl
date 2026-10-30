// 定数バッファ
cbuffer PerFrameCB : register(b0)
{
    float4x4 WorldViewProjection;   // ワールド行列×ビュー行列×プロジェクション行列
    float4x4 WorldInverseTranspose; // ワールド行列の逆転置行列

    float4 LightDirection;          // ライトの方向
    float4 LightDiffuseColor;       // ライトのディフューズ色
    float4 LightAmbientColor;       // ライトのアンビエント色

    float4 DiffuseColor;            // マテリアルのディフューズ色
    float4 AmbientColor;            // マテリアルのアンビエント色
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
    float4 Position : SV_POSITION;  // 位置
    float4 Diffuse  : COLOR0;       // ディフューズ色
    float2 TexCoord : TEXCOORD;     // テクスチャ座標
};

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換    
    vout.Position = mul(WorldViewProjection, float4(vin.Position, 1.0f));

    float4 diffuse = DiffuseColor * LightDiffuseColor;
    float4 ambient = AmbientColor * LightAmbientColor;
    
    // ライトの色
    float3 N = normalize(mul((float3x3)WorldInverseTranspose, vin.Normal));
    float3 L = normalize(-LightDirection.xyz);
    vout.Diffuse = diffuse * saturate(dot(N, L)) + ambient;
    
    // テクスチャ座標
    vout.TexCoord = vin.TexCoord;
    
    return vout;
}

