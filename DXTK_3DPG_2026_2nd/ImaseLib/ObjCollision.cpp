//--------------------------------------------------------------------------------------
// File: CollisionMesh.cpp
//
// Obj形式のメッシュをコリジョンにするクラス
//
// Date: 2018.7.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ObjCollision.h"
#include <fstream>
#include <map>
#include <format>

using namespace DirectX;
using namespace Imase;

// 頂点インデックスを保持する構造体
struct FaceVertex
{
    int v = 0;  // 頂点座標インデックス
    int vt = 0; // テクスチャ座標インデックス (0は「なし」を意味する)
    int vn = 0; // 法線ベクトルインデックス (0は「なし」を意味する)
};

// 文字列" f 1/2/3 1/2/3 1/2/3"の各インデックス情報を取得する関数
static void ParseFaceIndex(const std::string& str, std::vector<FaceVertex>& faceVertices)
{
    const char* cstr = str.c_str();

	// 先頭の "f " の分、ポインタを進める
    cstr += 2;

    int offset = 0;
    int read_bytes = 0;

    while (true)
    {
        FaceVertex fv;

        // パターン1: v/vt/vn
        if (sscanf_s(cstr + offset, "%d/%d/%d%n", &fv.v, &fv.vt, &fv.vn, &read_bytes) == 3)
        {
            // 成功
        }
        // パターン2: v//vn (テクスチャなし)
        else if (sscanf_s(cstr + offset, "%d//%d%n", &fv.v, &fv.vn, &read_bytes) == 2)
        {
            fv.vt = 0;
        }
        // パターン3: v/vt (法線なし)
        else if (sscanf_s(cstr + offset, "%d/%d%n", &fv.v, &fv.vt, &read_bytes) == 2)
        {
            fv.vn = 0;
        }
        // パターン4: v (頂点のみ)
        else if (sscanf_s(cstr + offset, "%d%n", &fv.v, &read_bytes) == 1)
        {
            fv.vt = 0;
            fv.vn = 0;
        }
        else
        {
            // これ以上解析できる数字がなければループを抜ける
            break;
        }

        // 解析できた頂点データを追加
        faceVertices.push_back(fv);

        // 読み進めた文字数分、オフセットを更新する
        offset += read_bytes;

        // 次の文字がスペースならスキップ、末尾（\0）なら終了
        while (cstr[offset] == ' ')
        {
            offset++;
        }
        if (cstr[offset] == '\0')
        {
            break;
        }
    }
}

