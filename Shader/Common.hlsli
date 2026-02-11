
// 定数バッファ：フレーム毎の更新
cbuffer PerFrameCB : register(b0)
{
    float4x4 View;
    float4x4 Projection;

    float4 AmbientLightColor;

    float4 LightDirection[3];
    float4 LightDiffuseColor[3];
    float4 LightSpecularColor[3];

    float4 EyePosition;
};

// 定数バッファ：オブジェクト毎の更新
cbuffer PerObjectCB : register(b1)
{
    float4 DiffuseColor;
    float4 EmissiveColor;
    float4 SpecularColor;
    float4 MaterialParams; // x = SpecularPower, y = UseTexture, z = UseNormalMap

    float4x4 World;
    float4x4 WorldInverseTranspose;
};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position : POSITION; // 位置
    float3 Normal   : NORMAL;   // 法線
    float2 TexCoord : TEXCOORD; // テクスチャ座標
    float4 Tangent  : TANGENT;  // 接線
};
