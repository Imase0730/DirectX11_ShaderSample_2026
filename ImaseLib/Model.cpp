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
#include "ImdlLoader.h"

using namespace DirectX;
using namespace Imase;

#include "DDSTextureLoader.h"

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
	std::vector<VertexPositionNormalTextureTangent> vertices;
	std::vector<uint32_t> indices;

	auto model = std::make_unique<Model>(device, pEffect);

	// IMDLファイルのロード
	HRESULT hr = ImdlLoader::LoadImdl(
		fname,
		textures, materials,
		model->m_subMeshes, model->m_meshGroups, model->m_nodes, model->m_animations,
		vertices, indices
	);
	if (hr == E_FAIL)
	{
		OutputDebugString(L"Failed to load IMDL file.\n");
	}

	// エフェクトにテクスチャのシェダーリソースを作成して登録
	model->GetEffect()->RegisterTextures(device, textures);

	// エフェクトにマテリアルを登録
	model->GetEffect()->RegisterMaterials(materials);

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

void Imase::Model::Draw(ID3D11DeviceContext* context, const DirectX::XMMATRIX& world)
{
	DrawInternal(context, world, nullptr);
}

void Imase::Model::Draw(ID3D11DeviceContext* context, const DirectX::XMMATRIX& world, const std::vector<DirectX::XMFLOAT4X4>* animatedWorldMatrices)
{
	DrawInternal(context, world, animatedWorldMatrices);
}

// 描画関数
void Imase::Model::DrawInternal(
	ID3D11DeviceContext* context,
	const DirectX::XMMATRIX& world,
	const std::vector<DirectX::XMFLOAT4X4>* animatedWorldMatrices
)
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

	// ---- ノード行列準備 ---- //

	std::vector<XMMATRIX> worldMatrices(m_nodes.size());

	if (animatedWorldMatrices)
	{
		// ★ アニメーションあり
		for (size_t i = 0; i < m_nodes.size(); ++i)
		{
			worldMatrices[i] =
				XMLoadFloat4x4(&(*animatedWorldMatrices)[i]) * world;
		}
	}
	else
	{
		// ★ 静的ノード計算
		for (size_t i = 0; i < m_nodes.size(); ++i)
		{
			const NodeInfo& node = m_nodes[i];

			XMVECTOR t = XMLoadFloat3(&node.defaultTranslation);
			XMVECTOR r = XMLoadFloat4(&node.defaultRotation);
			XMVECTOR s = XMLoadFloat3(&node.defaultScale);

			XMMATRIX local = XMMatrixScalingFromVector(s) *	XMMatrixRotationQuaternion(r) *	XMMatrixTranslationFromVector(t);

			if (m_nodes[i].parentIndex >= 0)
			{
				worldMatrices[i] =
					local * worldMatrices[m_nodes[i].parentIndex];
			}
			else
			{
				worldMatrices[i] = local;
			}

			worldMatrices[i] *= world;
		}
	}

	// ---- 描画 ---- //

	for (size_t n = 0; n < m_nodes.size(); ++n)
	{
		auto& node = m_nodes[n];
		XMMATRIX nodeWorld = worldMatrices[n];

		uint32_t start = m_meshGroups[node.meshGroupIndex].subMeshStart;
		uint32_t count = m_meshGroups[node.meshGroupIndex].subMeshCount;

		for (uint32_t i = 0; i < count; ++i)
		{
			SubMeshInfo& mesh = m_subMeshes[start + i];

			m_pEffect->SetMaterialIndex(mesh.materialIndex);
			m_pEffect->SetWorld(nodeWorld);
			m_pEffect->Apply(context);

			context->DrawIndexed(mesh.indexCount, mesh.startIndex, 0);
		}
	}
}

// ノードを取得する関数
const std::vector<Imase::NodeInfo>& Imase::Model::GetNodes() const
{
	return m_nodes;
}

// アニメーションを取得する関数
const Imase::AnimationClip* Imase::Model::GetAnimation(uint32_t index) const
{
	if (index >= m_animations.size())
	{
		return nullptr;
	}
	return &m_animations[index];
}
