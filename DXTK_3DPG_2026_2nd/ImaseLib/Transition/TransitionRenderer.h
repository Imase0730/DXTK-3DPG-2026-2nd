// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#pragma once
#include "ITransitionRenderer.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <d3d11.h>
#include <wrl/client.h>
#include "DeviceResources.h"
#include "ITransitionEffect.h"
#include "DirectXTK_Utilities/RenderTexture.h"

namespace Imase
{
    class TransitionRenderer final : public ITransitionRenderer
    {
    public:
        TransitionRenderer(DX::DeviceResources& deviceResources);

        void Render(float progress, const std::function<void()>& renderFrom,
                    const std::function<void()>& renderTo) override;

        void SetEffect(ITransitionEffect& effect);
        void OnWindowSizeChanged();
        void OnDeviceLost();
        void OnDeviceRestored();

    private:
        void CreateVertexShader(ID3D11Device* device);
        void CreateSamplerState(ID3D11Device* device);
        void CreateDepthStencilState(ID3D11Device* device);

        DX::DeviceResources& m_deviceResources;
        DX::RenderTexture m_fromTexture;
        DX::RenderTexture m_toTexture;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
        ITransitionEffect* m_effect;
    };
}