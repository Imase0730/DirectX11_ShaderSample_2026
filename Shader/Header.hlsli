// 定数バッファ
cbuffer Parameters : register(b0)
{
    float3 DiffuseColor     : packoffset(c0);       // ディフューズ色
    float3 EmissiveColor    : packoffset(c1);       // エミッシブ色
    float3 SpecularColor    : packoffset(c2);       // スペキュラー色
    float SpecularPower     : packoffset(c2.w);     // スペキュラーパワー

    float4x4 World          : packoffset(c3);       // ワールド行列
    float4x4 WorldViewProj  : packoffset(c7);       // ワールド行列 × ビュー行列 × プロジェクション行列
    float4x4 WorldInverseTranspose : packoffset(c11);    // ワールド行列の逆転置行列

    float3 LightDirection   : packoffset(c15);      // ライトの方向
    float3 EyePosition      : packoffset(c16);      // カメラの位置
    uint   UseTexture       : packoffset(c16.w);    // 0 = 使わない, 1 = 使う

};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position     : POSITION;     // 位置
    float3 Normal       : NORMAL;       // 法線
    float2 TextCoord    : TEXCOORD;     // テクスチャ座標
    float4 Tangent      : TANGENT;      // 接線
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Diffuse  : COLOR;           // ディフューズ色
    float2 TexCoord : TEXCOORD;        // テクスチャ座標
    float4 Tangent  : TANGENT;         // 接線
    float4 Position : SV_POSITION;     // 位置
};
