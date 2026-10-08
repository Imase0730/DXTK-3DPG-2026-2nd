//--------------------------------------------------------------------------------------
// File: DirectXTK_ImGui.cpp
//
// ImGuiの処理をまとめたクラス
//
// Date: 2025.10.14
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"

#include "DirectXTK_ImGui.h"

bool Imase::DXTK_ImGui::m_isInitialized = false;
int Imase::DXTK_ImGui::m_screenWidth = 0;
int Imase::DXTK_ImGui::m_screenHeight = 0;

// 初期化関数
void Imase::DXTK_ImGui::Initialize(
    HWND hWnd,
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    int screenWidth,
    int screenHeight
)
{
    // リセット処理
    Imase::DXTK_ImGui::Reset();

    // 初期化済み
    m_isInitialized = true;

    // ゲーム画面の解像度
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    //  バージョンの確認
    IMGUI_CHECKVERSION();

    //  コンテキストの作成
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // キーボードによるナビゲーションの有効化
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // コントローラーによるナビゲーションの有効化

    //  Win32用の初期化
    ImGui_ImplWin32_Init(hWnd);

    //  DirectX11用の初期化
    ImGui_ImplDX11_Init(device, context);
}

// リセット関数
void Imase::DXTK_ImGui::Reset()
{
    if (m_isInitialized)
    {
        m_isInitialized = false;

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

// 更新処理
void Imase::DXTK_ImGui::Update(HWND hWnd)
{
    //  新フレームの開始
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // 実際のモニターの解像度に合わせてスケールを掛ける
    RECT rect = {};
    GetClientRect(hWnd, &rect);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(
        m_screenWidth / static_cast<float>(rect.right - rect.left),
        m_screenHeight / static_cast<float>(rect.bottom - rect.top) );
    
    ImGui::NewFrame();

    //  デモウィンドウの描画
//    ImGui::ShowDemoWindow();
}

// 描画処理
void Imase::DXTK_ImGui::Render()
{
    //  ImGuiの描画処理
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// メッセージ処理
bool Imase::DXTK_ImGui::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //  ImGuiのメッセージ処理
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;

    return false;
}
