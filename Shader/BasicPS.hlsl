#include "Common.hlsli"
#include "BasicHeader.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(LightingVaryings pin) : SV_Target
{
    // 色
    float4 color = pin.Color;

    // テクスチャ（MaterialParams.y = UseTexture）
    if (MaterialParams.y) color *= Texture.Sample(Sampler, pin.TexCoord);

    return color;
}

