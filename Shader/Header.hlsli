// 定数バッファ
cbuffer Parameters : register(b0)
{
    float4x4 WorldViewProj;         // ワールド行列 × ビュー行列 × プロジェクション行列
};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position : POSITION;     // 位置
    float4 Color    : COLOR;        // 色
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Color    : COLOR;        // 色
    float4 Position : SV_POSITION;  // 位置
};
