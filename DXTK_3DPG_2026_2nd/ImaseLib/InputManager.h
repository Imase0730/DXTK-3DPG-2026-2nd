//--------------------------------------------------------------------------------------
// File: InputManager.h
//
// キー入力マネージャー
//
// Usage: BindActionでアクション名と対応するキーを登録して使用してください。
//
// Date: 2026.7.5
// Author: Hideyasu Imase
//
//--------------------------------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace Imase
{
    class InputManager
    {
    public:
        
        // 修飾キーのフラグ定義
        enum Modifier
        {
            None = 0,
            Shift = 1 << 0,
            Ctrl = 1 << 1,
            Alt = 1 << 2
        };

    public:

        // コンストラクタ
        InputManager(DirectX::Keyboard::KeyboardStateTracker& tracker);

        // 有効・無効を切り替える関数
        void SetEnabled(bool enable)
        {
            m_isEnabled = enable;
        }

        // 拡張したBind関数（第3引数で修飾キーを指定、デフォルトは None）
        void BindAction(const std::string& actionName, DirectX::Keyboard::Keys key, int modifiers = Modifier::None);

        bool IsPressed(const std::string& actionName) const;
        bool IsTriggered(const std::string& actionName) const;
        bool IsReleased(const std::string& actionName) const;

    private:

        // 操作可能かどうかのフラグ
        bool m_isEnabled = true;

        using ActionId = std::size_t;

        // アクション名からハッシュを求める関数
        ActionId GetActionId(const std::string& actionName) const
        {
            return std::hash<std::string>{}(actionName);
        }

        // 1つのキーと修飾キーの組み合わせを保持する構造体
        struct BindingInfo
        {
            DirectX::Keyboard::Keys mainKey; // 例: Keys::Up
            int modifiers;                   // 例: Modifier::Shift
        };

        const DirectX::Keyboard::KeyboardStateTracker& m_tracker;

        // アクション名（ハッシュ）に、BindingInfoのリストを紐付ける
        std::unordered_map<ActionId, std::vector<BindingInfo>> m_actionBindings;

        // 現在のフレームで修飾キーの条件が満たされているかチェックするヘルパー関数
        bool CheckModifiers(int requiredModifiers, const DirectX::Keyboard::State& state) const;

        // 有効・無効を調べる関数
        bool IsEnabled() const
        {
            return m_isEnabled;
        }
 
    };
}

