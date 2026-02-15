#include "Common.hlsli"
#include "Basic.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    // 色
    float4 color = pin.Color;

    // テクスチャ（Flags 1bit UseBaseColorTexture）
    if (Flags & 0x1) color *= Texture.Sample(Sampler, pin.TexCoord);

    return color;
}

