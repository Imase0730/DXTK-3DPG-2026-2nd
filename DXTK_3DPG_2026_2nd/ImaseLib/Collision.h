//--------------------------------------------------------------------------------------
// File: Collision.h
//
// 衝突判定用コリジョンクラス
// 備考：ゲームプログラミングのためのリアルタイム衝突判定を参考に作成
//
// Date: 2023.6.15
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

namespace Imase
{
	class Collision
	{
	public:
		//------------------------------//
		// 線分と三角形の交差判定用		//
		//------------------------------//

		// 三角形の構造体（線分と三角形の交差判定用）
		struct Triangle
		{
			// 三角形の平面方程式
			DirectX::SimpleMath::Plane p;
			// 辺BCの平面方程式（重心座標の頂点aに対する重みuを与える）
			DirectX::SimpleMath::Plane edgePlaneBC;
			// 辺CAの平面方程式（重心座標の頂点bに対する重みvを与える）
			DirectX::SimpleMath::Plane edgePlaneCA;

			// 各頂点の法線
            DirectX::SimpleMath::Vector3 normal[3];

			// コンストラクタ内で衝突判定を軽くするために前処理で計算しておく
			Triangle(const DirectX::VertexPositionNormal* v)
			{
				// 各頂点の位置
                DirectX::SimpleMath::Vector3 a(v[0].position);
                DirectX::SimpleMath::Vector3 b(v[1].position);
                DirectX::SimpleMath::Vector3 c(v[2].position);
                
				// 各頂点の法線
				normal[0] = v[0].normal;
                normal[1] = v[1].normal;
                normal[2] = v[2].normal;

				DirectX::SimpleMath::Vector3 n = (c - a).Cross(b - a);
				p = DirectX::SimpleMath::Plane(a, n);
				DirectX::SimpleMath::Plane pp = DirectX::SimpleMath::Plane(b, n);
				edgePlaneBC = DirectX::SimpleMath::Plane(b, n.Cross(b - c));
				edgePlaneCA = DirectX::SimpleMath::Plane(c, n.Cross(c - a));
				p.Normalize(); edgePlaneBC.Normalize(); edgePlaneCA.Normalize();
				float bc_scale = 1.0f / (a.Dot(edgePlaneBC.Normal()) + edgePlaneBC.D());
				float ca_scale = 1.0f / (b.Dot(edgePlaneCA.Normal()) + edgePlaneCA.D());
				edgePlaneBC.x *= bc_scale; edgePlaneBC.y *= bc_scale; edgePlaneBC.z *= bc_scale; edgePlaneBC.w *= bc_scale;
				edgePlaneCA.x *= ca_scale; edgePlaneCA.y *= ca_scale; edgePlaneCA.z *= ca_scale; edgePlaneCA.w *= ca_scale;
			}
		};

		/// <summary>
		/// 線分と三角形の交差判定
		/// </summary>
		/// <param name="p">線分の始点</param>
		/// <param name="q">線分の終点</param>
		/// <param name="tri">三角形</param>
		/// <param name="s">線分と三角形の交差点</param>
		/// <param name="normal">交差点における補間された法線（出力用・不要ならnullptr可）</param>
		/// <returns>trueの場合交差している</returns>
		static bool IntersectSegmentTriangle(
			DirectX::SimpleMath::Vector3 p,
			DirectX::SimpleMath::Vector3 q,
			Triangle tri,
			DirectX::SimpleMath::Vector3* s,
            DirectX::SimpleMath::Vector3* normal = nullptr
		)
		{
			// 浮動小数点の誤差で当たりぬけするので少し余裕をもつ
			constexpr float EPSILON = 1.0e-06F;
			float distp = p.Dot(tri.p.Normal()) + tri.p.D();
			if (distp < 0.0f) return false;
			float distq = q.Dot(tri.p.Normal()) + tri.p.D();
			if (distq >= 0.0f) return false;
			float denom = distp - distq;
			float t = distp / denom;
			*s = p + t * (q - p);

			// 重心座標 u (頂点aの重み)
			float u = s->Dot(tri.edgePlaneBC.Normal()) + tri.edgePlaneBC.D();
			if (fabsf(u) < EPSILON) u = 0.0f;
			if (u < 0.0f || u > 1.0f) return false;
            
			// 重心座標 v (頂点bの重み)
			float v = s->Dot(tri.edgePlaneCA.Normal()) + tri.edgePlaneCA.D();
			if (fabsf(v) < EPSILON) v = 0.0f;
			if (v < 0.0f) return false;

			// 重心座標 w (頂点cの重み)
			float w = 1.0f - u - v;
			if (fabsf(w) < EPSILON) w = 0.0f;
			if (w < 0.0f) return false;

			// --- 法線の補間処理を追加 ---
            if (normal != nullptr)
            {
                // 各頂点の法線に重心座標の重みをかけて足し合わせる
                *normal = u * tri.normal[0] + v * tri.normal[1] + w * tri.normal[2];
				// 単位行列化
                normal->Normalize();
            }

			return true;
		}

	};
}


