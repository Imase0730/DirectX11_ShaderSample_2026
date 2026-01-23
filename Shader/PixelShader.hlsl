#include "Header.hlsli"

Texture2D<float4> Texture : register(t0);
sampler Sampler : register(s0);

float4 main(VSOutput vout) : SV_TARGET
{
    return Texture.Sample(Sampler, vout.TexCoord);
}
