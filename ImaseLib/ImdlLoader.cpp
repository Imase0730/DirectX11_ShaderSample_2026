//--------------------------------------------------------------------------------------
// File: ImdlLoader.cpp
//
// Imdl形式のモデルデータをロードするクラス
//
// Date: 2025.3.1
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "ImdlLoader.h"
#include "ChunkIO.h"

Imase::MaterialInfo Imase::ImdlLoader::DeserializeMaterial(BinaryReader& reader)
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

Imase::VertexPositionNormalTextureTangent Imase::ImdlLoader::DeserializeVertex(BinaryReader& reader)
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

Imase::SubMeshInfo Imase::ImdlLoader::DeserializeSubMesh(BinaryReader& reader)
{
	SubMeshInfo m{};
	m.startIndex = reader.ReadUInt32();
	m.indexCount = reader.ReadUInt32();
	m.materialIndex = reader.ReadUInt32();
	return m;
}

Imase::MeshGroupInfo Imase::ImdlLoader::DeserializeMeshGroup(BinaryReader& reader)
{
	MeshGroupInfo m{};
	m.subMeshStart = reader.ReadUInt32();
	m.subMeshCount = reader.ReadUInt32();
	return m;
}

Imase::NodeInfo Imase::ImdlLoader::DeserializeNode(BinaryReader& reader)
{
	NodeInfo m{};

	m.meshGroupIndex = reader.ReadInt32();

	m.parentIndex = reader.ReadInt32();

	m.defaultTranslation.x = reader.ReadFloat();
	m.defaultTranslation.y = reader.ReadFloat();
	m.defaultTranslation.z = reader.ReadFloat();

	m.defaultRotation.x = reader.ReadFloat();
	m.defaultRotation.y = reader.ReadFloat();
	m.defaultRotation.z = reader.ReadFloat();
	m.defaultRotation.w = reader.ReadFloat();

	m.defaultScale.x = reader.ReadFloat();
	m.defaultScale.y = reader.ReadFloat();
	m.defaultScale.z = reader.ReadFloat();

	return m;
}

Imase::AnimationChannelVec3 Imase::ImdlLoader::DeserializeChannelVec3(BinaryReader& reader)
{
	AnimationChannelVec3 m = {};

	m.nodeIndex = reader.ReadUInt32();

	uint32_t time_size = reader.ReadUInt32();
	m.times.resize(time_size);
	for (uint32_t j = 0; j < time_size; j++)
	{
		m.times[j] = reader.ReadFloat();
	}

	m.values = reader.ReadVector<DirectX::XMFLOAT3>();

	return m;
}

Imase::AnimationChannelQuat Imase::ImdlLoader::DeserializeChannelQuat(BinaryReader& reader)
{
	AnimationChannelQuat m = {};

	m.nodeIndex = reader.ReadUInt32();

	uint32_t time_size = reader.ReadUInt32();
	m.times.resize(time_size);
	for (uint32_t j = 0; j < time_size; j++)
	{
		m.times[j] = reader.ReadFloat();
	}

	m.values = reader.ReadVector<DirectX::XMFLOAT4>();

	return m;
}

Imase::AnimationClip Imase::ImdlLoader::DeserializeAnimationClip(BinaryReader& reader)
{
	AnimationClip m = {};

	m.name = reader.ReadString();
	m.duration = reader.ReadFloat();

	uint32_t count = reader.ReadUInt32();
	m.translations.resize(count);
	for (uint32_t i = 0; i < count; i++)
	{
		m.translations[i] = DeserializeChannelVec3(reader);
	}

	count = reader.ReadUInt32();
	m.rotations.resize(count);
	for (uint32_t i = 0; i < count; i++)
	{
		m.rotations[i] = DeserializeChannelQuat(reader);
	}

	count = reader.ReadUInt32();
	m.scales.resize(count);
	for (uint32_t i = 0; i < count; i++)
	{
		m.scales[i] = DeserializeChannelVec3(reader);
	}

	return m;
}

// Imdlのロード関数
HRESULT Imase::ImdlLoader::LoadImdl
(
	const std::wstring& filename,
	std::vector<TextureEntry>& textures,
	std::vector<MaterialInfo>& materials,
	std::vector<SubMeshInfo>& subMeshes,
	std::vector<MeshGroupInfo>& meshGroups,
	std::vector<NodeInfo>& nodes,
	std::vector<AnimationClip>& animationClips,
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

			for (uint32_t j = 0; j < count; j++)
			{
				// テクスチャ
				textures[j].type = static_cast<TextureType>(reader.ReadUInt32());
				uint32_t size = reader.ReadUInt32();
				textures[j].data.resize(size);
				reader.ReadBytes(textures[j].data.data(), size);
			}
			break;
		}

		case CHUNK_MATERIAL:	// MaterialInfo
		{
			uint32_t count = reader.ReadUInt32();
			materials.reserve(count);
			for (uint32_t j = 0; j < count; j++)
			{
				materials.push_back(DeserializeMaterial(reader));
			}
			break;
		}

		case CHUNK_SUBMESH:		// SubMeshInfo
		{
			uint32_t count = reader.ReadUInt32();
			subMeshes.reserve(count);
			for (uint32_t j = 0; j < count; j++)
			{
				subMeshes.push_back(DeserializeSubMesh(reader));
			}
			break;
		}

		case CHUNK_MESHGROUP:	// MeshGroupInfo
		{
			uint32_t count = reader.ReadUInt32();
			meshGroups.reserve(count);
			for (uint32_t j = 0; j < count; j++)
			{
				meshGroups.push_back(DeserializeMeshGroup(reader));
			}
			break;
		}

		case CHUNK_NODE:	// NodeInfo
		{
			uint32_t count = reader.ReadUInt32();
			nodes.reserve(count);
			for (uint32_t j = 0; j < count; j++)
			{
				nodes.push_back(DeserializeNode(reader));
			}
			break;
		}

		case CHUNK_VERTEX:		// VertexPositionNormalTextureTangent
		{
			uint32_t count = reader.ReadUInt32();
			vertices.reserve(count);
			for (uint32_t j = 0; j < count; j++)
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

		case CHUNK_ANIMATION:	// AnimationClip
		{
			uint32_t count = reader.ReadUInt32();
			animationClips.reserve(count);
			for (uint32_t j = 0; j < count; j++)
			{
				animationClips.push_back(DeserializeAnimationClip(reader));
			}
			break;
		}

		default:
			throw std::runtime_error("Unknown chunk type");
		}

	}

	return S_OK;
}
