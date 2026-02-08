#include "Common.hlsli"
#include "Header.hlsli"

// ワールド空間 → タンジェント空間
float3x3 WorldToTangentMatrix(
   float3 tangent,
   float3 binormal,
   float3 normal)
{
    float3x3 mat =
    {
        tangent,
        binormal,
        normal,
    };
    return transpose(mat); // 転置
}

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換
    float4x4 wv = mul(World, View);
    float4x4 wvp = mul(wv, Projection);
    vout.Position = mul(float4(vin.Position, 1.0f), wvp);

    // ワールド座標（ライト計算用）
    float3 pos_ws = mul(float4(vin.Position, 1.0f), World);
    vout.WorldPos = pos_ws;

    float3 normal = normalize(mul(vin.Normal, (float3x3)WorldInverseTranspose));
    float3 tangent = normalize(mul(vin.Tangent.xyz, (float3x3)WorldInverseTranspose));
    float3 binormal = cross(normal, tangent) * vin.Tangent.w;

    // ワールド空間→タンジェント空間
    float3x3 TBN = WorldToTangentMatrix(tangent, binormal, normal);
    
    // ライト方向のベクトルをタンジェント空間へ
    vout.LightTangentDirect = normalize(mul(-LightDirection, TBN));

    // 視線方向のベクトルをタンジェント空間へ
    vout.ViewTangent = normalize(mul(CameraPosition - pos_ws, TBN));

    // テクスチャ座標
    vout.TexCoord = float2(vin.TexCoord.x, vin.TexCoord.y);

    return vout;
}

