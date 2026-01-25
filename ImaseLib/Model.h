#pragma once

#include "Effect.h"

namespace Imase
{
	// 頂点情報
	struct VertexPositionNormalTextureTangent
	{
		DirectX::XMFLOAT3 position;    // 位置
		DirectX::XMFLOAT3 normal;      // 法線
		DirectX::XMFLOAT2 texcoord;    // テクスチャ座標
		DirectX::XMFLOAT4 tangent;     // 接線（w：UVが反転しているかどうかを表す符号）
	};

	// メッシュ情報
	struct Mesh
	{
		uint32_t materialIndex;     // マテリアルインデックス
		uint32_t startIndex;        // スタートインデックス  
		uint32_t primCount;         // プリミティブ数
	};

	// モデルクラス
	class Model
	{
	private:

		// エフェクトへのポインタ
		Imase::Effect* m_pEffect;

		// マテリアル
		std::vector<Imase::Material> m_materials;

		// メッシュ情報
		std::vector<Mesh> m_meshes;

		// 頂点バッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

		// インデックスバッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	public:

		// コンストラクタ
		Model(Imase::Effect* pEffect);

		// モデルデータ作成関数
		static std::unique_ptr<Imase::Model> CreateModel(ID3D11Device* device, const uint8_t* meshData, Imase::Effect* pEffect);

		// 描画関数
		void Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection);

		// エフェクトの更新
		void UpdateEffect(std::function<void (Imase::Effect*)> setEffect);

	};
}
