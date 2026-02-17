//--------------------------------------------------------------------------------------
// File: Imdl.h
//
// モデルデータ共通ヘッダ（変換コンバーターと読み込み側共通）
//
// Date: 2026.2.16
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include <vector>
#include <DirectXMath.h>

namespace Imase
{
    // マテリアル情報
    struct MaterialInfo
    {
        // -------------------
        // 基本色 / PBRパラメータ
        // -------------------
        DirectX::XMFLOAT4 diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };    // BaseColor (白)
        float metallicFactor = 0.0f;                                    // 金属度 (0:非金属, 1:金属)
        float roughnessFactor = 1.0f;                                   // 粗さ (0:ツルツル, 1:粗い)
        DirectX::XMFLOAT3 emissiveColor = { 0.0f, 0.0f, 0.0f };         // 放射色

        // -------------------
        // テクスチャインデックス (-1 = 無効)
        // -------------------
        int baseColorTexIndex = -1;   // BaseColor テクスチャ
        int normalTexIndex = -1;      // NormalMap テクスチャ
        int metalRoughTexIndex = -1;  // Metallic-Roughness テクスチャ
        int emissiveTexIndex = -1;    // Emissive テクスチャ
    };

    // メッシュ情報
    struct MeshInfo
    {
        uint32_t startIndex;        // スタートインデックス  
        uint32_t primCount;         // プリミティブ数
        uint32_t materialIndex;     // マテリアルインデックス
    };

    // 頂点情報
    struct VertexPositionNormalTextureTangent
    {
        DirectX::XMFLOAT3 position;    // 位置
        DirectX::XMFLOAT3 normal;      // 法線
        DirectX::XMFLOAT2 texcoord;    // テクスチャ座標
        DirectX::XMFLOAT4 tangent;     // xyz = 接線, w = 従接線の向きを調整（1,-1)
    };

    // -------------------------------------------------------------------------------------- //
    // ヘッダ
    struct FileHeader
    {
        uint32_t magic;      // 'IMDL'
        uint32_t version;
        uint32_t chunkCount;
    };

    // チャンクタイプ
    enum ChunkType : uint32_t
    {
        CHUNK_TEXTURE = 'TXTR',
        CHUNK_MATERIAL = 'MTRL',
        CHUNK_MESH = 'MESH',
        CHUNK_VERTEX = 'VERT',
        CHUNK_INDEX = 'INDX'
    };

    // テクスチャタイプ
    enum class TextureType
    {
        BaseColor,
        Normal,
        MetalRough,
        Emissive
    };

    // テクスチャデータ
    struct TextureEntry
    {
        TextureType type;           // 種類
        std::vector<uint8_t> data;  // データ
    };
}
