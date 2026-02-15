
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

static const float PI = 3.14159265f;

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}