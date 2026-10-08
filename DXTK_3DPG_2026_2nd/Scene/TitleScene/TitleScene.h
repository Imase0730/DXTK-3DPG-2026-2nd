#pragma once
#include <ImaseLib/SceneManager.h>
#include <GameContext.h>
#include "../SceneId.h"

class TitleScene : public Imase::SceneBase<SceneId, GameContext>
{
public:
    // 更新
    void Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext) override;

    // 描画
    void Render(GameContext& gameContext) override;

    // シーン切り替え時に呼び出される関数
    void OnEnter(GameContext& gameContext) override;

private:
    // スプライトバッチ
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

    // テクスチャハンドル
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

    // 位置
    DirectX::SimpleMath::Vector2 m_position = {400.0f, 400.0f};

    // 移動の速さ(dot/s)
    static constexpr float Speed = 600.0f;

    // スケール
    float m_scale = 4.0f;

    // 拡大縮小する速さ
    float m_speed = 0.0f;

    // ばねの強さ
    static constexpr float K = 0.08f;

    // 摩擦係数（1.0fで減衰なし、小さい程すぐ止まる）
    static constexpr float FRICTION = 0.97f;
};