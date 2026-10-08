// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#pragma once
#include <d3d11.h>
#include <wrl/client.h>

namespace Imase
{
    struct MaskTransitionConstants
    {
        float progress = 0.0f;
        float padding[3] = {};
    };

    static_assert(sizeof(MaskTransitionConstants) % 16 == 0, "MaskTransitionConstants must be 16-byte aligned.");

    class MaskTransitionResources final
    {
    public:
        explicit MaskTransitionResources(ID3D11Device* device);

        ID3D11PixelShader* GetPixelShader() const noexcept
        {
            return m_pixelShader.Get();
        }

        ID3D11Buffer* GetConstantBuffer() const noexcept
        {
            return m_constantBuffer.Get();
        }

    private:
        void CreatePixelShader(ID3D11Device* device);
        void CreateConstantBuffer(ID3D11Device* device);

        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    };
}