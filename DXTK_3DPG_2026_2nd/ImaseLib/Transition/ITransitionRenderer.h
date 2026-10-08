// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
#pragma once
#include <functional>

namespace Imase
{
    struct ITransitionRenderer
    {
        virtual ~ITransitionRenderer() = default;

        virtual void Render(float progress, const std::function<void()>& renderFrom,
                            const std::function<void()>& renderTo) = 0;
    };
}