#include "pch.h"
#include "Model.h"

using namespace DirectX;
using namespace Imase;

#include "DDSTextureLoader.h"

// UTF-8 → UTF-16 変換
static std::wstring StringToWString(const std::string& s)
{
	if (s.empty()) return L"";

	int size = MultiByteToWideChar(
		CP_UTF8,                // 入力はUTF-8
		0,
		s.data(),
		static_cast<int>(s.size()),
		nullptr,
		0
	);

	std::wstring result(size, 0);

	MultiByteToWideChar(
		CP_UTF8,
		0,
		s.data(),
		static_cast<int>(s.size()),
		&result[0],
		size
	);

	return result;
}

// 拡張子をddsに変更する関数
static std::string ChangeExtPngToDds(const std::string& filename)
{
	std::string result = filename;

	// 最後の '.' を探す
	size_t pos = result.find_last_of('.');
	if (pos == std::string::npos)
	{
		// 拡張子なし → .dds を追加
		return result + ".dds";
	}

	// 拡張子を置き換え
	result.replace(pos, std::string::npos, ".dds");

	return result;
}

// コンストラクタ
Imase::Model::Model(Imase::Effect* pEffect)
	: m_pEffect(pEffect)
{
}

// モデルデータ作成関数
std::unique_ptr<Imase::Model>  Imase::Model::CreateModel
(
	ID3D11Device* device,
	const uint8_t* meshData,
	Imase::Effect* pEffect
)
{
	auto model = std::make_unique<Model>(pEffect);

	size_t usedSize = 0;

	// テクスチャ数
	const uint32_t* texture_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
	usedSize += sizeof(uint32_t);

	for (uint32_t i = 0; i < *texture_cnt; i++)
	{
		// テクスチャファイル名の文字数
		const uint32_t* nName = reinterpret_cast<const uint32_t*>(meshData + usedSize);
		usedSize += sizeof(uint32_t);

		// テクスチャファイル名
		const char* texName = reinterpret_cast<const char*>(meshData + usedSize);
		usedSize += (*nName);

		std::string str;
		str.assign(texName, *nName);

		// 拡張子をddsに変更
		str = ChangeExtPngToDds(str);

		// UTF-8 → UTF-16 変換
		std::wstring name = StringToWString(str);

		// テクスチャを登録
		pEffect->SetTexture(device, name.c_str());
	}

	// マテリアル数
	const uint32_t* material_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
	usedSize += sizeof(uint32_t);
	model->m_materials.reserve(*material_cnt);

	// マテリアル
	for (uint32_t i = 0; i < *material_cnt; i++)
	{
		Imase::Material temp;
		std::memcpy(&temp, meshData + usedSize, sizeof(Imase::Material));
		model->m_materials.emplace_back(temp);
		usedSize += sizeof(Imase::Material);
	}

	// メッシュ情報数
	const uint32_t* mesh_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
	model->m_meshes.reserve(*mesh_cnt);
	usedSize += sizeof(uint32_t);

	// メッシュ情報
	for (uint32_t i = 0; i < *mesh_cnt; i++)
	{
		Mesh temp;
		std::memcpy(&temp, meshData + usedSize, sizeof(Mesh));
		model->m_meshes.emplace_back(temp);
		usedSize += sizeof(Mesh);
	}

	// インデックスバッファ作成
	{
		const uint32_t* index_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
		usedSize += sizeof(uint32_t);

		// インデックス頂点バッファの作成
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(uint16_t) * (*index_cnt);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = meshData + usedSize;

		DX::ThrowIfFailed(
			device->CreateBuffer(&desc, &data, model->m_indexBuffer.ReleaseAndGetAddressOf())
		);
		usedSize += sizeof(uint16_t) * (*index_cnt);
	}

	// 頂点バッファの作成
	{
		const uint32_t* vertex_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
		usedSize += sizeof(uint32_t);

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(VertexPositionNormalTextureTangent) * (*vertex_cnt);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = meshData + usedSize;

		DX::ThrowIfFailed(
			device->CreateBuffer(&desc, &data, model->m_vertexBuffer.ReleaseAndGetAddressOf())
		);
		usedSize += sizeof(VertexPositionNormalTextureTangent) * (*vertex_cnt);
	}

	return model;
}

// 描画関数
void Imase::Model::Draw
(
	ID3D11DeviceContext* context,
	DirectX::XMMATRIX world,
	DirectX::XMMATRIX view,
	DirectX::XMMATRIX projection
)
{
	// 頂点バッファの設定
	ID3D11Buffer* buffers[] = { m_vertexBuffer.Get() };
	UINT stride = sizeof(Imase::VertexPositionNormalTextureTangent);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, buffers, &stride, &offset);

	// インデックスバッファの設定
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// トポロジーの設定
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// メッシュ描画
	for (auto& mesh : m_meshes)
	{
		// エフェクトの設定
		m_pEffect->SetMaterial(&m_materials[mesh.materialIndex]);
		m_pEffect->SetWorld(world);
		m_pEffect->SetView(view);
		m_pEffect->SetProjection(projection);
		m_pEffect->Apply(context);

		context->DrawIndexed(mesh.primCount * 3, mesh.startIndex, 0);
	}
}

// エフェクトの更新
void Imase::Model::UpdateEffect(std::function<void(Imase::Effect*)> setEffect)
{
	if (setEffect) setEffect(m_pEffect);
}

