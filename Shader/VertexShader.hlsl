#include "Header.hlsli"

float3 ComputeLight(float3 worldNormal)
{
    float3 dotL = mul(-LightDirection, worldNormal);
    float3 zeroL = step(0, dotL);
    return zeroL * dotL;
}

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 法線をワールド座標系へ変換する
    float3 worldNormal = normalize(mul(vin.Normal, (float3x3)WorldInverseTranspose));
    
    vout.Position = mul(float4(vin.Position, 1.0f), WorldViewProj);
    vout.Diffuse = float4(ComputeLight(worldNormal), 1.0f) * vin.Color;
    
	return vout;
}
