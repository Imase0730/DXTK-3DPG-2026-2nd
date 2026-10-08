#include "pch.h"
#include "Robot.h"

using namespace DirectX;

// コンストラクタ
Robot::Robot(
    const GameContext& gameContext,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& projection,
    const std::array<std::unique_ptr<DirectX::Model>, PARTS_CNT>& model
)
    : m_gameContext(gameContext)
    , m_view(view)
    , m_projection(projection)
{
    // ロボットのキー入力マネージャーの作成
    m_inputRobot = std::make_unique<Imase::InputManager>(gameContext.keyboardTracker);

    // ----- 入力キーの登録 ----- //
    // ロボットの移動
    m_inputRobot->BindAction("MoveForward",  Keyboard::Keys::Up);
    m_inputRobot->BindAction("MoveBackward", Keyboard::Keys::Down);

    // ロボットの回転
    m_inputRobot->BindAction("RotateLeft",  Keyboard::Keys::Left);
    m_inputRobot->BindAction("RotateRight", Keyboard::Keys::Right);

    // ロボットの体の回転
    m_inputRobot->BindAction("BodyRotateLeft",  Keyboard::Keys::Left,  Imase::InputManager::Shift);
    m_inputRobot->BindAction("BodyRotateRight", Keyboard::Keys::Right, Imase::InputManager::Shift);

    // ロボットの左腕の回転
    m_inputRobot->BindAction("ArmLRotateUp",   Keyboard::Keys::Up,   Imase::InputManager::Shift);
    m_inputRobot->BindAction("ArmLRotateDown", Keyboard::Keys::Down, Imase::InputManager::Shift);

    // ミサイル関係
    m_inputRobot->BindAction("ShootMissile", Keyboard::Keys::X);
    m_inputRobot->BindAction("ResetMissile", Keyboard::Keys::Z);
    // -------------------------- //
 
    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        // モデルへのポインタを取得
        m_pModels[i] = model[i].get();

        // コリジョン情報を作成
        m_modelCollisions[i] =
            Imase::ModelCollisionFactory::CreateCollision(m_pModels[i], Imase::ModelCollision::CollisionType::OBB);
    }

    // 親のインデックスを設定する
    m_parentIndexes[static_cast<int>(Parts::LEG)] = -1;                                 // 親はいない
    m_parentIndexes[static_cast<int>(Parts::BODY)] = static_cast<int>(Parts::LEG);      // 親はLEG
    m_parentIndexes[static_cast<int>(Parts::HEAD)] = static_cast<int>(Parts::BODY);     // 親はBODY
    m_parentIndexes[static_cast<int>(Parts::ARM_R)] = static_cast<int>(Parts::BODY);    // 親はBODY
    m_parentIndexes[static_cast<int>(Parts::ARM_L)] = static_cast<int>(Parts::BODY);    // 親はBODY
    m_parentIndexes[static_cast<int>(Parts::MISSILE)] = static_cast<int>(Parts::ARM_L); // 親はARM_L

    // 初期化行列を作成
    m_initializeMatrices[static_cast<int>(Parts::LEG)] =
        SimpleMath::Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
    m_initializeMatrices[static_cast<int>(Parts::BODY)] =
        SimpleMath::Matrix::CreateTranslation(0.0f, 0.32f, 0.0f);       // LEGからの相対距離
    m_initializeMatrices[static_cast<int>(Parts::HEAD)] =
        SimpleMath::Matrix::CreateTranslation(0.0f, 0.43f, 0.0f);       // BODYからの相対距離
    m_initializeMatrices[static_cast<int>(Parts::ARM_R)] =
        SimpleMath::Matrix::CreateTranslation(0.18f, 0.4f, 0.0f);       // BODYからの相対距離
    m_initializeMatrices[static_cast<int>(Parts::ARM_L)] =
        SimpleMath::Matrix::CreateTranslation(-0.18f, 0.4f, 0.0f);      // BODYからの相対距離
    m_initializeMatrices[static_cast<int>(Parts::MISSILE)] =
        SimpleMath::Matrix::CreateTranslation(-0.1f, -0.2f, -0.39f);    // ARM_Lからの相対距離
}

