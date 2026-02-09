#include "Common.hlsli"
#include "Header.hlsli"

struct ColorPair
{
    float3 Diffuse;
    float3 Specular;
};

// Blinn-Phong
ColorPair ComputeLights(float3 eyeVector, float3 normal)
{
    float3 N = normalize(normal);
    float3 V = normalize(eyeVector);

    ColorPair result;
    result.Diffuse = EmissiveColor;
    result.Specular = 0;

    for (int i = 0; i < 3; i++)
    {
        float3 L = normalize(-LightDirection[i]);   // ライト方向へのベクトル
        float3 H = normalize(L + V);                // ハーフベクトル

        // 法線と内積を取って光の強度を計算
        float NdotL = saturate(dot(N, L));
        float NdotH = saturate(dot(N, H));

        // ディフューズ
        result.Diffuse += LightDiffuseColor[i] * DiffuseColor.rgb * NdotL;

        // 影の部分はスペキュラなし
        if (NdotL > 0)
        {
            // スペキュラ
            result.Specular += LightSpecularColor[i] * SpecularColor * pow(NdotH, SpecularPower);
        }
    }
    return result;
}

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換
    float4x4 wv = mul(World, View);
    float4x4 wvp = mul(wv, Projection);
    vout.Position = mul(float4(vin.Position, 1.0f), wvp);

    // 頂点の位置をワールド空間へ
    float3 pos_ws = mul(float4(vin.Position, 1.0f), World);

    // カメラへの方向ベクトル
    float3 eyeVector = normalize(EyePosition - pos_ws.xyz);

    // 法線ベクトルをワールド空間へ
    float3 normal = normalize(mul(vin.Normal, (float3x3) WorldInverseTranspose));

    // ライトの計算
    ColorPair result = ComputeLights(eyeVector, normal);

    // 色
    vout.Color = float4(result.Diffuse + result.Specular, 1.0f);
    
    // テクスチャ座標
    vout.TexCoord = vin.TexCoord;

    return vout;
}





//// ワールド空間 → タンジェント空間
//float3x3 WorldToTangentMatrix(
//   float3 tangent,
//   float3 binormal,
//   float3 normal)
//{
//    float3x3 mat =
//    {
//        tangent,
//        binormal,
//        normal,
//    };
//    return transpose(mat); // 転置
//}

//VSOutput main(VSInput vin)
//{
//    VSOutput vout;

//    // 座標変換
//    float4x4 wv = mul(World, View);
//    float4x4 wvp = mul(wv, Projection);
//    vout.Position = mul(float4(vin.Position, 1.0f), wvp);

//    // ワールド座標（ライト計算用）
//    float3 pos_ws = mul(float4(vin.Position, 1.0f), World);
//    vout.WorldPos = pos_ws;

//    float3 normal = normalize(mul(vin.Normal, (float3x3)WorldInverseTranspose));
//    float3 tangent = normalize(mul(vin.Tangent.xyz, (float3x3)WorldInverseTranspose));
//    float3 binormal = cross(normal, tangent) * vin.Tangent.w;

//    // ワールド空間→タンジェント空間
//    float3x3 TBN = WorldToTangentMatrix(tangent, binormal, normal);
    
//    // ライト方向のベクトルをタンジェント空間へ
//    vout.LightTangentDirect = normalize(mul(-LightDirection[0], TBN));

//    // 視線方向のベクトルをタンジェント空間へ
//    vout.ViewTangent = normalize(mul(CameraPosition - pos_ws, TBN));

//    // テクスチャ座標
//    vout.TexCoord = float2(vin.TexCoord.x, vin.TexCoord.y);

//    return vout;
//}

