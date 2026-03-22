#include "Common.hlsli"
#include "Lighting.hlsli"
#include "Basic.hlsli"

VSOutput main(VSInput vin)
{
    VSOutput vout;

    float4 pos = float4(vin.Position, 1.0);

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

    // カメラへの方向ベクトル
    float3 eyeVector = normalize(EyePosition.xyz - worldPos.xyz);

    // 法線ベクトルをワールド空間へ
    float3 normal = normalize(mul((float3x3)WorldInverseTranspose, vin.Normal));

    // ライトの計算
    ColorPair result = ComputeLights(eyeVector, normal);

    // ディフューズ色
    vout.Diffuse = result.Diffuse;
    
    // スペキュラ色
    vout.Specular = result.Specular;
    
    // テクスチャ座標
    vout.TexCoord = vin.TexCoord;

    return vout;
}
