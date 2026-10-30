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

    // ライトの回転クォータニオン
    DirectX::SimpleMath::Quaternion m_lightRotation;

    // ライトの方向
    DirectX::SimpleMath::Vector3 m_lightDirection;

    // ライトのディフューズ色
    DirectX::SimpleMath::Color m_lightDiffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    // ライトのアンビエント色
    DirectX::SimpleMath::Color m_lightAmbientColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    // サイコロのモデル
    std::unique_ptr<DirectX::Model> m_diceModel;

    // 頂点シェーダー
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

    // ピクセルシェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;


};


