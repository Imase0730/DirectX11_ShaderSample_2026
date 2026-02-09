#include "Common.hlsli"
#include "Header.hlsli"

Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    // 色
    float4 color = pin.Color;

    // テクスチャ
    if (UseTexture != 0)
        color *= Texture.Sample(Sampler, pin.TexCoord);

    return color;
}

//float4 main(VSOutput pin) : SV_Target
//{
//    // 法線マップサンプル
//    float2 xy = NormalMap.Sample(Sampler, pin.TexCoord).rg * 2.0f - 1.0f;
    
//    // Z再構築
//    float z = sqrt(saturate(1.0f - dot(xy, xy)));

//    // 法線
//    float3 N = normalize(float3(xy, z));
//    // ライト
//    float3 L = normalize(pin.LightTangentDirect);
//    // 視線
//    float3 V = normalize(pin.ViewTangent);

//    // ディフューズ
//    float diffuse = saturate(dot(L, N));

//    // スペキュラー
//    float3 H = normalize(L + V);
//    float spec = pow(saturate(dot(N, H)), SpecularPower);
    
//    // 色
//    float4 color = float4(DiffuseColor * diffuse + SpecularColor * spec, 1.0f);

//    // テクスチャ
//    if (UseTexture != 0)
//        color *= Texture.Sample(Sampler, pin.TexCoord);

//    return color;
//}

