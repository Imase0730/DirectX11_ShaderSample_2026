// 定数バッファ
cbuffer Parameters : register(b0)
{
    float4x4 WorldViewProj  : packoffset(c0);        // ワールド行列 × ビュー行列 × プロジェクション行列
    float4x4 WorldInverseTranspose : packoffset(c4); // ワールド行列の逆転置行列
    float3 LightDirection   : packoffset(c8);        // ライトの方向
};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position : POSITION;     // 位置
    float3 Normal   : NORMAL;       // 法線
    float4 Color    : COLOR;        // 色
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Diffuse  : COLOR;        // ディフューズ色
    float4 Position : SV_POSITION;  // 位置
};
