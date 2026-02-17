
// 頂点シェーダーの出力用
struct VSOutput
{
    float2 TexCoord     : TEXCOORD0;    // テクスチャ座標
    float3 WorldPos     : TEXCOORD1;    // ワールド空間の位置
    float3 NormalWS     : TEXCOORD2;    // ワールド空間の法線
    float4 TangentWS    : TEXCOORD3;    // ワールド空間の接線
    float4 Position     : SV_POSITION;  // 位置
};

