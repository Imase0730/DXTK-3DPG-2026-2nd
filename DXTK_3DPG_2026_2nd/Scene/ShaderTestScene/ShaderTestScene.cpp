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

    ImGui::Begin("ConstantBuffer");

    // ImGuiを使って、色を変更する
    std::vector<float> v;
    v = { m_color.x, m_color.y, m_color.z };
    ImGui::ColorEdit3("VertexColor", v.data());
    m_color = { v[0], v[1], v[2] };

    // デバッグカメラの更新
    m_debugCamera->Update(!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows));

    ImGui::End();

    debugRenderer.DrawText({ 0.0f, 0.0f }, L"ShaderTestScene");
}

// 描画
void ShaderTestScene::Render(GameContext& gameContext)
{
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // ----- 各行列の設定 ----- //
    SimpleMath::Matrix w, v, p;

    // ワールド行列（単位行列）
    w = SimpleMath::Matrix::Identity;

    // ビュー行列
    v = m_debugCamera->GetCameraMatrix();

    // プロジェクション行列
    p = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    // グリッドの床の描画
    m_gridFloor->Render(context, v, p);

    // ----- IA（入力アセンブラ） ----- //
    // 頂点バッファの設定
    ID3D11Buffer* vsBuffers[] = { m_vertexBuffer.Get() };
    UINT strides[] = { sizeof(VertexPositionTexture) };
    UINT offsets[] = { 0 };
    context->IASetVertexBuffers(0, 1, vsBuffers, strides, offsets);

    // インデックスバッファの設定
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // トポロジーの設定
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 入力レイアウトの設定
    context->IASetInputLayout(m_inputLayout.Get());

    // ----- 定数バッファの更新 ----- //
    D3D11_MAPPED_SUBRESOURCE subresource;

    // 定数バッファの更新（頂点シェーダー用）
    ConstantBufferVS bufVS = { w * v * p };
    context->Map(m_constantBufferVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    memcpy(subresource.pData, &bufVS, sizeof(ConstantBufferVS));
    context->Unmap(m_constantBufferVS.Get(), 0);

    // 定数バッファの更新（ピクセルシェーダー用）
    ConstantBufferPS bufPS = { m_color };
    context->Map(m_constantBufferPS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    memcpy(subresource.pData, &bufPS, sizeof(ConstantBufferPS));
    context->Unmap(m_constantBufferPS.Get(), 0);

    ID3D11Buffer* cstBuffers[] = { m_constantBufferVS.Get(), m_constantBufferPS.Get() };

    // ----- VS（頂点シェーダー） ----- //
    // 定数バッファの設定
    context->VSSetConstantBuffers(0, 2, cstBuffers);
    // 頂点シェーダーの設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    // ----- RS（ラスタライザ） ----- //
    // カリングなし
    context->RSSetState(gameContext.commonStates.CullNone());

    // ----- PS（ピクセルシェーダー） ----- //
    // シェーダーリソースの設定
    ID3D11ShaderResourceView* srvs[] = { m_srv.Get() };
    context->PSSetShaderResources(0, 1, srvs);
    // サンプラーの設定
    ID3D11SamplerState* samplers[] = { gameContext.commonStates.PointClamp() };
    context->PSSetSamplers(0, 1, samplers);
    // 定数バッファの設定
    context->PSSetConstantBuffers(0, 2, cstBuffers);
    // ピクセルシェーダーの設定
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // ----- OM（出力マージャー） ----- //
    // 深度ステンシルステートの設定
    context->OMSetDepthStencilState(gameContext.commonStates.DepthDefault(), 0);
    // ブレンドステートの設定
    context->OMSetBlendState(gameContext.commonStates.NonPremultiplied(), nullptr, 0xffffffff);

    // 四角形の描画
    context->DrawIndexed(6, 0, 0);
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

    // 四角形の頂点データ
    VertexPositionTexture vertexData[4] =
    {
        { SimpleMath::Vector3(-1.0f,  1.0f, 0.0f), SimpleMath::Vector2(0.0f,   0.0f)  },   // 0
        { SimpleMath::Vector3( 1.0f,  1.0f, 0.0f), SimpleMath::Vector2(0.125f, 0.0f)  },   // 1
        { SimpleMath::Vector3( 1.0f, -1.0f, 0.0f), SimpleMath::Vector2(0.125f, 0.25f) },   // 2
        { SimpleMath::Vector3(-1.0f, -1.0f, 0.0f), SimpleMath::Vector2(0.0f,   0.25f) },   // 3
    };

    // ----- 頂点バッファの作成 ----- //
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(VertexPositionTexture) * 4; // バッファのサイズ
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;          // 頂点バッファとして使用
        desc.CPUAccessFlags = 0;                            // CPU からアクセスは不要
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

    // 四角形のインデックスデータ
    uint16_t indexData[6] = { 0, 1, 2, 0, 2, 3 };

    // ----- インデックスバッファの作成 ----- //
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(uint16_t) * 6;          // バッファのサイズ
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;       // インデックスバッファとして使用
        desc.CPUAccessFlags = 0;                        // CPU からアクセスは不要
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = indexData;   // CPU 側のデータのポインタを渡す
        initData.SysMemPitch = 0;       // インデックスバッファの場合、ピッチは不要（ 0 にする）
        initData.SysMemSlicePitch = 0;  // インデックスバッファの場合、スライスも不要（ 0 にする）

        DX::ThrowIfFailed(
            device->CreateBuffer(&desc, &initData, m_indexBuffer.ReleaseAndGetAddressOf())
        );
    }

    // ----- 定数バッファの作成 ----- //
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;    // 定数バッファとして使用
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // CPU からアクセス は必要
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        // 頂点シェーダー用
        desc.ByteWidth = sizeof(ConstantBufferVS);      // バッファのサイズ
        DX::ThrowIfFailed(
            device->CreateBuffer(&desc, nullptr, m_constantBufferVS.ReleaseAndGetAddressOf())
        );

        // ピクセルシェーダー用
        desc.ByteWidth = sizeof(ConstantBufferPS);      // バッファのサイズ
        DX::ThrowIfFailed(
            device->CreateBuffer(&desc, nullptr, m_constantBufferPS.ReleaseAndGetAddressOf())
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
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // ----- 入力レイアウトの作成 ----- //
    DX::ThrowIfFailed(
        device->CreateInputLayout( layout, _countof(layout),
                                   vs_data.data(), vs_data.size(),
                                   m_inputLayout.ReleaseAndGetAddressOf())
    );

    // テクスチャのロード
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(
            device, L"Resources/Textures/ShootingGame.dds", nullptr, m_srv.ReleaseAndGetAddressOf())
    );

}
