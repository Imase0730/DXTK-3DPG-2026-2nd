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
#include <ImaseLib/GridFloor.h>

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

    // プロジェクション行列
    DirectX::SimpleMath::Matrix m_projection;

    // デバッグカメラ
    std::unique_ptr<Imase::DebugCamera> m_debugCamera;

    // グリッドの床
    std::unique_ptr<Imase::GridFloor> m_gridFloor;

    // 頂点バッファの構造体
    struct VertexPositionNormalTexture
    {
        DirectX::XMFLOAT3 position; // 位置
        DirectX::XMFLOAT3 normal;   // 法線
        DirectX::XMFLOAT2 texCoord; // テクスチャ座標
    };

    // 定数バッファの構造体（頂点シェーダー用）
    struct ConstantBufferVS
    {
        DirectX::XMFLOAT4X4 WorldViewProjection;    // ワールド行列×ビュー行列×プロジェクション行列
        DirectX::XMFLOAT4X4 WorldInverseTranspose;  // ワールド行列の逆転置行列
        DirectX::XMFLOAT4 LightDirection;           // ライトの方向
    };

    // 定数バッファの構造体（ピクセルシェーダー用）
    struct ConstantBufferPS
    {
        DirectX::XMFLOAT4 color;    // 色
    };

    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

    // インデックスバッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

    // 定数バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBufferVS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBufferPS;

    // 頂点シェーダー
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

    // ピクセルシェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

    // 入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    // 色
    DirectX::SimpleMath::Color m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

    // シェーダーリソースビュー
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

    // ライトの方向
    DirectX::SimpleMath::Vector3 m_lightDirection = { 0.0f, -1.0f, 0.0f };

};