// コンストラクタ
ObjCollision::ObjCollision(const char* fname)
{
    std::ifstream ifs(fname);
    if (!ifs.is_open())
    {
        // 例外を投げて処理を中断する
        throw std::runtime_error(std::format("Failed to open the file.: {}", fname));
    }

    // ファイルから読み込んだ生のデータを一時保存する配列
    std::vector<DirectX::SimpleMath::Vector3> raw_positions;
    std::vector<DirectX::SimpleMath::Vector3> raw_normals;

    // クラスのメンバ変数をクリア（初期化）
    m_vertexes.clear();
    m_indexes.clear();

    std::string str;
    // ファイルを1行ずつ読み込む
    while (std::getline(ifs, str))
    {
        // 頂点座標
        if (str.starts_with("v "))
        {
            DirectX::SimpleMath::Vector3 pos = {};
            sscanf_s(str.data(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
            raw_positions.push_back(pos);
        }
        // 法線
        else if (str.starts_with("vn "))
        {
            DirectX::SimpleMath::Vector3 n = {};
            sscanf_s(str.data(), "vn %f %f %f", &n.x, &n.y, &n.z);
            raw_normals.push_back(n);
        }
        // 面情報（インデックス）
        else if (str.starts_with("f "))
        {
            std::vector<FaceVertex> faceIndex;
            ParseFaceIndex(str, faceIndex);

            // 3つ以上の頂点がある場合のみ処理（不正なデータを除外）
            if (faceIndex.size() >= 3)
            {
                // 「(座標インデックス,
                // 法線インデックス)」のペアから「m_vertexesの新しいインデックス」を引くためのマップ
                static std::map<std::pair<int, int>, uint16_t> vertexMap;

                // 最初の処理時にマップをクリア
                if (m_indexes.empty())
                    vertexMap.clear();

                // 面を構成する全頂点を一時的に格納する配列
                std::vector<uint16_t> currentFaceIndices;

                for (size_t i = 0; i < faceIndex.size(); i++)
                {
                    int vIdx = faceIndex[i].v - 1;
                    int vnIdx = faceIndex[i].vn - 1;

                    std::pair<int, int> vertexKey = {vIdx, vnIdx};

                    // すでに同じ「座標＋法線」の組み合わせが登録されているか確認
                    auto it = vertexMap.find(vertexKey);
                    if (it != vertexMap.end())
                    {
                        // 既存の頂点があるなら、そのインデックスを再利用
                        currentFaceIndices.push_back(it->second);
                    }
                    else
                    {
                        // 新しい組み合わせなので、頂点を作成して m_vertexes に追加
                        DirectX::VertexPositionNormal newVertex = {};

                        // 座標の設定
                        if (vIdx >= 0 && vIdx < (int)raw_positions.size())
                        {
                            newVertex.position = raw_positions[vIdx];
                        }

                        // 法線の設定（法線がない場合は仮の法線を入れる）
                        if (vnIdx >= 0 && vnIdx < (int)raw_normals.size())
                        {
                            newVertex.normal = raw_normals[vnIdx];
                        }
                        else
                        {
                            newVertex.normal = DirectX::SimpleMath::Vector3::Up;
                        }

                        // 新しい頂点のインデックス番号を取得
                        uint16_t newIndex = static_cast<uint16_t>(m_vertexes.size());
                        m_vertexes.push_back(newVertex);
                        currentFaceIndices.push_back(newIndex);

                        // マップに記録して次回から再利用できるようにする
                        vertexMap[vertexKey] = newIndex;
                    }
                }

                // --- ここから三角形・四角形ポリゴンの切り分け処理 ---

                // 三角形ポリゴンの場合 (頂点数が3つ)
                if (currentFaceIndices.size() == 3)
                {
                    m_indexes.push_back(currentFaceIndices[0]);
                    m_indexes.push_back(currentFaceIndices[1]);
                    m_indexes.push_back(currentFaceIndices[2]);
                }
                // 四角形ポリゴンの場合 (頂点数が4つ)
                else if (currentFaceIndices.size() == 4)
                {
                    // 1つ目の三角形 (0, 1, 2)
                    m_indexes.push_back(currentFaceIndices[0]);
                    m_indexes.push_back(currentFaceIndices[1]);
                    m_indexes.push_back(currentFaceIndices[2]);

                    // 2つ目の三角形 (0, 2, 3)
                    m_indexes.push_back(currentFaceIndices[0]);
                    m_indexes.push_back(currentFaceIndices[2]);
                    m_indexes.push_back(currentFaceIndices[3]);
                }
            }
        }
    }
    ifs.close();

    // 三角形リスト（Collision用）に登録
    size_t triangleCount = m_indexes.size() / 3;
    for (size_t i = 0; i < triangleCount; i++)
    {
        // Collision::Triangle(const DirectX::VertexPositionNormal* v) に渡すための3頂点配列
        DirectX::VertexPositionNormal v[3];
        v[0] = m_vertexes[m_indexes[i * 3 + 0]];
        v[1] = m_vertexes[m_indexes[i * 3 + 2]];
        v[2] = m_vertexes[m_indexes[i * 3 + 1]];

        // 3頂点の配列の先頭ポインタをそのまま渡して登録
        AddTriangle(v);
    }
}

// コリジョン用三角形データの追加関数
void ObjCollision::AddTriangle(const DirectX::VertexPositionNormal* v)
{
	Imase::Collision::Triangle t(v);
	m_triangles.push_back(t);
}

// コリジョン情報の更新
void ObjCollision::UpdateBoundingInfo(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Quaternion& rotate)
{
	m_position = position;
	m_rotation = rotate;
}

// 線分との交差判定関数
bool ObjCollision::IntersectLineSegment(
	DirectX::SimpleMath::Vector3 a,
	DirectX::SimpleMath::Vector3 b,
	DirectX::SimpleMath::Vector3* hitPosition,
	DirectX::SimpleMath::Vector3* normal
)
{
	// 線分に逆行列を掛ける
	SimpleMath::Matrix rotate = SimpleMath::Matrix::CreateFromQuaternion(m_rotation);
	SimpleMath::Matrix trans = SimpleMath::Matrix::CreateTranslation(m_position);
	SimpleMath::Matrix world = rotate * trans;
	SimpleMath::Matrix matInvert = world.Invert();
	a = SimpleMath::Vector3::Transform(a, matInvert);
	b = SimpleMath::Vector3::Transform(b, matInvert);

	for (int i = 0; i < m_triangles.size(); i++)
	{
		if (Imase::Collision::IntersectSegmentTriangle(a, b, m_triangles[i], hitPosition, normal) == true)
		{
			// 衝突位置をワールド座標系へ変換
			*hitPosition = SimpleMath::Vector3::Transform(*hitPosition, world);
			// 衝突した三角形の法線情報を取得
			if (normal) *normal = SimpleMath::Vector3::Transform(*normal, rotate);
			return true;
		}
	}
	return false;
}

void ObjCollision::AddCollisionRenderer(Imase::CollisionRenderer* collisionRenderer, DirectX::FXMVECTOR lineColor) const
{
	collisionRenderer->AddBoundingVolume(m_vertexes, m_indexes, m_position, m_rotation, lineColor);
}
