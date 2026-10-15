// 定数バッファ
cbuffer PerFrameCB : register(b0)
{
    float4x4 wvp; // ワールド行列×ビュー行列×プロジェクション行列
};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position : POSITION; // 位置
    float2 TexCoord : TEXCOORD; // テクスチャ座標
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Position : SV_POSITION;  // 位置
    float2 TexCoord : TEXCOORD;     // テクスチャ座標
};

VSOutput main(VSInput vin)
{
    VSOutput vout;
    
    vout.Position = mul(wvp, float4(vin.Position, 1.0f));
    vout.TexCoord = vin.TexCoord;
    
    return vout;
}

