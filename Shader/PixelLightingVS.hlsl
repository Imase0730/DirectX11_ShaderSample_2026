#include "Common.hlsli"
#include "PixelLighting.hlsli"

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換
    float4 worldPos = mul(World, float4(vin.Position, 1.0f));
    float4 viewPos = mul(View, worldPos);
    vout.Position = mul(Projection, viewPos);

    // ワールド空間の位置、法線を計算
    vout.WorldPos = worldPos.xyz;
    vout.NormalWS = normalize(mul((float3x3)WorldInverseTranspose, vin.Normal));
    
    // テクスチャ座標
    vout.TexCoord = float2(vin.TexCoord.x, vin.TexCoord.y);

    return vout;
}

