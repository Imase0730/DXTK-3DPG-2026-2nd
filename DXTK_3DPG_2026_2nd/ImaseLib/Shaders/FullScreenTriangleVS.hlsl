// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#include "FullScreen.hlsli"

FullScreenVSOutput main(uint vertexId : SV_VertexID)
{
    static const float2 positions[] =
    {
        float2(-1.0f, 1.0f),
        float2(3.0f, 1.0f),
        float2(-1.0f, -3.0f),
    };

    static const float2 texCoords[] =
    {
        float2(0.0f, 0.0f),
        float2(2.0f, 0.0f),
        float2(0.0f, 2.0f),
    };

    FullScreenVSOutput output;
    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.texcoord = texCoords[vertexId];

    return output;
}