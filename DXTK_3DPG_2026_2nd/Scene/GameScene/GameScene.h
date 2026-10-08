#pragma once

#include <ImaseLib/SceneManager.h>
#include <GameContext.h>
#include "../SceneId.h"

#include <ImaseLib/DebugCamera.h>
#include <Imaselib/GridFloor.h>

#include <ImaseLib/CollisionRenderer.h>
#include <ImaseLib/ObjCollision.h>

#include <ImaseLib/InputManager.h>

#include "Robot.h"

class GameScene : public Imase::SceneBase<SceneId, GameContext>
{
public:
    // コンストラクタ
    GameScene();

    // 更新
    void Update(Imase::ISceneController<SceneId>& sceneController, GameContext& gameContext) override;

    // 描画
    void Render(GameContext& gameContext) override;

    // シーン切り替え時に呼び出される関数
    void OnEnter(GameContext& gameContext) override;

private:
    // メイン用のキー入力マネージャー
    std::unique_ptr<Imase::InputManager> m_inputMain;

    // カメラ用のキー入力マネージャー
    std::unique_ptr<Imase::InputManager> m_inputCamera;

    // ビュー行列
    DirectX::SimpleMath::Matrix m_view;

    // プロジェクション行列
    DirectX::SimpleMath::Matrix m_projection;

    // デバッグカメラ
    std::unique_ptr<Imase::DebugCamera> m_debugCamera;

    // グリッドフロア
    std::unique_ptr<Imase::GridFloor> m_gridFloor;

    // プロジェクション行列を作成する関数
    DirectX::SimpleMath::Matrix CreateProjectionMatrix(GameContext& gameContext, float nearClip, float farClip);

    // ウインドウサイズ変更時に呼び出される関数
    void OnWindowSizeChanged(GameContext& gameContext) override;

    // モデルハンドル
    std::unique_ptr<DirectX::Model> m_model;

    // コリジョン情報表示オブジェクト
    std::unique_ptr<Imase::CollisionRenderer> m_collisionRenderer;

    // モデルハンドル（床）
    std::unique_ptr<DirectX::Model> m_modelFloor;

    // モデルデータの衝突判定
    std::unique_ptr<Imase::ObjCollision> m_objCollision;

    // 各パーツのモデルハンドル（ロボット）
    std::array<std::unique_ptr<DirectX::Model>, Robot::PARTS_CNT> m_modelRobots;

    // ロボットへのユニークポインタ
    std::unique_ptr<Robot> m_robot;

    // ----- カメラ関係 ----- //

    // カメラの移動の速さ（1秒間あたりの移動量）
    static constexpr float CAMERA_MOVE_SPEED = 2.0f;

    // カメラの回転の速さ（１秒間あたりの回転量）
    static constexpr float CAMERA_ROTATE_SPEED_RAD = DirectX::XMConvertToRadians(60.0f);

    // カメラモード
    enum class CameraMode
    {
        Game,  // ゲーム中
        Debug, // デバッグモード
    };

    CameraMode m_cameraMode = CameraMode::Debug;

    // カメラの位置
    DirectX::SimpleMath::Vector3 m_cameraPosition = {0.0f, 1.0f, 3.0f};

    // カメラの回転
    DirectX::SimpleMath::Quaternion m_cameraRotation;

    // カメラの更新用の入力情報構造体
    struct CameraInput
    {
        float forwardAxis; //   前進 1　  後進 -1
        float rightAxis;   // 右移動 1　左移動 -1
        float yawAxis;     // 左回転 1　右回転 -1
        float pitchAxis;   // 上回転 1　下回転 -1
    };

    // ゲームカメラの更新関数
    void UpdateGameCamera(float elapsedTime, const CameraInput& input);

    // ゲーム中のビュー行列
    DirectX::SimpleMath::Matrix m_gameViewMatrix;

    // マウスの位置
    DirectX::SimpleMath::Vector2 m_mousePosition;

    // 操作モード
    enum class ControlMode
    {
        Robot,
        Camera,
    };

    ControlMode m_controlMode = ControlMode::Camera;
};
