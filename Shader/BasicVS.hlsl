#include "Common.hlsli"
#include "Basic.hlsli"

struct ColorPair
{
    float4 Diffuse;
    float4 Specular;
};

// ライトの計算（Blinn-Phong）
ColorPair ComputeLights(float3 eyeVector, float3 normal)
{
    float3 N = normalize(normal);
    float3 V = normalize(eyeVector);

    ColorPair result;
    result.Diffuse = EmissiveColor + AmbientLightColor * DiffuseColor;
    result.Specular = 0;

    [unroll]
    for (int i = 0; i < 3; i++)
    {
        float3 L = normalize(-LightDirection[i].xyz);   // ライト方向へのベクトル
        float3 H = normalize(L + V);                // ハーフベクトル

        // 法線と内積を取って光の強度を計算
        float NdotL = saturate(dot(N, L));
        float NdotH = saturate(dot(N, H));

        // ディフューズ
        result.Diffuse += LightDiffuseColor[i] * DiffuseColor * NdotL;

        // 影の部分はスペキュラなし
        if (NdotL > 0)
        {
            // スペキュラ（MaterialParams.x = SpecularPower）
            result.Specular += LightSpecularColor[i] * SpecularColor * pow(NdotH, MaterialParams.x);
        }
    }
    return result;
}

Varyings main(VSInput vin)
{
    Varyings vout;

    // 座標変換
    float4 worldPos = mul(World, float4(vin.Position, 1.0f));
    float4 viewPos = mul(View, worldPos);
    vout.Position = mul(Projection, viewPos);

    // カメラへの方向ベクトル
    float3 eyeVector = normalize(EyePosition.xyz - worldPos.xyz);

    // 法線ベクトルをワールド空間へ
    float3 normal = normalize(mul((float3x3)WorldInverseTranspose, vin.Normal));

    // ライトの計算
    ColorPair result = ComputeLights(eyeVector, normal);

    // 色
    vout.Color = result.Diffuse + result.Specular;
    vout.Color.a = 1.0f;
    
    // テクスチャ座標
    vout.TexCoord = vin.TexCoord;

    return vout;
}

