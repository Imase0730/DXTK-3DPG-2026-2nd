#include "pch.h"
#include "TitleScene.h"

using namespace DirectX;

void TitleScene::Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext)
{
    Imase::DebugRenderer& debugRenderer = gameContext.debugRenderer;

    auto& tracker = gameContext.keyboardTracker;

    if (tracker.pressed.A)
    {
        sceneController.RequestSwitch(SceneId::GameScene, 2.0f);
    }
    debugRenderer.DrawText({0.0f, 0.0f}, L"TitleScene");

    // 経過時間を取得する
    float elapsedTime = static_cast<float>(gameContext.timer.GetElapsedSeconds());

    // 移動量
    float move = Speed * elapsedTime;

    // キーボードステートを取得する
    auto kb = Keyboard::Get().GetState();

    // 上キーが押された
    if (kb.Up)
    {
        m_position.y -= move;
    }
    // 下キーが押された
    if (kb.Down)
    {
        m_position.y += move;
    }
    // 左キーが押された
    if (kb.Left)
    {
        m_position.x -= move;
    }
    // 右キーが押された
    if (kb.Right)
    {
        m_position.x += move;
    }

    // マウスステートを取得する
    auto mouse = Mouse::Get().GetState();

    // マウスの左ボタンが押された
    if (mouse.leftButton)
    {
        m_scale += 0.01f;
    }
    else
    {
        // ----- ばねの動きでスケールを変化させる ----- //

        // 1.力（加速度）を計算：（目標値 - 現在値）* 剛性（ばねの強さ）
        float force = (4.0f - m_scale) * K;

        // 2.速さ更新（摩擦を掛ける）
        m_speed += force;
        m_speed *= FRICTION;

        // 3.スケールを更新する
        m_scale += m_speed;
    }

    debugRenderer.DrawText({0.0f, 0.0f}, L"TitleScene");
}

void TitleScene::Render(GameContext& gameContext)
{
    m_spriteBatch->Begin(SpriteSortMode_Deferred,                     // 通常のソートモード
                         gameContext.commonStates.NonPremultiplied(), // 半透明
                         gameContext.commonStates.PointWrap()         // テクスチャサンプラー
    );

    // テクスチャ上の矩形
    RECT srcRect = {0, 0, 32, 32};

    // ここで描画
    m_spriteBatch->Draw(m_texture.Get(), m_position, &srcRect, Colors::White, 0.0f, SimpleMath::Vector2(16.0f, 16.0f),
                        m_scale);

    m_spriteBatch->End();
}

void TitleScene::OnEnter(GameContext& gameContext)
{ // DirectX3Dのデバイスを取得する
    auto device = gameContext.deviceResources.GetD3DDevice();

    // DirectX3Dのデバイスコンテキストを取得する
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // スプライトバッチの作成
    m_spriteBatch = std::make_unique<SpriteBatch>(context);

    // テクスチャの読み込み
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Resources/Textures/ShootingGame.dds", nullptr,
                                               m_texture.ReleaseAndGetAddressOf()));
}
