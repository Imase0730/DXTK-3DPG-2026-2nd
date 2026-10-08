//--------------------------------------------------------------------------------------
// File: ModelTestScene.cpp
//
// モデルテスト用シーン
//
// Date: 2026.4.13
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "GameScene.h"

using namespace DirectX;

// コンストラクタ
GameScene::GameScene()
{
}

// 更新
void GameScene::Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext)
{
    Imase::DebugRenderer& debugRenderer = gameContext.debugRenderer;

    // 経過時間を取得する
    float elapsedTime = static_cast<float>(gameContext.timer.GetElapsedSeconds());

    if (m_inputMain->IsTriggered("ReturnToTitle"))
    {
        sceneController.RequestSwitch(SceneId::TitleScene, 2.0f);
    }

    // カメラモードを切り替え
    if (m_inputMain->IsTriggered("CameraMode"))
    {
        if (m_cameraMode == CameraMode::Game)
        {
            m_cameraMode = CameraMode::Debug;
        }
        else
        {
            m_cameraMode = CameraMode::Game;
        }
    }

    // 操作モードを切り替え
    if (m_inputMain->IsTriggered("ControlMode"))
    {
        if (m_controlMode == ControlMode::Robot)
        {
            m_controlMode = ControlMode::Camera;
        }
        else
        {
            m_controlMode = ControlMode::Robot;
        }
    }

    // キーで操作する物を切り替える
    if (m_controlMode == ControlMode::Robot)
    {
        // ロボットの操作を有効
        m_robot->SetEnableControl(true);
        // カメラの操作を無効
        m_inputCamera->SetEnabled(false);
        debugRenderer.DrawText({0.0f, 25.0f}, L"Control:Robot");
    }
    else
    {
        // カメラの操作を有効
        m_inputCamera->SetEnabled(true);
        // ロボットの操作を無効
        m_robot->SetEnableControl(false);
        debugRenderer.DrawText({0.0f, 25.0f}, L"Control:Camera");
    }

    // ゲーム中なら
    if (m_cameraMode == CameraMode::Game)
    {
        // マウスの位置を取得する
        auto mouse = Mouse::Get().GetState();
        m_mousePosition.x = static_cast<float>(mouse.x);
        m_mousePosition.y = static_cast<float>(mouse.y);
    }

    CameraInput input = {};

    // 前進後進
    if (m_inputCamera->IsPressed("MoveForward"))
    {
        input.forwardAxis = 1.0f;
    }
    if (m_inputCamera->IsPressed("MoveBackward"))
    {
        input.forwardAxis = -1.0f;
    }

    // 左右移動
    if (m_inputCamera->IsPressed("MoveRight"))
    {
        input.rightAxis = 1.0f;
    }
    if (m_inputCamera->IsPressed("MoveLeft"))
    {
        input.rightAxis = -1.0f;
    }

    // 上下回転
    if (m_inputCamera->IsPressed("RotateUp"))
    {
        input.pitchAxis = 1.0f;
    }
    if (m_inputCamera->IsPressed("RotateDown"))
    {
        input.pitchAxis = -1.0f;
    }

    // 左右回転
    if (m_inputCamera->IsPressed("RotateLeft"))
    {
        input.yawAxis = 1.0f;
    }
    if (m_inputCamera->IsPressed("RotateRight"))
    {
        input.yawAxis = -1.0f;
    }

    // ゲーム中のカメラを更新
    UpdateGameCamera(elapsedTime, input);

    // ロボットの更新
    m_robot->Update(elapsedTime, m_objCollision);

    // デバッグカメラの更新
    //m_debugCamera->Update(elapsedTime);

    debugRenderer.DrawText({0.0f, 0.0f}, L"GameScene");

    ImGui::Begin("ConstantBuffer");

    //std::vector<float> v;

    //v = { m_color.x, m_color.y, m_color.z };
    //ImGui::ColorEdit3("PolygonColor", v.data());
    //m_color = { v[0], v[1], v[2] };

    // デバッグカメラの更新
    m_debugCamera->Update(!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows));

    ImGui::End();
}

