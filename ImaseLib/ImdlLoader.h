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

		// ロードする関数（マテリアル）
		static Imase::MaterialInfo DeserializeMaterial(BinaryReader& reader);

		// ロードする関数（頂点情報）
		static Imase::VertexPositionNormalTextureTangent DeserializeVertex(BinaryReader& reader);

		// ロードする関数（メッシュ）
		static Imase::SubMeshInfo DeserializeSubMesh(BinaryReader& reader);

		// ロードする関数（メッシュグループ）
		static Imase::MeshGroupInfo DeserializeMeshGroup(BinaryReader& reader);

		// ロードする関数（ノード）
		static Imase::NodeInfo DeserializeNode(BinaryReader& reader);

		// ロードする関数（Vector3の配列）
		static Imase::AnimationChannelVec3 DeserializeChannelVec3(BinaryReader& reader);

		// ロードする関数（クォータニオンの配列）
		static Imase::AnimationChannelQuat DeserializeChannelQuat(BinaryReader& reader);

		// ロードする関数（アニメーション）
		static Imase::AnimationClip DeserializeAnimationClip(BinaryReader& reader);

		// ロードする関数（スキン）
		static Imase::SkinInfo DeserializeSkinInfo(BinaryReader& reader);

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
			std::vector<SkinInfo>& skins,
			std::vector<VertexPositionNormalTextureTangent>& vertices,
			std::vector<uint32_t>& indices
		);

	};
}