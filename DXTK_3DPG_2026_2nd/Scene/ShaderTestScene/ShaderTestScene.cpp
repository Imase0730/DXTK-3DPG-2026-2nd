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

using namespace DirectX;

// 更新
void ShaderTestScene::Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext)
{
    Imase::DebugRenderer& debugRenderer = gameContext.debugRenderer;

    // デバッグカメラの更新
    m_debugCamera->Update();

    debugRenderer.DrawText({ 0.0f, 0.0f }, L"ShaderTestScene");
}

// 描画
void ShaderTestScene::Render(GameContext& gameContext)
{
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // ----- IA（入力アセンブラ） ----- //
    // 頂点バッファの設定
    ID3D11Buffer* vsBuffers[] = { m_vertexBuffer.Get() };
    UINT strides[] = { sizeof(VertexPosition) };
    UINT offsets[] = { 0 };
    context->IASetVertexBuffers(0, 1, vsBuffers, strides, offsets);

    // トポロジーの設定
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 入力レイアウトの設定
    context->IASetInputLayout(m_inputLayout.Get());

    // ----- VS（頂点シェーダー） ----- //
    SimpleMath::Matrix w, v, p;

    // ワールド行列（単位行列）
    w = SimpleMath::Matrix::Identity;

    // ビュー行列
    v = SimpleMath::Matrix::CreateLookAt( SimpleMath::Vector3(0.0f, 0.0f, 5.0f),
                                          SimpleMath::Vector3(0.0f, 0.0f, 0.0f),
                                          SimpleMath::Vector3::Up );

    v = m_debugCamera->GetCameraMatrix();

    // プロジェクション行列
    p = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
                                    XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    ConstantBuffer buf = { w * v * p };

    // 定数バッファの更新
    D3D11_MAPPED_SUBRESOURCE subresource;
    context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    memcpy(subresource.pData, &buf, sizeof(ConstantBuffer));
    context->Unmap(m_constantBuffer.Get(), 0);

    // 定数バッファの設定
    ID3D11Buffer* cstBuffers[] = { m_constantBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, cstBuffers);

    // 頂点シェーダーの設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    // ----- RS（ラスタライザ） ----- //
    // カリングなし
    context->RSSetState(gameContext.commonStates.CullNone());

    // ----- PS（ピクセルシェーダー） ----- //
    // ピクセルシェーダーの設定
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // ----- OM（出力マージャー） ----- //
    // 深度ステンシルステートの設定
    context->OMSetDepthStencilState(gameContext.commonStates.DepthDefault(), 0);
    // ブレンドステートの設定
    context->OMSetBlendState(gameContext.commonStates.Opaque(), nullptr, 0xffffffff);

    // 三角形の描画
    context->Draw(3, 0);
}

// シーン切り替え時に呼び出される関数
void ShaderTestScene::OnEnter(GameContext& gameContext)
{
    // 頂点バッファの構造体
    struct VertexPosition
    {
        DirectX::XMFLOAT3 position; // 位置
    };

    // 三角形の頂点データ
    VertexPosition vertexData[3] =
    {
        SimpleMath::Vector3(-1.0f, 0.0f, 0.0f),     // 0
        SimpleMath::Vector3( 0.0f, 1.0f, 0.0f),     // 1
        SimpleMath::Vector3( 1.0f, 0.0f, 0.0f),     // 2
    };

    auto device = gameContext.deviceResources.GetD3DDevice();

    // ----- 頂点バッファの作成 ----- //
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(VertexPosition) * 3;    // バッファのサイズ
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;      // 頂点バッファとして使用
        desc.CPUAccessFlags = 0;                        // CPU からアクセスは不要
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertexData;  // CPU 側のデータのポインタを渡す
        initData.SysMemPitch = 0;       // 頂点バッファの場合、ピッチは不要（ 0 にする）
        initData.SysMemSlicePitch = 0;  // 頂点バッファの場合、スライスも不要（ 0 にする）

        DX::ThrowIfFailed(
            device->CreateBuffer(&desc, &initData, m_vertexBuffer.ReleaseAndGetAddressOf())
        );
    }

    // ----- 定数バッファの作成 ----- //
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(ConstantBuffer);        // バッファのサイズ
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;    // 定数バッファとして使用
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // CPU からアクセス は必要
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        DX::ThrowIfFailed(
            device->CreateBuffer(&desc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf())
        );
    }

    // ----- 頂点シェーダーの作成 ----- //　
    std::vector<uint8_t> vs_data = DX::ReadData(L"Resources/Shaders/VertexShader.cso");
    DX::ThrowIfFailed(
        device->CreateVertexShader( vs_data.data(), vs_data.size(),
                                    nullptr, m_vertexShader.ReleaseAndGetAddressOf())
    );

    // ----- ピクセルシェーダーの作成 ----- //　
    std::vector<uint8_t> ps_data = DX::ReadData(L"Resources/Shaders/PixelShader.cso");
    DX::ThrowIfFailed(
        device->CreatePixelShader( ps_data.data(), ps_data.size(),
                                   nullptr, m_pixelShader.ReleaseAndGetAddressOf())
    );

    // 頂点データの構造
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // ----- 入力レイアウトの作成 ----- //
    DX::ThrowIfFailed(
        device->CreateInputLayout( layout, _countof(layout),
                                   vs_data.data(), vs_data.size(),
                                   m_inputLayout.ReleaseAndGetAddressOf())
    );

    // デバッグカメラの作成
    m_debugCamera = std::make_unique<Imase::DebugCamera>(1280, 720);
}
