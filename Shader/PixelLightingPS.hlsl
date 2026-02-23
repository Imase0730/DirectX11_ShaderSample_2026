#include "Common.hlsli"
#include "PixelLighting.hlsli"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    // 法線
    float3 N = normalize(pin.NormalWS);
    
    // 視線ベクトル（ワールド空間）
    float3 V = normalize(EyePosition.xyz - pin.WorldPos);

    float4 albedo = BaseColor;

    // テクスチャ色
    if (Flags & 0x1)
        albedo *= Texture.Sample(Sampler, pin.TexCoord);
    
    float3 diffuseColor = albedo.rgb;
    // 金属は環境拡散は持たない
    float4 diffuse = AmbientLightColor * float4(diffuseColor, albedo.a);
    float4 specular = 0;

    [unroll]
    for (int i = 0; i < 3; i++)
    {
        float3 L = normalize(-LightDirection[i].xyz); // ライト方向へのベクトル
        float3 H = normalize(L + V); // ハーフベクトル

        // 法線と内積を取って光の強度を計算
        float NdotL = saturate(dot(N, L));
        float NdotH = saturate(dot(N, H));

        // ディフューズ
        diffuse += LightDiffuseColor[i] * float4(diffuseColor, albedo.a) * NdotL;

        if (NdotL > 0)
        {
            // スペキュラ（PBRのメタリックとラフネスから簡易方法で算出）
            // ※F0の意味　Fresnel 0° reflectance「入射角 0度（＝正面から見たとき）の反射率」
            float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, Metallic);
            float power = lerp(2.0f, 256.0f, (1 - Roughness) * (1 - Roughness));
            float spec = pow(NdotH, power);
            spec *= (1 - Roughness); // 強度減衰
            spec *= NdotL;
            specular += LightSpecularColor[i] * float4(F0, 1.0f) * spec;
        }
    }
 
    // 色
    float4 color = float4(EmissiveColor, 0.0f) + diffuse + specular;
    color.a = 1.0f;
  
    return color;
}