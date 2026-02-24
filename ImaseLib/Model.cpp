//--------------------------------------------------------------------------------------
// File: Model.cpp
//
// Imdl形式のモデルデータを描画するクラス
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Model.h"
#include "Imdl.h"
#include "ChunkIO.h"
#include "BinaryReader.h"

using namespace DirectX;
using namespace Imase;

#include "DDSTextureLoader.h"

// --------------------------------------------------------------------------------------------- //
// Imdl形式のローダー
// --------------------------------------------------------------------------------------------- //
static MaterialInfo DeserializeMaterial(BinaryReader& reader)
{
	MaterialInfo m{};

	m.diffuseColor.x = reader.ReadFloat();
	m.diffuseColor.y = reader.ReadFloat();
	m.diffuseColor.z = reader.ReadFloat();
	m.diffuseColor.w = reader.ReadFloat();

	m.metallicFactor = reader.ReadFloat();
	m.roughnessFactor = reader.ReadFloat();

	m.emissiveColor.x = reader.ReadFloat();
	m.emissiveColor.y = reader.ReadFloat();
	m.emissiveColor.z = reader.ReadFloat();

	m.baseColorTexIndex = reader.ReadInt32();
	m.normalTexIndex = reader.ReadInt32();
	m.metalRoughTexIndex = reader.ReadInt32();
	m.emissiveTexIndex = reader.ReadInt32();

	return m;
}

static VertexPositionNormalTextureTangent DeserializeVertex(BinaryReader& reader)
{
	VertexPositionNormalTextureTangent v{};

	v.position.x = reader.ReadFloat();
	v.position.y = reader.ReadFloat();
	v.position.z = reader.ReadFloat();

	v.normal.x = reader.ReadFloat();
	v.normal.y = reader.ReadFloat();
	v.normal.z = reader.ReadFloat();

	v.texcoord.x = reader.ReadFloat();
	v.texcoord.y = reader.ReadFloat();

	v.tangent.x = reader.ReadFloat();
	v.tangent.y = reader.ReadFloat();
	v.tangent.z = reader.ReadFloat();
	v.tangent.w = reader.ReadFloat();

	return v;
}

static SubMeshInfo DeserializeSubMesh(BinaryReader& reader)
{
	SubMeshInfo m{};
	m.startIndex = reader.ReadUInt32();
	m.indexCount = reader.ReadUInt32();
	m.materialIndex = reader.ReadUInt32();
	return m;
}

static MeshGroupInfo DeserializeMeshGroup(BinaryReader& reader)
{
	MeshGroupInfo m{};
	m.subMeshStart = reader.ReadUInt32();
	m.subMeshCount = reader.ReadUInt32();
	return m;
}

static NodeInfo DeserializeNode(BinaryReader& reader)
{
	NodeInfo m{};

	m.meshGroupIndex = reader.ReadInt32();

	m.parentIndex = reader.ReadInt32();

	m.localMatrix._11 = reader.ReadFloat();
	m.localMatrix._12 = reader.ReadFloat();
	m.localMatrix._13 = reader.ReadFloat();
	m.localMatrix._14 = reader.ReadFloat();

	m.localMatrix._21 = reader.ReadFloat();
	m.localMatrix._22 = reader.ReadFloat();
	m.localMatrix._23 = reader.ReadFloat();
	m.localMatrix._24 = reader.ReadFloat();

	m.localMatrix._31 = reader.ReadFloat();
	m.localMatrix._32 = reader.ReadFloat();
	m.localMatrix._33 = reader.ReadFloat();
	m.localMatrix._34 = reader.ReadFloat();

	m.localMatrix._41 = reader.ReadFloat();
	m.localMatrix._42 = reader.ReadFloat();
	m.localMatrix._43 = reader.ReadFloat();
	m.localMatrix._44 = reader.ReadFloat();

	return m;
}