// 描画
void GameScene::Render(GameContext& gameContext)
{
    // DirectX3Dのデバイスコンテキストを取得する
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // ゲーム中のビュー行列を設定
    m_view = m_gameViewMatrix;

    // ゲーム中のプロジェクション行列を作成
    m_projection = CreateProjectionMatrix(gameContext, 1.0f, 5.0f);

    // ----- マウスレイの表示 ----- //
    // ビューポート
    SimpleMath::Viewport viewport(0.0f, 0.0f, 1280.0f, 720.0f);

    // マウスの位置
    SimpleMath::Vector3 screenPosNear(m_mousePosition.x, m_mousePosition.y, 0.0f);
    SimpleMath::Vector3 screenPosFar(m_mousePosition.x, m_mousePosition.y, 1.0f);

    // マウスの位置をワールド空間へ
    SimpleMath::Vector3 worldPosNear =
        viewport.Unproject(screenPosNear, m_projection, m_view, SimpleMath::Matrix::Identity);
    SimpleMath::Vector3 worldPosFar =
        viewport.Unproject(screenPosFar, m_projection, m_view, SimpleMath::Matrix::Identity);

    // 線分の表示
    m_collisionRenderer->AddLineSegment(worldPosNear, worldPosFar, Colors::Yellow);
    // ---------------------------- //

    // マウスとロボットの交差判定
    SimpleMath::Vector3 direction = worldPosFar - worldPosNear;
    // 正規化する
    direction.Normalize();
    if (m_robot->IsHitRay(worldPosNear, direction))
    {
        gameContext.debugRenderer.DrawText({0.0f, 50.0f}, L"MouseRay Hit!");
    }

    // デバッグモードなら
    if (m_cameraMode == CameraMode::Debug)
    {
        // ----- ゲーム中の視錐台の表示 ----- //
        DirectX::BoundingFrustum worldFrustum;

        // プロジェクション行列から視錐台を作成
        DirectX::BoundingFrustum::CreateFromMatrix(worldFrustum, m_projection, true);

        // 視錐台をワールド空間に変換する
        worldFrustum.Transform(worldFrustum, m_view.Invert());

        // 視錐台の表示
        m_collisionRenderer->AddBoundingVolume(worldFrustum, Colors::Red);

        // デバッグカメラからビュー行列を取得する
        m_view = m_debugCamera->GetCameraMatrix();

        // プロジェクション行列を元に戻す
        m_projection = CreateProjectionMatrix(gameContext, 0.1f, 100.0f);

        // ロボットと視錐台の交差判定
        if (m_robot->IsHitFrustum(worldFrustum))
        {
            gameContext.debugRenderer.DrawText({0.0f, 75.0f}, L"Frustum Hit!");
        }
    }

    // グリッドフロアの描画
    // m_gridFloor->Render(context, m_view, m_projection);

    // 床の描画
    SimpleMath::Matrix world;
    // m_modelFloor->Draw(context, gameContext.commonStates, world, m_view, m_projection);

    // ロボットの描画
    m_robot->Render();

    // ロボットのコリジョン情報の表示
    m_robot->DisplayCollision(m_collisionRenderer.get());

    // 衝突判定用のモデルデータ（床）の表示
    m_objCollision->AddCollisionRenderer(m_collisionRenderer.get());

    // コリジョン情報の描画
    m_collisionRenderer->DrawCollision(context, gameContext.commonStates, m_view, m_projection);
}

// シーン切り替え時に呼び出される関数
void GameScene::OnEnter(GameContext& gameContext)
{
    // メイン用のキー入力マネージャーの作成
    m_inputMain = std::make_unique<Imase::InputManager>(gameContext.keyboardTracker);

    // カメラ用のキー入力マネージャーの作成
    m_inputCamera = std::make_unique<Imase::InputManager>(gameContext.keyboardTracker);

    // ----- 入力キーの登録 ----- //
    // メイン用
    m_inputMain->BindAction("CameraMode", Keyboard::Keys::Space);
    m_inputMain->BindAction("ControlMode", Keyboard::Keys::C);
    m_inputMain->BindAction("ReturnToTitle", Keyboard::Keys::A);

    // カメラの移動
    m_inputCamera->BindAction("MoveForward", Keyboard::Keys::Up);
    m_inputCamera->BindAction("MoveBackward", Keyboard::Keys::Down);
    m_inputCamera->BindAction("MoveLeft", Keyboard::Keys::Left);
    m_inputCamera->BindAction("MoveRight", Keyboard::Keys::Right);

    // カメラの回転
    m_inputCamera->BindAction("RotateUp", Keyboard::Keys::Up, Imase::InputManager::Shift);
    m_inputCamera->BindAction("RotateDown", Keyboard::Keys::Down, Imase::InputManager::Shift);
    m_inputCamera->BindAction("RotateLeft", Keyboard::Keys::Left, Imase::InputManager::Shift);
    m_inputCamera->BindAction("RotateRight", Keyboard::Keys::Right, Imase::InputManager::Shift);
    // -------------------------- //

    // プロジェクション行列を設定する
    m_projection = CreateProjectionMatrix(gameContext, 0.1f, 100.0f);

    // DirectX3Dのデバイスを取得する
    auto device = gameContext.deviceResources.GetD3DDevice();

    // DirectX3Dのデバイスコンテキストを取得する
    auto context = gameContext.deviceResources.GetD3DDeviceContext();

    // 画面のサイズを取得する
    RECT rect = gameContext.deviceResources.GetOutputSize();

    // デバッグカメラの作成
    m_debugCamera = std::make_unique<Imase::DebugCamera>(rect.right, rect.bottom);

    // グリッドフロアの作成
    m_gridFloor = std::make_unique<Imase::GridFloor>(device, context, &gameContext.commonStates);

    // エフェクトを作成する工場
    EffectFactory fx(device);
    fx.SetDirectory(L"Resources/Models"); // <- ddsのフォルダ

    // モデルの読み込み（床）
    m_modelFloor = Model::CreateFromCMO(device, L"Resources/Models/Floor.cmo", fx);

    // コリジョン情報表示オブジェクトの作成
    m_collisionRenderer = std::make_unique<Imase::CollisionRenderer>(device, context);

    // モデルデータから衝突判定用データを作成
    m_objCollision = std::make_unique<Imase::ObjCollision>("Resources/Models/Floor.obj");

    // 各パーツのモデルの読み込み（ロボット）
    m_modelRobots[static_cast<int>(Robot::Parts::LEG)] = Model::CreateFromCMO(device, L"Resources/Models/Leg.cmo", fx);
    m_modelRobots[static_cast<int>(Robot::Parts::BODY)] =
        Model::CreateFromCMO(device, L"Resources/Models/Body.cmo", fx);
    m_modelRobots[static_cast<int>(Robot::Parts::HEAD)] =
        Model::CreateFromCMO(device, L"Resources/Models/Head.cmo", fx);
    m_modelRobots[static_cast<int>(Robot::Parts::ARM_R)] =
        Model::CreateFromCMO(device, L"Resources/Models/Arm_R.cmo", fx);
    m_modelRobots[static_cast<int>(Robot::Parts::ARM_L)] =
        Model::CreateFromCMO(device, L"Resources/Models/Arm_L.cmo", fx);
    m_modelRobots[static_cast<int>(Robot::Parts::MISSILE)] =
        Model::CreateFromCMO(device, L"Resources/Models/Missile.cmo", fx);

    // ロボットの作成
    m_robot = std::make_unique<Robot>(gameContext, m_view, m_projection, m_modelRobots);

    // ----- 画面切り替え演出中はRender関数のみ呼ばれるため初期化のため一度更新処理を行う ----- //
    
    // ゲーム中のカメラを更新
    CameraInput input = {};
    UpdateGameCamera(0.0f, input);

    // ロボットの更新
    m_robot->Update(0.0f, m_objCollision);

    // デバッグカメラの更新
    m_debugCamera->Update(false);
}

