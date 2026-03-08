#include "Common.hlsli"
#include "PixelLighting.hlsli"

VSOutput main(VSInput vin)
{
    VSOutput vout;
    
    float4 pos = float4(vin.Position, 1.0f);

    // スキン有り
    if (UseSkin)
    {
        pos = mul(pos, SkinMatrices[vin.Joint.x]) * vin.Weight.x
            + mul(pos, SkinMatrices[vin.Joint.y]) * vin.Weight.y
            + mul(pos, SkinMatrices[vin.Joint.z]) * vin.Weight.z
            + mul(pos, SkinMatrices[vin.Joint.w]) * vin.Weight.w;
    }
    
    // 座標変換
    float4 worldPos = mul(World, pos);
    float4 viewPos = mul(View, worldPos);
    vout.Position = mul(Projection, viewPos);

    // ワールド空間の位置、法線を計算
    vout.WorldPos = worldPos.xyz;
    vout.NormalWS = normalize(mul((float3x3)WorldInverseTranspose, vin.Normal));
    
    // テクスチャ座標
    vout.TexCoord = float2(vin.TexCoord.x, vin.TexCoord.y);

    return vout;
}

