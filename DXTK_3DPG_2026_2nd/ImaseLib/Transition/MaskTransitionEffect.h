// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#pragma once
#include "ITransitionEffect.h"
#include <d3d11.h>
#include <wrl/client.h>
#include "MaskTransitionResources.h"

namespace Imase
{
    // マスク画像を使用したトランジション効果を実現するクラス
    class MaskTransitionEffect final : public ITransitionEffect
    {
    public:
        MaskTransitionEffect(MaskTransitionResources& resources, ID3D11ShaderResourceView* maskTexture);

        MaskTransitionEffect(const MaskTransitionEffect&) = delete;
        MaskTransitionEffect& operator=(const MaskTransitionEffect&) = delete;

        void Apply(ID3D11DeviceContext* context, ID3D11ShaderResourceView* fromTexture,
                   ID3D11ShaderResourceView* toTexture, float progress) override;

    private:
        MaskTransitionResources& m_resources;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_maskTexture{};
        MaskTransitionConstants m_constants{};
    };
}