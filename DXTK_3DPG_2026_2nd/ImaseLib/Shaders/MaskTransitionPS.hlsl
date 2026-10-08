// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
//
// マスク画像を使用したマスクトランジション描画シェーダーのサンプルです。
// MaskTexture のr成分が Progress の値に達していないピクセルは FromTexture の色を、それ以外のピクセルは ToTexture の色を描画します。
//
// Progress の値を 0.0 から 1.0 の範囲で変化させることでトランジションの進行を表現します。
//
// FromTexture : 切り替え前の画面
// ToTexture   : 切り替え後の画面
// MaskTexture : ピクセルごとの切り替えタイミングを表すマスク画像
#include "FullScreen.hlsli"

cbuffer MaskTransitionConstants : register(b0)
{
    float Progress;
    float3 Padding;
};

Texture2D FromTexture : register(t0);
Texture2D ToTexture : register(t1);
Texture2D MaskTexture : register(t2);

SamplerState TextureSampler : register(s0);

float4 main(FullScreenVSOutput input) : SV_TARGET
{
    const float4 fromColor = FromTexture.Sample(TextureSampler, input.texcoord);
    const float4 toColor = ToTexture.Sample(TextureSampler, input.texcoord);
    const float mask = MaskTexture.Sample(TextureSampler, input.texcoord).r;
    
    // step(y, x) は x が y 以上のとき 1.0 を返し、それ以外のとき 0.0 を返す関数です。
    const float toRate = step(mask, Progress);

    return lerp(fromColor, toColor, toRate);
}