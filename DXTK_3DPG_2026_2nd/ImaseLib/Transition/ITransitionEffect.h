// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#pragma once
#include <d3d11.h>

namespace Imase
{
    struct ITransitionEffect
    {
        virtual ~ITransitionEffect() = default;

        virtual void Apply(ID3D11DeviceContext* context, ID3D11ShaderResourceView* fromTexture,
                           ID3D11ShaderResourceView* toTexture, float progress) = 0;
    };
}