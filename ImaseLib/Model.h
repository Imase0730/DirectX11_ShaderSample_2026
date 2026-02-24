//--------------------------------------------------------------------------------------
// File: Model.h
//
// Imdl形式のモデルデータを描画するクラス
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "Effect.h"
#include "Imdl.h"

namespace Imase
{
	// モデルクラス
	class Model
	{
	private:

		// エフェクトへのポインタ
		Imase::Effect* m_pEffect;

		// メッシュ情報
		std::vector<Imase::SubMeshInfo> m_subMeshes;

		// メッシュグループ情報
		std::vector<Imase::MeshGroupInfo> m_meshGroups;

		// ノード情報
		std::vector<Imase::NodeInfo> m_nodes;

		// 頂点バッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

		// インデックスバッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

		// ラスタライザーステート
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		// 深度ステンシルステート
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		// ブレンドステート
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

	public:

		// コンストラクタ
		Model(ID3D11Device* device, Imase::Effect* pEffect);

		// モデルデータ作成関数
		static std::unique_ptr<Imase::Model> CreateFromImdl(ID3D11Device* device, std::wstring fname, Imase::Effect* pEffect);

		// 描画関数
		void Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX world);

		// エフェクトを取得する関数
		Imase::Effect* GetEffect() const { return m_pEffect; }

	};
}
