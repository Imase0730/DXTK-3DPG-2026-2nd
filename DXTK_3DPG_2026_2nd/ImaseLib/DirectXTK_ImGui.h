//-------------------------------------------------------------------------------------
// File: DirectXTK_ImGui.h
//
// ImGuiの処理をまとめたクラス
//
// Date: 2025.10.14
// Author: Hideyasu Imase
// 
// 組み込み方：
//	１・pch.hに【 DirectXTK_ImGui.h 】をインクルードする。
//	２・Main.cppのWndProc関数の先頭に【 DXTK_ImGui::ProcessMessage(hWnd, message, wParam, lParam); 】を追加する。
//	３・Main.cppのwWinMain関数のg_game.reset()の直前に【 DXTK_ImGui::Reset(); 】を追加する。
//	４・Game.cppのCreateDeviceDependentResources関数に【 DXTK_ImGui::Initialize(m_deviceResources->GetWindow(), m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext()); 】を追加する。
//	５・Game.cppのUpdate関数に【 DXTK_ImGui::Update(); 】を追加する。
//	６・Game.cppのRender関数のm_deviceResources->PIXEndEvent();の直前に【 DXTK_ImGui::Render(); 】を追加する。
//
// 使用方法はデモウインドウを参考にしてください。
//
//	<<< 記述例 >>
//	ImGui::Begin("TestWindow");
// 
//	// float型３つを入力する項目を追加 
//	static float v[3] = { 0.1f, 0.2f, 0.3f };
//	ImGui::DragFloat3("input float3", v, 0.1f);
// 
//	// ImGuiのウインドウにフォーカスがあっている時はカメラの操作を制限する）
//	m_debugCamera->Update(!ImGui::IsWindowFocused());
// 
//	ImGui::End();
//
//--------------------------------------------------------------------------------------
#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

namespace Imase
{
	class DXTK_ImGui
	{
	private:

		// trueの場合は初期化済み
		static bool m_isInitialized;

		// ゲーム画面の解像度
		static int m_screenWidth;
		static int m_screenHeight;

	public:
		// 初期化関数
		static void Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

		// リセット関数
		static void Reset();

		// 更新処理
		static void Update(HWND hWnd);

		// 描画処理
		static void Render();

		// メッセージ処理
		static bool ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}