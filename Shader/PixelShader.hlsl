#include "Header.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VSOutput vout) : SV_Target
{
    float4 color = vout.Diffuse;

    if (UseTexture != 0)
    {
        color *= Texture.Sample(Sampler, vout.TexCoord);
    }

    return color;
}
