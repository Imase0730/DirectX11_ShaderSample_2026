
// 頂点シェーダーの出力用
struct LightingVaryings
{
    float4 Color    : COLOR;        // 色
    float2 TexCoord : TEXCOORD0;    // テクスチャ座標
    float4 Position : SV_POSITION;  // 位置
};
