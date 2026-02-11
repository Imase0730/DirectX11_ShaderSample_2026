#include "Common.hlsli"
#include "NormalMap.hlsli"

Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);
SamplerState Sampler : register(s0);

float4 main(Varyings pin) : SV_Target
{
    // 法線マップサンプル
    float2 xy = NormalMap.Sample(Sampler, pin.TexCoord).rg * 2.0f - 1.0f;
    
    // Z再構築
    float z = sqrt(saturate(1.0f - dot(xy, xy)));

    // 法線
    float3 N = normalize(float3(xy, z));
    
    float3 normal = normalize(pin.NormalWS);
    float3 tangent = normalize(pin.TangentWS.xyz);
    // 再直交化
    tangent = normalize(tangent - normal * dot(normal, tangent));
    float3 binormal = cross(normal, tangent) * pin.TangentWS.w;

    // 接線空間→ワールド空間
    float3x3 TBN = transpose(float3x3(
        tangent,
        binormal,
        normal
    ));
    
    // 法線をワールド空間へ変換
    N = normalize(mul(TBN, N));

    // 視線ベクトル（ワールド空間）
    float3 V = normalize(EyePosition.xyz - pin.WorldPos);

    float4 diffuse = EmissiveColor + AmbientLightColor * DiffuseColor;
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
        diffuse += LightDiffuseColor[i] * DiffuseColor * NdotL;

        if (NdotL > 0)
        {
            // スペキュラ（MaterialParams.x = SpecularPower）
            float spec = pow(NdotH, MaterialParams.x);
            // エネルギー保存近似（ハイライトを自然に見せるため）
            spec *= (MaterialParams.x + 2.0f) / 8.0f;
            spec *= NdotL;
            specular += LightSpecularColor[i] * SpecularColor * spec;
        }
    }
 
    // 色
    float4 color = diffuse + specular;
    color.a = 1.0f;

    // テクスチャ
    color *= lerp(1.0f, Texture.Sample(Sampler, pin.TexCoord), MaterialParams.y);

    return color;
}