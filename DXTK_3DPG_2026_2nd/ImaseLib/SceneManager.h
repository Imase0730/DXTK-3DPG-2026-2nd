// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2026 トライデントコンピュータ専門学校
//--------------------------------------------------------------------------------------
// File: SceneManager.h
//
// シーンを管理するクラス
//
// Date: 2026.8.28
// Author: Suzuki Ryutaro
//--------------------------------------------------------------------------------------
#pragma once
#include <cassert>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>
#include "Transition/TransitionController.h"

namespace Imase
{
    // シーンマネージャーへのインターフェイス
    template <class TSceneKey> struct ISceneController
    {
        // デストラクタに=defaultを付ける理由
        // ムーブコンストラクタやコピーコンストラクタを明示的に定義した際、
        // デストラクタが暗黙的に生成されなくなる（または動作が変わる）のを防ぐ
        virtual ~ISceneController() = default;

        // シーン切り替え関数
        virtual void RequestSwitch(const TSceneKey& key, float duration = 0.0f) = 0;
        // シーンのプッシュ関数
        virtual void RequestPush(const TSceneKey& key, float duration = 0.0f) = 0;
        // シーンのポップ関数
        virtual void RequestPop(float duration = 0.0f) = 0;
    };

    // シーンの基底クラス
    template <class TSceneKey, class TContext> struct SceneBase
    {
        virtual ~SceneBase() = default;

        // 更新
        virtual void Update(ISceneController<TSceneKey>& sceneController, TContext& context) = 0;
        // 描画
        virtual void Render(TContext& gameContext) = 0;

        // 自シーンへ移行時に呼び出される関数
        virtual void OnEnter(TContext&)
        {
        }
        // 他シーンへ移行時に呼び出さる関数
        virtual void OnExit(TContext&)
        {
        }
        // 待機状態へ移行時に呼び出さる関数（Pushされた場合）
        virtual void OnSuspend(TContext&)
        {
        }
        // 待機状態から復帰時に呼び出さる関数（Popされた場合）
        virtual void OnResume(TContext&)
        {
        }

        // DirectXのデバイスロスト時に呼び出される関数
        virtual void OnDeviceLost(TContext&)
        {
        }
        // DirectXのデバイス復帰時に呼び出される関数
        virtual void OnDeviceRestored(TContext&)
        {
        }
        // ウインドウサイズ変更時に呼び出される関数
        virtual void OnWindowSizeChanged(TContext&)
        {
        }
    };

    template <class TSceneKey, class TContext> using ScenePtr = std::unique_ptr<SceneBase<TSceneKey, TContext>>;

    // シーンマネージャークラス
    template <class TSceneKey, class TContext> class SceneManager final : public ISceneController<TSceneKey>
    {
        // SceneFactoryは、対応するシーンをunique_ptrで返す関数
        using SceneFactory = std::function<ScenePtr<TSceneKey, TContext>()>;

        // シーン遷移の種類
        enum class SceneCommand
        {
            None,   // なし
            Switch, // シーンの切り替え
            Push,   // シーンのプッシュ
            Pop     // シーンのポップ
        };

        // シーン遷移リクエスト
        struct SceneRequest
        {
            SceneCommand command;
            std::optional<TSceneKey> sceneKey;
            float duration;
        };

        // シーン遷移の情報
        struct SceneTransition
        {
            SceneBase<TSceneKey, TContext>* fromScene;
            ScenePtr<TSceneKey, TContext> retiredScene;
        };

        // シーン生成テーブル
        std::unordered_map<TSceneKey, SceneFactory> m_sceneFactories;

        // 作成されたシーンのスタック
        std::vector<ScenePtr<TSceneKey, TContext>> m_sceneStack;

        // 次のシーン遷移リクエスト
        std::optional<SceneRequest> m_nextRequest = std::nullopt;

        // 進行中のシーン遷移情報
        std::optional<SceneTransition> m_activeTransition = std::nullopt;

        // シーン遷移コントローラー
        TransitionController m_transitionController;

    public:
        // コンストラクタ
        explicit SceneManager(ITransitionRenderer& renderer) : m_transitionController(renderer)
        {
        }

        // シーンの登録
        template <class TScene>
            requires std::derived_from<TScene, SceneBase<TSceneKey, TContext>> // <- SceneBaseを継承したクラスか？
        void RegisterScene(const TSceneKey& key)
        {
            auto [it, inserted] = m_sceneFactories.emplace(key, []() { return std::make_unique<TScene>(); });
            assert(inserted && "同じシーンキーが既に登録されています");
        }

        // 起動シーンの設定
        void SetFirstScene(const TSceneKey& key, TContext& context);

        // 更新
        void Update(TContext& context, float deltaTime);

        // 描画
        void Render(TContext& context);

        // シーン切り替え関数
        void RequestSwitch(const TSceneKey& key, float duration = 0.0f) override
        {
            SetRequest(SceneCommand::Switch, key, duration);
        }

        // シーンのプッシュ関数
        void RequestPush(const TSceneKey& key, float duration = 0.0f) override
        {
            SetRequest(SceneCommand::Push, key, duration);
        }

        // シーンのポップ関数
        void RequestPop(float duration = 0.0f) override
        {
            SetRequest(SceneCommand::Pop, std::nullopt, duration);
        }

        // DirectXのデバイスロスト時に呼び出される関数
        void OnDeviceLost(TContext& context)
        {
            ForEachScene([&](auto& scene) { scene.OnDeviceLost(context); });
        }

        // DirectXのデバイス復帰時にに呼び出される関数
        void OnDeviceRestored(TContext& context)
        {
            ForEachScene([&](auto& scene) { scene.OnDeviceRestored(context); });
        }

        // ウインドウサイズ変更時に呼び出される関数
        void OnWindowSizeChanged(TContext& context)
        {
            ForEachScene([&](auto& scene) { scene.OnWindowSizeChanged(context); });
        }

    private:
        // シーンの生成関数
        ScenePtr<TSceneKey, TContext> CreateScene(const TSceneKey& key);

        // シーン遷移のリクエストをする関数
        void SetRequest(SceneCommand command, std::optional<TSceneKey> sceneKey, float duration);

        // リクエストを適応する関数
        void ApplyRequest(TContext& context);

        void CompleteTransition(TContext& context);

        template <class Func> void ForEachScene(Func function)
        {
            for (auto& scene : m_sceneStack)
            {
                function(*scene);
            }
            if (m_activeTransition && m_activeTransition->retiredScene)
            {
                function(*m_activeTransition->retiredScene);
            }
        }
    };

