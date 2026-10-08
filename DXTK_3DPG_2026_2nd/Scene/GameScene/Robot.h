#pragma once

#include "GameContext.h"
#include <array>
#include <ImaseLib/InputManager.h>
#include <ImaseLib/ModelCollision.h>
#include <ImaseLib/ObjCollision.h>

class Robot
{
public:

    // パーツ
    enum class Parts
    {
        LEG,        // 足
        BODY,       // 体
        HEAD,       // 頭
        ARM_R,      // 右手
        ARM_L,      // 左手
        MISSILE,    // ミサイル

        PARTS_CNT
    };

    // パーツ数
    static constexpr int PARTS_CNT = static_cast<int>(Parts::PARTS_CNT);

	// コンストラクタ
    Robot(
        const GameContext& gameContext,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const std::array<std::unique_ptr<DirectX::Model>, PARTS_CNT>& model
	);

    // 更新
    void Update(float elapsedTime, const std::unique_ptr<Imase::ObjCollision>& floor);

    // 描画
    void Render();

    // 位置を取得する関数
    DirectX::SimpleMath::Vector3 GetPosition() const;

    // ロボットのY座標を設定する関数
    void SetPositionY(float y)
    {
        m_positions[static_cast<int>(Parts::LEG)].y = y;
    }

    // ロボットの傾きを設定する関数
    void SetTilt(DirectX::SimpleMath::Quaternion tilt)
    {
        m_rotates[static_cast<int>(Parts::LEG)] *= tilt;
    }

    // ロボットの操作を有効・無効を切り替える関数
    void SetEnableControl(bool enable)
    {
        m_inputRobot->SetEnabled(enable);
    }

    // コリジョン情報を表示する関数
    void DisplayCollision(Imase::CollisionRenderer* pCollisionRenderer);

    // Rayとの交差判定関数
    bool IsHitRay(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 direction);

    // Frustumとの交差判定関数
    bool IsHitFrustum(DirectX::BoundingFrustum frustum);

private:

    // ロボットのキー入力マネージャー
    std::unique_ptr<Imase::InputManager> m_inputRobot;

    // 回転の速さ（１秒間あたりの角度）
    static constexpr float ROTATE_SPEED_DEG = 90.0f;

    // 移動の速さ（１秒間あたりの移動量）
    static constexpr float MOVE_SPEED = 2.0f;

	// ゲームコンテキスト
    const GameContext& m_gameContext;

	// ビュー行列
    const DirectX::SimpleMath::Matrix& m_view;

	// プロジェクション行列
    const DirectX::SimpleMath::Matrix& m_projection;

    // 向いている角度（ラジアン）
    float m_facingAngleRad = 0.0f;

    // 各パーツのモデルへのポインタ
    DirectX::Model* m_pModels[PARTS_CNT];

    // 各パーツの位置
    DirectX::SimpleMath::Vector3 m_positions[PARTS_CNT];

    // 各パーツの回転
    DirectX::SimpleMath::Quaternion m_rotates[PARTS_CNT];

    // ----- 各パーツの行列 ----- //

    // 初期化行列
    DirectX::SimpleMath::Matrix m_initializeMatrices[PARTS_CNT];

    // ワールド行列
    DirectX::SimpleMath::Matrix m_worldMatrices[PARTS_CNT];

    // -------------------------- //

    // 各パーツの親のインデックス
    int m_parentIndexes[PARTS_CNT];

    // 各パーツのコリジョン情報
    std::unique_ptr<Imase::ModelCollision> m_modelCollisions[PARTS_CNT];

    // ミサイル発射フラグ
    bool m_fireFlag = false;

    // 各パーツのワールド行列を更新する関数
    void UpdateWorldMatrices();

};





