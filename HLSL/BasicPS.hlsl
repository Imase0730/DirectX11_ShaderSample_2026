#include "Common.hlsli"
#include "Basic.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    float4 color = pin.Diffuse;

    // テクスチャあり
    if (Flags & 0x1)
    {
        color *= Texture.Sample(Sampler, pin.TexCoord);
    }

    color += pin.Specular;
    
    return color;
}

