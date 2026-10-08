// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#include "pch.h"
#include "MaskTransitionResources.h"
#include <stdexcept>
#include "DirectXTK_Utilities/ReadData.h"

namespace Imase
{
    MaskTransitionResources::MaskTransitionResources(ID3D11Device* device)
    {
        CreatePixelShader(device);
        CreateConstantBuffer(device);
    }

    void MaskTransitionResources::CreatePixelShader(ID3D11Device* device)
    {
        const auto psByteCode = DX::ReadData(L"Resources/Shaders/MaskTransitionPS.cso");

        if (FAILED(
                device->CreatePixelShader(psByteCode.data(), psByteCode.size(), nullptr, m_pixelShader.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create mask transition pixel shader");
        }
    }

    void MaskTransitionResources::CreateConstantBuffer(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(MaskTransitionConstants);
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        if (FAILED(device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.GetAddressOf())))
        {
            throw std::runtime_error("failed to create mask transition constant buffer.");
        }
    }
}