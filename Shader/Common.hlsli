
// 定数バッファ：フレームで１回更新
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

// 定数バッファ：オブジェクト毎更新
cbuffer PerObjectCB : register(b1)
{
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

// 定数バッファ：マテリアル
cbuffer PerMaterialCB : register(b2)
{
    float4 BaseColor;   // RGB + Alpha

    float3 EmissiveColor;
    float Metallic;

    float Roughness;
    uint Flags;         // 1bit:UseBaseColorTexture 2bit:UseNormalTexture
    float2 _paddding_M0;
};

// 頂点シェーダーの入力用
struct VSInput
{
    float3 Position : POSITION; // 位置
    float3 Normal   : NORMAL;   // 法線
    float2 TexCoord : TEXCOORD; // テクスチャ座標
    float4 Tangent  : TANGENT;  // 接線
};

