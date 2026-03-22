#ifndef LIGHTING
#define LIGHTING

#include "Common.hlsli"

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
    result.Diffuse = float4(EmissiveColor, 1.0f) + AmbientLightColor * BaseColor;
    result.Specular = 0;

    [unroll]
    for (int i = 0; i < 3; i++)
    {
        float3 L = normalize(-LightDirection[i].xyz); // ライト方向へのベクトル
        float3 H = normalize(L + V); // ハーフベクトル

        // 内積を取って光の強度を計算
        float NdotL = saturate(dot(N, L));
        float NdotH = saturate(dot(N, H));

        // ディフューズ
        result.Diffuse += LightDiffuseColor[i] * BaseColor * NdotL;

        // 影の部分はスペキュラなし
        if (NdotL > 0)
        {
            // スペキュラ（メタリックとラフネスから簡易方法で算出）
            // 非金属の反射率（F0）は一般的に 0.04 (4%) 固定です
            float3 f0 = float3(0.04f, 0.04f, 0.04f);
            float3 specColor = lerp(f0, float3(1.0f, 1.0f, 1.0f), Metallic);

            float perceptualRoughness = (1.0f - Roughness) * (1.0f - Roughness);
            float specPower = perceptualRoughness * 1000.0f;

            float specularIntensity = pow(NdotH, specPower);

            result.Specular += LightSpecularColor[i] * float4(specColor * specularIntensity * NdotL, 1.0f);
        }
    }

    // 放射光を加算    
    result.Diffuse += float4(EmissiveColor, 0.0f);

    return result;
}

#endif // LIGHTING