//--------------------------------------------------------------------------------------
// File: SceneId.h
//
// シーンID
//
// Date: 2026.9.16
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include <cstdint>

enum class SceneId : uint8_t
{
    BaseScene,
    TitleScene,
    GameScene,
    ShaderTestScene,
};