// プロジェクション行列を作成する関数
DirectX::SimpleMath::Matrix GameScene::CreateProjectionMatrix(GameContext& gameContext, float nearClip, float farClip)
{
    SimpleMath::Matrix m;

    // 画面のサイズを取得する
    RECT rect = gameContext.deviceResources.GetOutputSize();

    // プロジェクション行列を作成
    m = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f),                                        // 画角
        static_cast<float>(rect.right) / static_cast<float>(rect.bottom), // アスペクト比
        nearClip,                                                         // Near Clip
        farClip                                                           // Far Clip
    );

    return m;
}

// ウインドウサイズ変更時に呼び出される関数
void GameScene::OnWindowSizeChanged(GameContext& gameContext)
{
    // プロジェクション行列を設定する
    m_projection = CreateProjectionMatrix(gameContext, 0.1f, 100.0f);
}

// ゲームカメラの更新関数
void GameScene::UpdateGameCamera(float elapsedTime, const CameraInput& input)
{
    // 今回の移動量と回転量をもとめる
    float movesSpeed = CAMERA_MOVE_SPEED * elapsedTime;
    float rotateSpeed = CAMERA_ROTATE_SPEED_RAD * elapsedTime;

    // 現在の回転から「カメラの右方向（横軸）」をもとめる
    SimpleMath::Vector3 right = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Right, m_cameraRotation);

    // 左右：ワールド空間も上方向を軸にする
    SimpleMath::Quaternion yawRotation =
        SimpleMath::Quaternion::CreateFromAxisAngle(SimpleMath::Vector3::Up, input.yawAxis * rotateSpeed);

    // 上下：カメラの右方向(right)を軸にする
    SimpleMath::Quaternion pitchRotation =
        SimpleMath::Quaternion::CreateFromAxisAngle(right, input.pitchAxis * rotateSpeed);

    // 「現在の回転」に対して上下回転を適用し、最後に左右回転を適応する
    m_cameraRotation = m_cameraRotation * pitchRotation * yawRotation;

    // 誤差が蓄積して歪むのを防ぐため正規化する
    m_cameraRotation.Normalize();

    // 最新の回転から「前方向」と「右方向」を再計算する
    SimpleMath::Vector3 forward = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward, m_cameraRotation);
    right = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Right, m_cameraRotation);

    // カメラの位置を移動
    m_cameraPosition += forward * movesSpeed * input.forwardAxis;
    m_cameraPosition += right * movesSpeed * input.rightAxis;

    // ゲーム中のカメラのビュー行列を作成
    SimpleMath::Vector3 target = m_cameraPosition + forward;
    SimpleMath::Vector3 up = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Up, m_cameraRotation);
    m_gameViewMatrix = SimpleMath::Matrix::CreateLookAt(m_cameraPosition, target, up);
}
