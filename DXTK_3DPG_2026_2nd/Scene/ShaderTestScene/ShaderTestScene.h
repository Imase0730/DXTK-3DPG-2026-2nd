//--------------------------------------------------------------------------------------
// File: ShaderTestScene.h
//
// シェーダーをテストするシーン
//
// Date: 2026.4.13
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "ImaseLib/SceneManager.h"
#include "GameContext.h"
#include "../SceneId.h"
#include <ImaseLib/DebugCamera.h>

class ShaderTestScene : public Imase::SceneBase<SceneId, GameContext>
{
public:

    // 更新
    void Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext) override;

    // 描画
    void Render(GameContext& gameContext) override;

    // シーン切り替え時に呼び出される関数
    void OnEnter(GameContext& gameContext) override;

private:

    // 頂点バッファの構造体
    struct VertexPosition
    {
        DirectX::XMFLOAT3 position; // 位置
    };

    // 定数バッファの構造体
    struct ConstantBuffer
    {
        DirectX::XMFLOAT4X4 wvp;    // ワールド行列×ビュー行列×プロジェクション行列
    };

    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

    // 定数バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

    // 頂点シェーダー
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

    // ピクセルシェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

    // 入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    // デバッグカメラ
    std::unique_ptr<Imase::DebugCamera> m_debugCamera;

};


