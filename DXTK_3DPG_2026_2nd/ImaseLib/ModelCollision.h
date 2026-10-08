//--------------------------------------------------------------------------------------
// File: ModelCollision.h
//
// モデル用衝突判定クラス
//
// Date: 2023.6.13
// Author: S.Takaki
//
//--------------------------------------------------------------------------------------
#pragma once

#include "CollisionRenderer.h"
#include <functional>

namespace Imase
{
	// 衝突判定形状のインタフェース
	class ICollisionGeometry
	{
	public:

		using Collection = std::vector<std::unique_ptr<ICollisionGeometry>>;

	public:

		// デストラクタ
		virtual ~ICollisionGeometry() = default;

		// 更新
		virtual void Update(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Quaternion& rotate) = 0;

		// 衝突判定の表示に登録する関数
		virtual void AddCollisionRenderer(Imase::CollisionRenderer* collisionRenderer) const = 0;

		// 衝突判定を行う関数 (ディスパッチ用)
		virtual bool Intersects(const std::unique_ptr<ICollisionGeometry>& geometry) const = 0;

		// 衝突判定を行う関数 (実際の判定用)
		virtual bool Intersects(const DirectX::BoundingSphere& geometry) const = 0;
		virtual bool Intersects(const DirectX::BoundingBox& geometry) const = 0;
        virtual bool Intersects(const DirectX::BoundingOrientedBox& geometry) const = 0;
        virtual bool Intersects(const DirectX::BoundingFrustum& geometry) const = 0;

		// 衝突判定を行う関数 (対 Triangle)
        virtual bool Intersects(DirectX::FXMVECTOR V0, DirectX::FXMVECTOR V1, DirectX::FXMVECTOR V2) const = 0;
        // 衝突判定を行う関数 (対 Plane)
        virtual DirectX::PlaneIntersectionType Intersects(DirectX::FXMVECTOR Plane) const = 0;
        // 衝突判定を行う関数 (対 Ray)
		virtual bool Intersects(DirectX::FXMVECTOR origin, DirectX::FXMVECTOR direction, float& dist) const = 0;
	};

	// 衝突判定のクラス
	class ModelCollision
	{
		using GeometryFactory = std::function<ICollisionGeometry::Collection(const DirectX::Model* pModel)>;

	public:

		// コリジョンのタイプ
		enum class CollisionType
		{
			None,
			Sphere,
			AABB,
			OBB,
		};

	public:

		// コリジョンタイプ
		CollisionType m_type;

		// 衝突判定情報
		ICollisionGeometry::Collection m_geometries;

	public:

		// コンストラクタ
		ModelCollision(CollisionType type, const DirectX::Model* pModel, GeometryFactory geometryFactory);
		ModelCollision(CollisionType type, ICollisionGeometry::Collection&& geometries);

		// デストラクタ
		~ModelCollision() = default;

		// ----- 衝突判定を行う関数 ---- //
		// 対 モデル
		bool Intersects(const ModelCollision* collision) const;
        // 対 Triangle
        bool Intersects(DirectX::FXMVECTOR V0, DirectX::FXMVECTOR V1, DirectX::FXMVECTOR V2) const;
        // 対 Plane
        DirectX::PlaneIntersectionType Intersects(DirectX::FXMVECTOR Plane) const;
		// 対 Ray
        bool Intersects(DirectX::FXMVECTOR origin, DirectX::FXMVECTOR direction, float& dist) const;
        // 対 視錐台
		bool Intersects(const DirectX::BoundingFrustum& frustum) const;

		// コリジョンタイプの取得関数
		CollisionType GetType() const { return m_type; }

		// コリジョン情報の更新
		void UpdateBoundingInfo(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Quaternion& rotate);

		// 衝突判定の表示に登録する関数
		void AddCollisionRenderer(Imase::CollisionRenderer* collisionRenderer) const;
	};

	// 衝突判定生成クラス
	class ModelCollisionFactory
	{
	public:

		static std::unique_ptr<ModelCollision> CreateCollision(const DirectX::Model* pModel, ModelCollision::CollisionType type = ModelCollision::CollisionType::Sphere);
	};
}

