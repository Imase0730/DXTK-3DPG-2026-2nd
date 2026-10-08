//--------------------------------------------------------------------------------------
// File: ObjCollision.h
//
// Obj形式のメッシュをコリジョンにするクラス
//
// Date: 2018.7.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "Collision.h"
#include "CollisionRenderer.h"

namespace Imase
{
	// Obj形式のメッシュをコリジョンにするクラス
	class ObjCollision
	{
	private:

		// 位置
		DirectX::SimpleMath::Vector3 m_position;

		// 回転
		DirectX::SimpleMath::Quaternion m_rotation;

	private:

		// 頂点情報
		std::vector<DirectX::VertexPositionNormal> m_vertexes;

		// インデックス情報
		std::vector<uint16_t> m_indexes;

		// コリジョン用三角形データ
		std::vector<Imase::Collision::Triangle> m_triangles;

		// コリジョン用三角形データの追加関数
        void AddTriangle(const DirectX::VertexPositionNormal* v);

	public:

		// コンストラクタ
        ObjCollision(const char* fname);

		// コリジョン情報の更新
		void UpdateBoundingInfo(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Quaternion& rotate);

		// 線分との交差判定関数
		bool IntersectLineSegment(
			DirectX::SimpleMath::Vector3 a,
			DirectX::SimpleMath::Vector3 b,
			DirectX::SimpleMath::Vector3* hitPosition,
			DirectX::SimpleMath::Vector3* normal = nullptr
		);

		// 衝突判定の表示に登録する関数
		void AddCollisionRenderer(Imase::CollisionRenderer* collisionRenderer, DirectX::FXMVECTOR lineColor = DirectX::XMVECTORF32{ 0.0f, 0.0f, 0.0f, 0.0f }) const;

	};
}