// 更新
void Robot::Update(
    float elapsedTime,
    const std::unique_ptr<Imase::ObjCollision>& floor
)
{
    // 左回転
    if (m_inputRobot->IsPressed("RotateLeft"))
    {
        m_facingAngleRad += XMConvertToRadians(ROTATE_SPEED_DEG) * elapsedTime;
    }
    // 右回転
    if (m_inputRobot->IsPressed("RotateRight"))
    {
        m_facingAngleRad -= XMConvertToRadians(ROTATE_SPEED_DEG) * elapsedTime;
    }

    // 回転クォータニオンを作成する
    m_rotates[static_cast<int>(Parts::LEG)] =
        SimpleMath::Quaternion::CreateFromYawPitchRoll(m_facingAngleRad, 0.0f, 0.0f);

    // Forward(0,0,-1)を回転クォータニオンで回転させる
    SimpleMath::Vector3 v = 
        SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward, m_rotates[static_cast<int>(Parts::LEG)]);

    // 前進
    if (m_inputRobot->IsPressed("MoveForward"))
    {
        m_positions[static_cast<int>(Parts::LEG)] += v * MOVE_SPEED * elapsedTime;
    }
    // 後進
    if (m_inputRobot->IsPressed("MoveBackward"))
    {
        m_positions[static_cast<int>(Parts::LEG)] -= v * MOVE_SPEED * elapsedTime;
    }

    // 体を左回転
    if (m_inputRobot->IsPressed("BodyRotateLeft"))
    {
        m_rotates[static_cast<int>(Parts::BODY)] *= SimpleMath::Quaternion::CreateFromAxisAngle(
            SimpleMath::Vector3::UnitY, XMConvertToRadians(ROTATE_SPEED_DEG) * elapsedTime);
    }
    // 体を右回転
    if (m_inputRobot->IsPressed("BodyRotateRight"))
    {
        m_rotates[static_cast<int>(Parts::BODY)] *= SimpleMath::Quaternion::CreateFromAxisAngle(
            SimpleMath::Vector3::UnitY, -XMConvertToRadians(ROTATE_SPEED_DEG) * elapsedTime);
    }
    
    // 左腕を上回転
    if (m_inputRobot->IsPressed("ArmLRotateUp"))
    {
        m_rotates[static_cast<int>(Parts::ARM_L)] *= SimpleMath::Quaternion::CreateFromAxisAngle(
            SimpleMath::Vector3::UnitX, XMConvertToRadians(ROTATE_SPEED_DEG) * elapsedTime);
    }
    // 左腕を下回転
    if (m_inputRobot->IsPressed("ArmLRotateDown"))
    {
        m_rotates[static_cast<int>(Parts::ARM_L)] *= SimpleMath::Quaternion::CreateFromAxisAngle(
            SimpleMath::Vector3::UnitX, -XMConvertToRadians(ROTATE_SPEED_DEG) * elapsedTime);
    }

    // ミサイルを発射
    if (m_inputRobot->IsTriggered("ShootMissile"))
    {
        // 発射フラグ（ON）
        m_fireFlag = true;

        // ミサイルを切り離す
        m_parentIndexes[static_cast<int>(Parts::MISSILE)] = -1;

        // ミサイルの位置をワールド行列から取得
        m_positions[static_cast<int>(Parts::MISSILE)] = 
            m_worldMatrices[static_cast<int>(Parts::MISSILE)].Translation();

        // ミサイルの回転をワールド行列から取得
        m_rotates[static_cast<int>(Parts::MISSILE)] =
            SimpleMath::Quaternion::CreateFromRotationMatrix(m_worldMatrices[static_cast<int>(Parts::MISSILE)]);

    }

    // ミサイルが発射中なら
    if (m_fireFlag)
    {
        // ミサイルを回転させる
        m_rotates[static_cast<int>(Parts::MISSILE)] =
              SimpleMath::Quaternion::CreateFromAxisAngle(SimpleMath::Vector3::UnitZ, XMConvertToRadians(2.0f))
            * m_rotates[static_cast<int>(Parts::MISSILE)];

        // ミサイルを移動させる
        SimpleMath::Vector3 velocity(0.0f, 0.0f, -0.01f);   // ミサイルの速度ベクトル
        m_positions[static_cast<int>(Parts::MISSILE)] +=
            SimpleMath::Vector3::Transform(velocity, m_rotates[static_cast<int>(Parts::MISSILE)]);
    }

    // ミサイルをリセット
    if (m_inputRobot->IsTriggered("ResetMissile"))
    {
        m_fireFlag = false;
        m_parentIndexes[static_cast<int>(Parts::MISSILE)] = static_cast<int>(Parts::ARM_L);
        m_positions[static_cast<int>(Parts::MISSILE)] = SimpleMath::Vector3::Zero;
        m_rotates[static_cast<int>(Parts::MISSILE)] = SimpleMath::Quaternion::Identity;
    }

    // ----- 床とロボットの位置の線分との交差判定 ----- //
    // ロボットの位置の線分
    SimpleMath::Vector3 line[2] = {
        { m_positions[static_cast<int>(Parts::LEG)].x,  5.0f, m_positions[static_cast<int>(Parts::LEG)].z },
        { m_positions[static_cast<int>(Parts::LEG)].x, -5.0f, m_positions[static_cast<int>(Parts::LEG)].z },
    };

    // 床のモデルと線分の交差判定
    SimpleMath::Vector3 hitPosition;
    SimpleMath::Vector3 normal;
    if (floor->IntersectLineSegment(line[0], line[1], &hitPosition, &normal))
    {
        // ロボットのY座標を設定する
        SetPositionY(hitPosition.y);

        // 床の法線方向へ傾けるクォータニオンを作成する
        SimpleMath::Quaternion q = SimpleMath::Quaternion::FromToRotation(SimpleMath::Vector3::Up, normal);

        // ロボットを傾ける
        SetTilt(q);
    }
    // ------------------------------------------------ //

    // 各パーツのワールド行列を作成
    UpdateWorldMatrices();

    // コリジョン情報を更新
    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        // パーツのワールド行列から移動量を取得
        SimpleMath::Vector3 position = m_worldMatrices[i].Translation();
        // パーツのワールド行列から回転量を取得
        SimpleMath::Quaternion rotation = SimpleMath::Quaternion::CreateFromRotationMatrix(m_worldMatrices[i]);
        // コリジョン情報を移動と回転をする
        m_modelCollisions[i]->UpdateBoundingInfo(position, rotation);
    }
}