// Imdlのロード関数
static HRESULT LoadImdl
(
	const std::wstring& filename,
	std::vector<TextureEntry>& textures,
	std::vector<MaterialInfo>& materials,
	std::vector<SubMeshInfo>& subMeshes,
	std::vector<MeshGroupInfo>& meshGroups,
	std::vector<NodeInfo>& nodes,
	std::vector<VertexPositionNormalTextureTangent>& vertices,
	std::vector<uint32_t>& indices
)
{
	// ファイルオープン
	std::ifstream ifs(filename, std::ios::binary);
	if (!ifs.is_open())
	{
		return E_FAIL;
	}

	// ヘッダ
	FileHeader header{};
	ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

	if (header.magic != 0x4C444D49)	// 'IMDL'
	{
		return E_FAIL;
	}

	// チャンク読み込み
	for (uint32_t i = 0; i < header.chunkCount; ++i)
	{
		Imase::ChunkHeader ch{};
		std::vector<uint8_t> buffer;

		// チェンクデータ読み込み
		if (!Imase::ReadChunk(ifs, ch, buffer))
			return E_FAIL;

		// 指定サイズのデータを取得するリーダー
		BinaryReader reader(buffer);

		switch (ch.type)
		{

		case CHUNK_TEXTURE:		// TextureType
		{
			// テクスチャの数
			uint32_t count = reader.ReadUInt32();
			textures.resize(count);

			for (uint32_t t = 0; t < count; t++)
			{
				// テクスチャ
				textures[t].type = static_cast<TextureType>(reader.ReadUInt32());
				uint32_t size = reader.ReadUInt32();
				textures[t].data.resize(size);
				reader.ReadBytes(textures[t].data.data(), size);
			}
			break;
		}

		case CHUNK_MATERIAL:	// MaterialInfo
		{
			uint32_t count = reader.ReadUInt32();
			materials.reserve(count);
			for (uint32_t m = 0; m < count; m++)
			{
				materials.push_back(DeserializeMaterial(reader));
			}
			break;
		}

		case CHUNK_SUBMESH:		// SubMeshInfo
		{
			uint32_t count = reader.ReadUInt32();
			subMeshes.reserve(count);
			for (uint32_t m = 0; m < count; m++)
			{
				subMeshes.push_back(DeserializeSubMesh(reader));
			}
			break;
		}

		case CHUNK_MESHGROUP:	// MeshGroupInfo
		{
			uint32_t count = reader.ReadUInt32();
			meshGroups.reserve(count);
			for (uint32_t m = 0; m < count; m++)
			{
				meshGroups.push_back(DeserializeMeshGroup(reader));
			}
			break;
		}

		case CHUNK_NODE:	// NodeInfo
		{
			uint32_t count = reader.ReadUInt32();
			nodes.reserve(count);
			for (uint32_t m = 0; m < count; m++)
			{
				nodes.push_back(DeserializeNode(reader));
			}
			break;
		}

		case CHUNK_VERTEX:		// VertexPositionNormalTextureTangent
		{
			uint32_t count = reader.ReadUInt32();
			vertices.reserve(count);
			for (uint32_t v = 0; v < count; v++)
			{
				vertices.push_back(DeserializeVertex(reader));
			}
			break;
		}

		case CHUNK_INDEX:		// uint32_t
		{
			indices = reader.ReadVector<uint32_t>();
			break;
		}

		default:
			throw std::runtime_error("Unknown chunk type");
		}
	}

	return S_OK;
}

// --------------------------------------------------------------------------------------------- //

// コンストラクタ
Imase::Model::Model(ID3D11Device* device, Imase::Effect* pEffect)
	: m_pEffect(pEffect)
{
	// ----- ラスタライザーステート ----- //
	{
		// ラスタライザーステートの作成
		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = TRUE;	// 反時計回りが表（CCW）
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthClipEnable = TRUE;
		desc.ScissorEnable = FALSE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;
		DX::ThrowIfFailed(
			device->CreateRasterizerState(&desc, m_rasterizerState.ReleaseAndGetAddressOf())
		);
	}

	// ----- 深度ステンシルステート ----- //
	{
		// 深度ステンシルステートの作成
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = FALSE;
		DX::ThrowIfFailed(
			device->CreateDepthStencilState(&desc, m_depthStencilState.ReleaseAndGetAddressOf())
		);
	}

	// ----- ブレンドステート ----- //
	{
		// ブレンドステートの作成
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;

		// ストレートアルファの設定
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		DX::ThrowIfFailed(
			device->CreateBlendState(&desc, m_blendState.ReleaseAndGetAddressOf())
		);
	}
}

