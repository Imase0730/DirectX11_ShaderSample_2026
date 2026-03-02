#include "Common.hlsli"
#include "PixelLighting.hlsli"

VSOutput main(VSInput vin)
{
    VSOutput vout;
    
    float4 pos = float4(vin.Position, 1.0f);

    // スキン有り
    if (UseSkin)
    {
        float4 skinnedPos = 0;
        
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            skinnedPos += mul(pos, Bones[vin.Joint[i]]) * vin.Weight[i];
        }
        
        pos = skinnedPos;
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

