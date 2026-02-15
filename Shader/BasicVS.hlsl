#include "Common.hlsli"
#include "Basic.hlsli"

float3 ComputePBR(float3 N, float3 V, float3 L, float3 lightColor)
{
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float HdotV = saturate(dot(H, V));

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), BaseColor.rgb, Metallic);

    float3 F = FresnelSchlick(HdotV, F0);
    float D = DistributionGGX(N, H, Roughness);
    float G = GeometrySmith(N, V, L, Roughness);

    float3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - Metallic);

    float3 diffuse = kD * BaseColor.rgb / PI;

    return (diffuse + specular) * lightColor * NdotL;
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

    // ライト計算
    float3 Lo = 0;

    for (int i = 0; i < 3; i++)
    {
        float3 L = normalize(-LightDirection[i].xyz);
        Lo += ComputePBR(normal, eyeVector, L, LightDiffuseColor[i].rgb);
    }
    
    float3 ambient = AmbientLightColor.rgb * BaseColor.rgb;
    float3 finalColor = Lo + ambient + EmissiveColor.rgb;

    // ガンマ補正
    finalColor = pow(finalColor, 1.0 / 2.2);

    vout.Color = float4(finalColor, 1.0);

    // テクスチャ座標
    vout.TexCoord = vin.TexCoord;

    return vout;
}

