#include "Common.hlsli"
#include "NormalMap.hlsli"

Texture2D BaseColorTex  : register(t0);
Texture2D NormalTex     : register(t1);
Texture2D MetalRoughTex : register(t2);
SamplerState Sampler : register(s0);

float4 main(VSOutput pin) : SV_Target
{
    float roughness = Roughness;
    float metallic  = Metallic;

    // ラフネスとメタリック
    if (Flags & 0x4)
    {
        float2 mr = MetalRoughTex.Sample(Sampler, pin.TexCoord).gb;
        roughness = mr.x;
        metallic = mr.y;
    }

    // 法線マップサンプル　(法線マップが2チャンネル：BC5圧縮)
    float2 xy = NormalTex.Sample(Sampler, pin.TexCoord).rg * 2.0f - 1.0f;
    
    // Z再構築
    float z = sqrt(saturate(1.0f - dot(xy, xy)));

    // 法線
    float3 N = normalize(float3(xy, z));
    
    float3 normal = normalize(pin.NormalWS);
    float3 tangent = normalize(pin.TangentWS.xyz);

    // 再直交化
    tangent = normalize(tangent - normal * dot(normal, tangent));
    float3 binormal = cross(normal, tangent) * pin.TangentWS.w;

    // 接空間基底を並べて行列を作成（各ベクトルを「行」に配置）
    float3x3 TBN = float3x3(tangent, binormal, normal);

    // mul(ベクトル, 行列) とすることで、接空間(N)をワールド空間へ変換
    N = normalize(mul(N, TBN));
    
    // 視線ベクトル（ワールド空間）
    float3 V = normalize(EyePosition.xyz - pin.WorldPos);

    float4 albedo = BaseColor;

    // ベースカラー
    if (Flags & 0x1)
        albedo *= BaseColorTex.Sample(Sampler, pin.TexCoord);

//    float3 diffuseColor = albedo.rgb * (1.0f - Metallic); // PBR用今はしない
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
            float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, metallic);
            float power = lerp(2.0f, 256.0f, (1 - roughness) * (1 - roughness));
            float spec = pow(NdotH, power);
            spec *= (1 - roughness); // 強度減衰
            spec *= NdotL;
            specular += LightSpecularColor[i] * float4(F0, 1.0f) * spec;
        }
    }
 
    // 色
    float4 color = float4(EmissiveColor, 0.0f) + diffuse + specular;
    color.a = 1.0f;

    return color;
}
