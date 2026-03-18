#include "Common.hlsli"
#include "Lighting.hlsli"
#include "PixelLighting.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    float4 color = BaseColor;

    // テクスチャ色
    if (Flags & 0x1)
        color *= Texture.Sample(Sampler, pin.TexCoord);

    // 法線
    float3 N = normalize(pin.NormalWS);
    
    // 視線ベクトル（ワールド空間）
    float3 V = normalize(EyePosition.xyz - pin.WorldPos);

    // ライトの計算
    ColorPair result = ComputeLights(V, N);

    color.rgb *= result.Diffuse.rgb;
    color += result.Specular;
  
    return color;
}