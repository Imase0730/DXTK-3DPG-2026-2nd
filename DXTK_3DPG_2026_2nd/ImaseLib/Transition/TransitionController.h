// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#pragma once
#include <functional>
#include "ITransitionRenderer.h"

namespace Imase
{
    class TransitionController final
    {
    public:
        explicit TransitionController(ITransitionRenderer& renderer) : m_renderer(renderer)
        {
        }

        void Start(float duration, std::function<void()> onCompleted);
        void Update(float deltaTime);
        void Render(const std::function<void()>& renderFrom, const std::function<void()>& renderTo) const;
        bool IsActive() const;

    private:
        ITransitionRenderer& m_renderer;
        float m_elapsedTime = 0.0f;
        float m_duration = 0.0f;
        std::function<void()> m_onCompleted;
    };
}
