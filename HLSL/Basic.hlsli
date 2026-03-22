#ifndef BASIC
#define BASIC

// 頂点シェーダーの出力用
struct VSOutput
{
    float4 Diffuse  : COLOR0;       // ディフューズ色
    float4 Specular : COLOR1;       // スペキュラ色
    float2 TexCoord : TEXCOORD0;    // テクスチャ座標
    float4 Position : SV_POSITION;  // 位置
};

#endif  // BASIC
