//--------------------------------------------------------------------------------------
// File: ShaderTestScene.cpp
//
// シェーダーをテストするシーン
//
// Date: 2026.4.13
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ShaderTestScene.h"

#include <DirectXTK_Utilities/ReadData.h>
#include <ImGui/ImGuizmo.h>

using namespace DirectX;

// 更新
void ShaderTestScene::Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext)
{
    Imase::DebugRenderer& debugRenderer = gameContext.debugRenderer;

    ImGui::Begin("ConstantBuffer");

    // ImGuiを使って、色を変更する
    std::vector<float> v;

    // ライトのディフューズ色
    v = { m_lightDiffuseColor.x, m_lightDiffuseColor.y, m_lightDiffuseColor.z };
    ImGui::ColorEdit3("LightDiffuseColor", v.data());
    m_lightDiffuseColor = { v[0], v[1], v[2] };

    // ライトのアンビエント色
    v = { m_lightAmbientColor.x, m_lightAmbientColor.y, m_lightAmbientColor.z };
    ImGui::ColorEdit3("LightAmbientColor", v.data());
    m_lightAmbientColor = { v[0], v[1], v[2] };

    // ギズモの表示領域を画面全体に設定
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    // ギズモの大きさを調整
    ImGuizmo::SetGizmoSizeClipSpace(0.2f);

    // クォータニオンからギズモ用のワールド行列を作成
    SimpleMath::Matrix lightMatrix = SimpleMath::Matrix::CreateFromQuaternion(m_lightRotation);
    SimpleMath::Matrix viewGizmo = m_debugCamera->GetCameraMatrix();
    SimpleMath::Matrix projGizmo = m_projection;

    // ギズモの描画と操作
    ImGuizmo::Manipulate(
        &viewGizmo.m[0][0],
        &projGizmo.m[0][0],
        ImGuizmo::ROTATE,
        ImGuizmo::WORLD,
        &lightMatrix.m[0][0]
    );

    // 行列からクォータニオンに変換する
    m_lightRotation = SimpleMath::Quaternion::CreateFromRotationMatrix(lightMatrix);
    // ライトの初期方向「真下（0,-1,0)」とし、現在の回転を適用する
    SimpleMath::Vector3 defaultLightDirection = { 0.0f, -1.0f, 0.0f };
    m_lightDirection = SimpleMath::Vector3::Transform(defaultLightDirection, m_lightRotation);
    m_lightDirection.Normalize();

    // デバッグカメラの更新（ImGuiまたはImGuizmoの上ならカメラは動かない）
    m_debugCamera->Update(!(ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) || (ImGuizmo::IsOver())));

    ImGui::End();

    debugRenderer.DrawText({ 0.0f, 0.0f }, L"ShaderTestScene");
}

// 描画
void ShaderTestScene::Render(GameContext& gameContext)
{
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // ----- 各行列の設定 ----- //
    SimpleMath::Matrix world, view;

    // ワールド行列（単位行列）
    world = SimpleMath::Matrix::Identity;

    // ビュー行列
    view = m_debugCamera->GetCameraMatrix();

    // プロジェクション行列
    RECT rect = gameContext.deviceResources.GetOutputSize();
    m_projection = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
                                XMConvertToRadians(45.0f),
                                static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
                                0.1f, 100.0f );

    // グリッドの床の描画
    m_gridFloor->Render(context, view, m_projection);

    // ライトの設定
    m_diceModel->UpdateEffects([&](IEffect* effect)
        {
            auto lights = dynamic_cast<IEffectLights*>(effect);
            if (lights)
            {
                lights->SetLightDirection(0, m_lightDirection);
                lights->SetLightDiffuseColor(0, m_lightDiffuseColor);
                lights->SetAmbientLightColor(m_lightAmbientColor);
            }
        }
    );

    // サイコロの描画
    m_diceModel->Draw(context, gameContext.commonStates, world, view, m_projection, false, [&]()
        {
            // ----- VS（頂点シェーダー） ----- //
            // 頂点シェーダーの設定
            context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

            // ----- PS（ピクセルシェーダー） ----- //
            // ピクセルシェーダーの設定
            context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        }
    );
}

// シーン切り替え時に呼び出される関数
void ShaderTestScene::OnEnter(GameContext& gameContext)
{
    auto device = gameContext.deviceResources.GetD3DDevice();
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // デバッグカメラの作成
    m_debugCamera = std::make_unique<Imase::DebugCamera>(1280, 720);

    // グリッドの床の作成
    m_gridFloor = std::make_unique<Imase::GridFloor>(device, context, &gameContext.commonStates);

    // サイコロのモデルの読み込み
    EffectFactory fx(device);
    fx.SetDirectory(L"Resources/Models");
    m_diceModel = Model::CreateFromCMO(device, L"Resources/Models/Dice.cmo", fx);

    // ライトの０番はON、１番と２番はOFFにする
    m_diceModel->UpdateEffects([&](IEffect* effect)
        {
            auto lights = dynamic_cast<IEffectLights*>(effect);
            if(lights)
            {
                lights->SetLightEnabled(1, false);
                lights->SetLightEnabled(2, false);
            }
        }
    );

    // ----- 頂点シェーダーの作成 ----- //　
    std::vector<uint8_t> vs_data = DX::ReadData(L"Resources/Shaders/VertexShader.cso");
    DX::ThrowIfFailed(
        device->CreateVertexShader(vs_data.data(), vs_data.size(),
            nullptr, m_vertexShader.ReleaseAndGetAddressOf())
    );

    // ----- ピクセルシェーダーの作成 ----- //　
    std::vector<uint8_t> ps_data = DX::ReadData(L"Resources/Shaders/PixelShader.cso");
    DX::ThrowIfFailed(
        device->CreatePixelShader(ps_data.data(), ps_data.size(),
            nullptr, m_pixelShader.ReleaseAndGetAddressOf())
    );
}