// 描画
void Robot::Render()
{
    // 各パーツの描画
    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        m_pModels[i]->Draw( m_gameContext.deviceResources.GetD3DDeviceContext(), m_gameContext.commonStates
                          , m_worldMatrices[i], m_view, m_projection );

    }
}

// 位置を取得する関数
DirectX::SimpleMath::Vector3 Robot::GetPosition() const
{
    return m_positions[static_cast<int>(Parts::LEG)];
}

// コリジョン情報を表示する関数
void Robot::DisplayCollision(Imase::CollisionRenderer* pCollisionRenderer)
{
    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        m_modelCollisions[i]->AddCollisionRenderer(pCollisionRenderer);
    }
}

// Rayとの交差判定関数
bool Robot::IsHitRay(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 direction)
{
    float dis;

    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        if (m_modelCollisions[i]->Intersects(origin, direction, dis))
        {
            return true;
        }
    }
    return false;
}

// Frustumとの交差判定関数
bool Robot::IsHitFrustum(DirectX::BoundingFrustum frustum)
{
    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        if (m_modelCollisions[i]->Intersects(frustum))
        {
            return true;
        }
    }
    return false;
}

// 各パーツのワールド行列を更新する関数
void Robot::UpdateWorldMatrices()
{
    // 各パーツのワールド行列を作成
    for (size_t i = 0; i < PARTS_CNT; i++)
    {
        // 各パーツの移動と回転から行列を作成
        m_worldMatrices[i] = SimpleMath::Matrix::CreateFromQuaternion(m_rotates[i]) *
                             SimpleMath::Matrix::CreateTranslation(m_positions[i]);

        // 親がいれば
        if (m_parentIndexes[i] != -1)
        {
            // 初期化行列をかける
            m_worldMatrices[i] *= m_initializeMatrices[i];
            // 親のワールド行列をかける
            m_worldMatrices[i] *= m_worldMatrices[m_parentIndexes[i]];
        }
    }
}
