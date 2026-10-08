// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#include "pch.h"
#include "MaskTransitionEffect.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace Imase
{
    MaskTransitionEffect::MaskTransitionEffect(MaskTransitionResources& resources,
                                               ID3D11ShaderResourceView* maskTexture)
        : m_resources(resources), m_maskTexture(maskTexture)
    {
    }

    void MaskTransitionEffect::Apply(ID3D11DeviceContext* context, ID3D11ShaderResourceView* fromTexture,
                                     ID3D11ShaderResourceView* toTexture, float progress)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource{};

        auto constantBuffer = m_resources.GetConstantBuffer();

        if (FAILED(context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            throw std::runtime_error("Failed to map constant buffer.");
        }

        m_constants.progress = std::clamp(progress, 0.0f, 1.0f);
        std::memcpy(mappedResource.pData, &m_constants, sizeof(m_constants));

        context->Unmap(constantBuffer, 0);

        ID3D11ShaderResourceView* shaderResources[] = {
            fromTexture,
            toTexture,
            m_maskTexture.Get(),
        };

        ID3D11Buffer* constantBuffers[] = {
            constantBuffer,
        };

        context->PSSetShader(m_resources.GetPixelShader(), nullptr, 0);
        context->PSSetShaderResources(0, 3, shaderResources);
        context->PSSetConstantBuffers(0, 1, constantBuffers);
    }
}