    // 起動シーンの設定
    template <class TSceneKey, class TContext>
    void SceneManager<TSceneKey, TContext>::SetFirstScene(const TSceneKey& key, TContext& context)
    {
        assert(m_sceneStack.empty() && "SetFirstScene() は1回だけ呼び出すことができます");

        auto scene = CreateScene(key);
        assert(scene && "ファクトリがnullptrを返却しました");

        m_sceneStack.push_back(std::move(scene));
        m_sceneStack.back()->OnEnter(context);
    }

    // 更新
    template <class TSceneKey, class TContext>
    void SceneManager<TSceneKey, TContext>::Update(TContext& context, float deltaTime)
    {
        assert(!m_sceneStack.empty() && "シーンスタックが空です");
        assert(deltaTime >= 0.0f && "deltaTimeには0以上の値を指定してください");

        // シーン遷移中の場合は遷移の更新を行う
        if (m_activeTransition)
        {
            m_transitionController.Update(deltaTime);
            return;
        }

        m_sceneStack.back()->Update(*this, context);
        ApplyRequest(context);
    }

    // 描画
    template <class TSceneKey, class TContext> void SceneManager<TSceneKey, TContext>::Render(TContext& context)
    {
        assert(!m_sceneStack.empty() && "シーンスタックが空です");

        // 遷移中でなければ現在のシーンを描画する
        if (!m_activeTransition)
        {
            m_sceneStack.back()->Render(context);
            return;
        }

        // 遷移中であれば遷移の描画を行う
        const auto renderFrom = [&]() { m_activeTransition->fromScene->Render(context); };
        const auto renderTo = [&]() { m_sceneStack.back()->Render(context); };

        m_transitionController.Render(renderFrom, renderTo);
    }

    // シーンの生成関数
    template <class TSceneKey, class TContext>
    ScenePtr<TSceneKey, TContext> SceneManager<TSceneKey, TContext>::CreateScene(const TSceneKey& key)
    {
        auto it = m_sceneFactories.find(key);
        if (it == m_sceneFactories.end())
        {
            assert(false && "未登録のシーンキーです");
            return nullptr;
        }
        return it->second();
    }

    // シーン遷移のリクエストをする関数
    template <class TSceneKey, class TContext>
    void SceneManager<TSceneKey, TContext>::SetRequest(SceneCommand command, std::optional<TSceneKey> sceneKey,
                                                       float duration)
    {
        assert(!m_nextRequest && !m_activeTransition && "シーン遷移を重ねて要求することはできません");

        if (duration <= 0.0f)
        {
            duration = 0.0f;
        }

        m_nextRequest = SceneRequest{command, std::move(sceneKey), duration};
    }

    // リクエストを適応する関数
    template <class TSceneKey, class TContext> void SceneManager<TSceneKey, TContext>::ApplyRequest(TContext& context)
    {
        if (!m_nextRequest)
        {
            return;
        }

        SceneRequest request = std::move(*m_nextRequest);
        m_nextRequest.reset();

        SceneBase<TSceneKey, TContext>* fromScene = m_sceneStack.back().get();
        ScenePtr<TSceneKey, TContext> retiredScene;

        switch (request.command)
        {
            case SceneCommand::Switch:
            {
                assert(request.sceneKey);

                retiredScene = std::move(m_sceneStack.back());

                m_sceneStack.back() = CreateScene(*request.sceneKey);
                m_sceneStack.back()->OnEnter(context);
                break;
            }

            case SceneCommand::Push:
                assert(request.sceneKey);
                m_sceneStack.back()->OnSuspend(context);
                m_sceneStack.push_back(CreateScene(*request.sceneKey));
                m_sceneStack.back()->OnEnter(context);
                break;

            case SceneCommand::Pop:
            {
                assert(m_sceneStack.size() > 1);

                retiredScene = std::move(m_sceneStack.back());

                m_sceneStack.pop_back();
                m_sceneStack.back()->OnResume(context);
                break;
            }
        }

        m_activeTransition = SceneTransition{fromScene, std::move(retiredScene)};
        m_transitionController.Start(request.duration, [this, &context]() { CompleteTransition(context); });
    }
    template <class TSceneKey, class TContext>
    void SceneManager<TSceneKey, TContext>::CompleteTransition(TContext& context)
    {
        assert(m_activeTransition);

        if (m_activeTransition->retiredScene)
        {
            m_activeTransition->retiredScene->OnExit(context);
        }

        m_activeTransition.reset();
    }
}
