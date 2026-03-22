#include "Common.hlsli"
#include "Lighting.hlsli"
#include "NormalMap.hlsli"

Texture2D BaseColorTex  : register(t0);
Texture2D NormalTex     : register(t1);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    // 法線
    float3 N = normalize(pin.NormalWS);

    // 法線マップあり
    if (Flags & 0x2)
    {
        // 法線マップサンプル　(法線マップが2チャンネル：BC5圧縮)
        float2 xy = NormalTex.Sample(Sampler, pin.TexCoord).rg * 2.0f - 1.0f;
    
        // Z再構築
        float z = sqrt(saturate(1.0f - dot(xy, xy)));

        // 法線
        N = normalize(float3(xy, z));
    
        float3 normal = normalize(pin.NormalWS);
        float3 tangent = normalize(pin.TangentWS.xyz);

        // 再直交化
        tangent = normalize(tangent - normal * dot(normal, tangent));
        float3 binormal = cross(normal, tangent) * pin.TangentWS.w;

        // 接空間基底を並べて行列を作成（各ベクトルを「行」に配置）
        float3x3 TBN = float3x3(tangent, binormal, normal);

        // mul(ベクトル, 行列) とすることで、接空間(N)をワールド空間へ変換
        N = normalize(mul(N, TBN));
    }
   
    // 視線ベクトル（ワールド空間）
    float3 V = normalize(EyePosition.xyz - pin.WorldPos);

    // ライトの計算
    ColorPair result = ComputeLights(V, N);
   
    float4 color = result.Diffuse;

    // テクスチャあり
    if (Flags & 0x1)
    {
        color *= BaseColorTex.Sample(Sampler, pin.TexCoord);
    }

    color += result.Specular;
  
    return color;
}
