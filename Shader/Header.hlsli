
// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position     : POSITION;     // 位置
    float3 Normal       : NORMAL;       // 法線
    float2 TexCoord     : TEXCOORD;     // テクスチャ座標
    float4 Tangent      : TANGENT;      // 接線
};

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Color    : COLOR;        // 色
    float2 TexCoord : TEXCOORD0;    // テクスチャ座標
    float4 Position : SV_POSITION;  // 位置
};

//// 頂点シェーダーの出力用
//struct VSOutput
//{
//    float4 Diffuse : COLOR; // ディフューズ色
//    float2 TexCoord : TEXCOORD0; // テクスチャ座標
//    float3 WorldPos : TEXCOORD1; // ワールド空間の位置
//    float3 Normal : TEXCOORD2; // 法線
//    float4 Tangent : TANGENT; // 接線
//    float4 Position : SV_POSITION; // 位置
//    float3 LightTangentDirect : TEXCOORD3;
//    float3 ViewTangent : TEXCOORD4;
//};
