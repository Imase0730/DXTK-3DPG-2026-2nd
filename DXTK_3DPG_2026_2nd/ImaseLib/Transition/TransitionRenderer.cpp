// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#include "pch.h"
#include "TransitionRenderer.h"
#include "DirectXTK_Utilities/ReadData.h"

namespace Imase
{
    TransitionRenderer::TransitionRenderer(DX::DeviceResources& deviceResources)
        : m_deviceResources(deviceResources), m_fromTexture(deviceResources.GetBackBufferFormat()),
          m_toTexture(deviceResources.GetBackBufferFormat())
    {
        auto device = m_deviceResources.GetD3DDevice();

        m_fromTexture.SetDevice(device);
        m_toTexture.SetDevice(device);

        CreateVertexShader(device);
        CreateSamplerState(device);
        CreateDepthStencilState(device);

        OnWindowSizeChanged();
    }

    void TransitionRenderer::Render(float progress, const std::function<void()>& renderFrom,
                                    const std::function<void()>& renderTo)
    {
        auto context = m_deviceResources.GetD3DDeviceContext();
        auto depthStencil = m_deviceResources.GetDepthStencilView();
        const auto outputSize = m_deviceResources.GetOutputSize();

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(outputSize.right - outputSize.left);
        viewport.Height = static_cast<float>(outputSize.bottom - outputSize.top);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        context->RSSetViewports(1, &viewport);

        constexpr float clearColor[] = {0, 0, 0, 1};

        auto fromRTV = m_fromTexture.GetRenderTargetView();
        context->OMSetRenderTargets(1, &fromRTV, depthStencil);
        context->ClearRenderTargetView(fromRTV, clearColor);
        context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        renderFrom();

        auto toRTV = m_toTexture.GetRenderTargetView();
        context->OMSetRenderTargets(1, &toRTV, depthStencil);
        context->ClearRenderTargetView(toRTV, clearColor);
        context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        renderTo();

        context->RSSetViewports(1, &viewport);
        auto backBufferRTV = m_deviceResources.GetRenderTargetView();
        context->OMSetRenderTargets(1, &backBufferRTV, nullptr);
        context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

        ID3D11Buffer* nullVertexBuffers[] = {nullptr};
        UINT strides[] = {0};
        UINT offsets[] = {0};

        context->IASetVertexBuffers(0, 1, nullVertexBuffers, strides, offsets);
        context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

        context->GSSetShader(nullptr, nullptr, 0);

        context->RSSetState(nullptr);

        ID3D11SamplerState* samplers[] = {m_samplerState.Get()};
        context->PSSetSamplers(0, 1, samplers);

        context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
        context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

        m_effect->Apply(context, m_fromTexture.GetShaderResourceView(), m_toTexture.GetShaderResourceView(), progress);

        context->Draw(3, 0);

        ID3D11ShaderResourceView* nullShaderResources[] = {
            nullptr,
            nullptr,
            nullptr
        };

        context->PSSetShaderResources(0, 3, nullShaderResources);
    }

    void TransitionRenderer::SetEffect(ITransitionEffect& effect)
    {
        m_effect = &effect;
    }

    void TransitionRenderer::OnWindowSizeChanged()
    {
        const RECT outputSize = m_deviceResources.GetOutputSize();

        m_fromTexture.SetWindow(outputSize);
        m_toTexture.SetWindow(outputSize);
    }

    void TransitionRenderer::OnDeviceLost()
    {
        m_fromTexture.ReleaseDevice();
        m_toTexture.ReleaseDevice();
    }

    void TransitionRenderer::OnDeviceRestored()
    {
        auto* device = m_deviceResources.GetD3DDevice();

        m_fromTexture.SetDevice(device);
        m_toTexture.SetDevice(device);

        OnWindowSizeChanged();
    }

    void TransitionRenderer::CreateVertexShader(ID3D11Device* device)
    {
        const auto vsByteCode = DX::ReadData(L"Resources/Shaders/FullScreenTriangleVS.cso");

        if (FAILED(device->CreateVertexShader(vsByteCode.data(), vsByteCode.size(), nullptr,
                                              m_vertexShader.GetAddressOf())))
        {
            throw std::runtime_error("Failed to create full screen triangle vertex shader.");
        }
    }

    void TransitionRenderer::CreateSamplerState(ID3D11Device* device)
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MaxLOD = D3D11_FLOAT32_MAX;

        if (FAILED(device->CreateSamplerState(&desc, m_samplerState.GetAddressOf())))
        {
            throw std::runtime_error("failed to create transition effect sampler state.");
        }
    }

    void TransitionRenderer::CreateDepthStencilState(ID3D11Device* device)
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = FALSE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.StencilEnable = FALSE;

        if (FAILED(device->CreateDepthStencilState(&desc, m_depthStencilState.GetAddressOf())))
        {
            throw std::runtime_error("failed to create transition effect depth stencil state.");
        }
    }
}
