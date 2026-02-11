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

// テクスチャロード関数
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Imase::Model::LoadTexture(ID3D11Device* device, const std::wstring& path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(device, path.c_str(), nullptr, srv.GetAddressOf())
	);

	return srv;
}

// コンストラクタ
Imase::Model::Model(Imase::Effect* pEffect)
	: m_pEffect(pEffect)
{
}

// フルパスからディレクトリ部分を取得する
static std::wstring GetDirectoryFromPath(const std::wstring& fullPath)
{
	// 最後の / または \ を探す
	size_t pos = fullPath.find_last_of(L"/\\");

	if (pos == std::wstring::npos)
	{
		// ディレクトリなし
		return L"";
	}

	// 区切り文字を含めたディレクトリ部分を返す
	return fullPath.substr(0, pos + 1);
}

// モデルデータ作成関数
std::unique_ptr<Imase::Model> Imase::Model::CreateModel(ID3D11Device* device, std::wstring fname, Imase::Effect* pEffect)
{
	std::vector<uint8_t> data = DX::ReadData(fname.c_str());
	return CreateModel(device, data.data(), pEffect, GetDirectoryFromPath(fname));
}

// モデルデータ作成関数
std::unique_ptr<Imase::Model>  Imase::Model::CreateModel
(
	ID3D11Device* device,
	const uint8_t* meshData,
	Imase::Effect* pEffect,
	std::wstring path
)
{
	auto model = std::make_unique<Model>(pEffect);
	auto& textures = model->m_textures;

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

		// テクスチャをロード
		std::wstring fullPath = path + name;

		// テクスチャハンドルを登録
		textures.push_back(LoadTexture(device, fullPath));
	}

	// マテリアル名の数
	const uint32_t* materialNam_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
	usedSize += sizeof(uint32_t);
	model->m_materialNames.resize(*materialNam_cnt);

	for (uint32_t i = 0; i < (*materialNam_cnt); i++)
	{
		const uint32_t* len = reinterpret_cast<const uint32_t*>(meshData + usedSize);
		usedSize += sizeof(uint32_t);
		const char* materialName = reinterpret_cast<const char*>(meshData + usedSize);
		usedSize += (*len);
		std::string str;
		str.assign(materialName, *len);
		model->m_materialNames[i]= StringToWString(str);
	}

	// マテリアル名→インデックスの検索用テーブルを作成
	for (uint32_t i = 0; i < model->m_materialNames.size(); i++)
	{
		model->m_materialIndexMap[model->m_materialNames[i]] = i;
	}

	// マテリアル数
	const uint32_t* material_cnt = reinterpret_cast<const uint32_t*>(meshData + usedSize);
	usedSize += sizeof(uint32_t);
	model->m_materials.reserve(*material_cnt);

	struct MaterialData
	{
		DirectX::XMFLOAT3 ambientColor;     // アンビエント色（使用しない）
		DirectX::XMFLOAT3 diffuseColor;     // ディフューズ色
		DirectX::XMFLOAT3 specularColor;    // スペキュラー色
		float specularPower;                // スペキュラーパワー
		DirectX::XMFLOAT3 emissiveColor;    // エミッシブ色
		int32_t textureIndex_BaseColor;     // テクスチャインデックス（ベースカラー）
		int32_t textureIndex_NormalMap;     // テクスチャインデックス（法線マップ）
	};

	// マテリアル
	for (uint32_t i = 0; i < *material_cnt; i++)
	{
		MaterialData temp;
		std::memcpy(&temp, meshData + usedSize, sizeof(MaterialData));

		Imase::Material m = {};
		m.diffuseColor  = { temp.diffuseColor.x,  temp.diffuseColor.y,  temp.diffuseColor.z,  1.0f };
		m.emissiveColor = { temp.emissiveColor.x, temp.emissiveColor.y, temp.emissiveColor.z, 1.0f };
		m.specularColor = { temp.specularColor.x, temp.specularColor.y, temp.specularColor.z, 1.0f };
		m.specularPower = temp.specularPower;
		if (temp.textureIndex_BaseColor >= 0) m.pBaseColorSRV = textures[temp.textureIndex_BaseColor].Get();
		if (temp.textureIndex_NormalMap >= 0) m.pNormalMapSRV = textures[temp.textureIndex_NormalMap].Get();
		model->m_materials.emplace_back(m);

		usedSize += sizeof(MaterialData);
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
void Imase::Model::Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX world)
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
		m_pEffect->SetMaterial(m_materials[mesh.materialIndex]);
		m_pEffect->SetWorld(world);
		m_pEffect->Apply(context);

		context->DrawIndexed(mesh.primCount * 3, mesh.startIndex, 0);
	}
}

// 指定マテリアルのディフューズ色を設定する関数
void Imase::Model::SetDiffuseColorByName(const std::wstring& name, const DirectX::XMVECTOR& color)
{
	auto it = m_materialIndexMap.find(name);
	if (it == m_materialIndexMap.end()) return;
	XMStoreFloat4(&m_materials[it->second].diffuseColor, color);
}

