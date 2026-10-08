// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#include "pch.h"
#include "TransitionController.h"
#include <algorithm>
#include <cassert>
#include <utility>

namespace Imase
{
    void TransitionController::Start(float duration, std::function<void()> onCompleted)
    {
        assert(!IsActive() && "トランジションはすでに実行中です");

        m_elapsedTime = 0.0f;
        m_duration = duration;
        m_onCompleted = std::move(onCompleted);
    }

    void TransitionController::Update(float deltaTime)
    {
        assert(deltaTime >= 0.0f);

        m_elapsedTime += deltaTime;

        if (m_elapsedTime < m_duration)
        {
            return;
        }

        auto onCompleted = std::move(m_onCompleted);
        m_onCompleted = {};

        if (onCompleted)
        {
            onCompleted();
        }
    }

    void TransitionController::Render(const std::function<void()>& renderFrom,
                                      const std::function<void()>& renderTo) const
    {
        const float progress = std::min(m_elapsedTime / m_duration, 1.0f);
        m_renderer.Render(progress, renderFrom, renderTo);
    }

    bool TransitionController::IsActive() const
    {
        return m_elapsedTime < m_duration;
    }
}