// モデルデータ作成関数
std::unique_ptr<Imase::Model> Imase::Model::CreateFromImdl(ID3D11Device* device, std::wstring fname, Imase::Effect* pEffect)
{
	std::vector<TextureEntry> textures;
	std::vector<MaterialInfo> materials;
	std::vector<SubMeshInfo> subMeshes;
	std::vector<MeshGroupInfo> meshGroups;
	std::vector<NodeInfo> nodes;
	std::vector<VertexPositionNormalTextureTangent> vertices;
	std::vector<uint32_t> indices;

	// IMDLファイルのロード
	HRESULT hr = LoadImdl(fname, textures, materials, subMeshes, meshGroups, nodes, vertices, indices);
	if (hr == E_FAIL)
	{
		OutputDebugString(L"Failed to load IMDL file.\n");
	}

	auto model = std::make_unique<Model>(device, pEffect);

	// エフェクトにテクスチャのシェダーリソースを作成して登録
	model->GetEffect()->RegisterTextures(device, textures);

	// エフェクトにマテリアルを登録
	model->GetEffect()->RegisterMaterials(materials);

	// サブメッシュを登録
	for (auto& mesh : subMeshes)
	{
		model->m_subMeshes.emplace_back(mesh);
	}

	// メッシュグループを登録
	for (auto& group : meshGroups)
	{
		model->m_meshGroups.emplace_back(group);
	}

	// ノードを登録
	for (auto& node : nodes)
	{
		model->m_nodes.emplace_back(node);
	}

	// 頂点バッファの作成
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionNormalTextureTangent) * vertices.size());
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = vertices.data();

		DX::ThrowIfFailed(
			device->CreateBuffer(&desc, &data, model->m_vertexBuffer.ReleaseAndGetAddressOf())
		);
	}

	// インデックスバッファ作成
	{
		// インデックス頂点バッファの作成
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = indices.data();

		DX::ThrowIfFailed(
			device->CreateBuffer(&desc, &data, model->m_indexBuffer.ReleaseAndGetAddressOf())
		);
	}

	return model;
}

// 描画関数
void Imase::Model::Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX world)
{
	// ラスタライザーステートの設定
	context->RSSetState(m_rasterizerState.Get());

	// 深度ステンシルバッファの設定
	context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	// ブレンドステートの設定
	context->OMSetBlendState(m_blendState.Get(), nullptr, 0xffffffff);

	// 頂点バッファの設定
	ID3D11Buffer* buffers[] = { m_vertexBuffer.Get() };
	UINT stride = sizeof(VertexPositionNormalTextureTangent);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, buffers, &stride, &offset);

	// インデックスバッファの設定
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// トポロジーの設定
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ----- メッシュ描画 ----- //

	// 各ノードのワールド行列を作成
	std::vector<XMMATRIX> worldMatrices(m_nodes.size());

	for (size_t i = 0; i < m_nodes.size(); i++)
	{
		XMMATRIX local = XMLoadFloat4x4(&m_nodes[i].localMatrix);

		if (m_nodes[i].parentIndex >= 0)
		{
			worldMatrices[i] = local * worldMatrices[m_nodes[i].parentIndex];
		}
		else
		{
			// ルートノード
			worldMatrices[i] = local;
		}
	}

	// ノード毎にメッシュを描画
	for (size_t n = 0; n < m_nodes.size(); n++)
	{
		auto& node = m_nodes[n];

		XMMATRIX nodeWorld = worldMatrices[n] * world;

		uint32_t start = m_meshGroups[node.meshGroupIndex].subMeshStart;
		uint32_t count = m_meshGroups[node.meshGroupIndex].subMeshCount;

		for (uint32_t i = 0; i < count; i++)
		{
			SubMeshInfo& mesh = m_subMeshes[start + i];

			m_pEffect->SetMaterialIndex(mesh.materialIndex);
			m_pEffect->SetWorld(nodeWorld);
			m_pEffect->Apply(context);

			context->DrawIndexed(mesh.indexCount, mesh.startIndex, 0);
		}
	}
}
