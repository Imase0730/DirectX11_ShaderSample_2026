//--------------------------------------------------------------------------------------
// File: ImdlLoader.h
//
// Imdl形式のモデルデータをロードするクラス
//
// Date: 2025.3.1
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "Imdl.h"
#include "BinaryReader.h"

namespace Imase
{
	class ImdlLoader
	{
	private:

		static Imase::MaterialInfo DeserializeMaterial(BinaryReader& reader);

		static Imase::VertexPositionNormalTextureTangent DeserializeVertex(BinaryReader& reader);

		static Imase::SubMeshInfo DeserializeSubMesh(BinaryReader& reader);

		static Imase::MeshGroupInfo DeserializeMeshGroup(BinaryReader& reader);

		static Imase::NodeInfo DeserializeNode(BinaryReader& reader);

		static Imase::AnimationChannelVec3 DeserializeChannelVec3(BinaryReader& reader);

		static Imase::AnimationChannelQuat DeserializeChannelQuat(BinaryReader& reader);

		static Imase::AnimationClip DeserializeAnimationClip(BinaryReader& reader);

	public:

		// Imdlのロード関数
		static HRESULT LoadImdl
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
		);

	};
}