// 定数バッファ
cbuffer PerFrameCB : register(b0)
{
    float4x4 wvp; // ワールド行列×ビュー行列×プロジェクション行列
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
    return mul(wvp, pos);
}
