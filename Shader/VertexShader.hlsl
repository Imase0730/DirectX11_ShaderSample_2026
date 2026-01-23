#include "Header.hlsli"

VSOutput main(VSInput vin)
{
    VSOutput vout;

    vout.Position = mul(float4(vin.Position, 1.0f), WorldViewProj);
    vout.Color = vin.Color;
    
	return vout;
}