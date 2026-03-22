#include "Common.hlsli"
#include "Lighting.hlsli"
#include "PixelLighting.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    // 法線
    float3 N = normalize(pin.NormalWS);
    
    // 視線ベクトル（ワールド空間）
    float3 V = normalize(EyePosition.xyz - pin.WorldPos);

    // ライトの計算
    ColorPair result = ComputeLights(V, N);

    float4 color = result.Diffuse;

    // テクスチャあり
    if (Flags & 0x1)
    {
        color *= Texture.Sample(Sampler, pin.TexCoord);
    }

    color += result.Specular;
  
    return color;
}