//--------------------------------------------------------------------------------------
// File: InputManager.cpp
//
// キー入力マネージャー
//
// Usage: BindActionでアクション名と対応するキーを登録して使用してください。
//
// Date: 2026.7.5
// Author: Hideyasu Imase
//
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "InputManager.h"

using namespace DirectX;
using namespace Imase;

// コンストラクタ
InputManager::InputManager(Keyboard::KeyboardStateTracker& tracker)
    : m_tracker(tracker)
{
}

// 拡張したBind関数（第3引数で修飾キーを指定、デフォルトは None）
void InputManager::BindAction(const std::string& actionName, Keyboard::Keys key, int modifiers)
{
    ActionId id = GetActionId(actionName);
    m_actionBindings[id].push_back({key, modifiers});
}

// 修飾キーの状態が一致しているか厳密にチェックする
bool InputManager::CheckModifiers(int requiredModifiers, const Keyboard::State& state) const
{
    // 現在の実際のキーボードの修飾キー状態をビットフラグにする
    int currentModifiers = Modifier::None;
    if (state.IsKeyDown(Keyboard::Keys::LeftShift) || state.IsKeyDown(Keyboard::Keys::RightShift))
    {
        currentModifiers |= Modifier::Shift;
    }
    if (state.IsKeyDown(Keyboard::Keys::LeftControl) || state.IsKeyDown(Keyboard::Keys::RightControl))
    {
        currentModifiers |= Modifier::Ctrl;
    }
    if (state.IsKeyDown(Keyboard::Keys::LeftAlt) || state.IsKeyDown(Keyboard::Keys::RightAlt))
    {
        currentModifiers |= Modifier::Alt;
    }

    // 要求された修飾キーと、実際の状態が完全に一致しているか（排他チェック）
    return currentModifiers == requiredModifiers;
}

bool InputManager::IsPressed(const std::string& actionName) const
{
    if (!m_isEnabled)
        return false; // 無効なら常にfalseを返す

    auto it = m_actionBindings.find(GetActionId(actionName));
    if (it == m_actionBindings.end())
        return false;

    for (const auto& binding : it->second)
    {
        // メインキーが押されており、かつ修飾キーの条件が完全に一致しているか
        if (m_tracker.lastState.IsKeyDown(binding.mainKey) && CheckModifiers(binding.modifiers, m_tracker.lastState))
        {
            return true;
        }
    }
    return false;
}

bool InputManager::IsTriggered(const std::string& actionName) const
{
    if (!m_isEnabled)
        return false; // 無効なら常にfalseを返す

    auto it = m_actionBindings.find(GetActionId(actionName));
    if (it == m_actionBindings.end())
        return false;

    for (const auto& binding : it->second)
    {
        // メインキーが「今押された瞬間」で、かつ修飾キーの条件が一致しているか
        if (m_tracker.IsKeyPressed(binding.mainKey) && CheckModifiers(binding.modifiers, m_tracker.lastState))
        {
            return true;
        }
    }
    return false;
}

bool InputManager::IsReleased(const std::string& actionName) const
{
    if (!m_isEnabled)
        return false; // 無効なら常にfalseを返す

    auto it = m_actionBindings.find(GetActionId(actionName));
    if (it == m_actionBindings.end())
        return false;

    for (const auto& binding : it->second)
    {
        // メインキーが「今離された瞬間」だったか
        if (m_tracker.IsKeyReleased(binding.mainKey))
        {
            return true;
        }
    }
    return false;
}
