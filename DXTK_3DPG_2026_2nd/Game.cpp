//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include <Scene/BaseScene/BaseScene.h>
#include <Scene/TitleScene/TitleScene.h>
#include <Scene/GameScene/GameScene.h>
#include <Scene/ShaderTestScene//ShaderTestScene.h>

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    // シーンマネージャ作成
    m_sceneManager = std::make_unique<Imase::SceneManager<SceneId, GameContext>>(*m_transitionRenderer);

    // シーンの登録
    m_sceneManager->RegisterScene<BaseScene>(SceneId::BaseScene);
    m_sceneManager->RegisterScene<TitleScene>(SceneId::TitleScene);
    m_sceneManager->RegisterScene<GameScene>(SceneId::GameScene);
    m_sceneManager->RegisterScene<ShaderTestScene>(SceneId::ShaderTestScene);

    // ゲームコンテキストの設定
    m_gameContext.emplace(
        m_timer,                 // <- StepTimer
        *m_deviceResources,      // <- DeviceResources
        m_keyboardTracker,       // <- KeyboardTracker
        m_mouseButtonTracker,    // <- MouseButtonTracker
        *m_states,               // <- CommonStates
        *m_debugRenderer         // <- DebugRenderer
    );

    // 起動シーンの設定
    m_sceneManager->SetFirstScene(SceneId::ShaderTestScene, *m_gameContext);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.

    // キーボードトラッカーの更新
    auto keyboard = Keyboard::Get().GetState();
    m_keyboardTracker.Update(keyboard);

    // マウスボタントラッカーの更新
    auto mouse = Mouse::Get().GetState();
    m_mouseButtonTracker.Update(mouse);

    Imase::DXTK_ImGui::Update(m_gameContext->deviceResources.GetWindow());

    // シーンの更新
    m_sceneManager->Update(*m_gameContext, elapsedTime);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.

    // シーンの描画
    m_sceneManager->Render(*m_gameContext);

    // デバッグ関連の文字列などを描画
    m_debugRenderer->Render(context, m_states.get());

    Imase::DXTK_ImGui::Render();

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    const auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Initialize device dependent objects here (independent of window size).

    int w, h;
    GetDefaultSize(w, h);
    Imase::DXTK_ImGui::Initialize(m_deviceResources->GetWindow(), m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(), w, h);

    // コモンステートの作成
    m_states = std::make_unique<CommonStates>(device);

    // マスクトランジション共通リソースの作成
    m_maskTransitionResources = std::make_unique<Imase::MaskTransitionResources>(device);
 
    // マスクテクスチャの読み込み
    DX::ThrowIfFailed(
        DirectX::CreateDDSTextureFromFile(device, L"Resources/Textures/Mask01.dds", nullptr, m_maskTexture.ReleaseAndGetAddressOf())
    );
    
    // マスクトランジションエフェクトの作成
    m_maskTransition = std::make_unique<Imase::MaskTransitionEffect>(*m_maskTransitionResources, m_maskTexture.Get());

    // シーン遷移レンダラーの作成
    m_transitionRenderer = std::make_unique<Imase::TransitionRenderer>(*m_deviceResources);
   
    // シーン遷移レンダラーにマスク遷移エフェクトを設定
    m_transitionRenderer->SetEffect(*m_maskTransition);

    // デバッグ用の描画セットの作成
    m_debugRenderer = std::make_unique<Imase::DebugRenderer>(device, context, L"Resources/Font/SegoeUI_18.spritefont");
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
