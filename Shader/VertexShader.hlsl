#include "Header.hlsli"

struct ColorPair
{
    float3 Diffuse;
    float3 Specular;
};

// ライトの計算
ColorPair ComputeLight(float3 worldNormal, float3 eyeVector)
{
    ColorPair result;
    
    float3 N = normalize(worldNormal);      // 法線
    float3 L = normalize(-LightDirection);  // 光の来る方向
    float3 V = normalize(eyeVector);        // 視線方向

    // ディフューズ
    float NdotL = saturate(dot(N, L));
    result.Diffuse = DiffuseColor * NdotL;

    // スペキュラー
    float3 H = L + V;
    H = (dot(H, H) > 0.0001f) ? normalize(H) : N;   // ハーフベクトルのゼロ割り防止
    float NdotH = saturate(dot(N, H));
    float spec = pow(NdotH, SpecularPower);
    result.Specular = spec * SpecularColor * NdotL;
  
    return result;
}

VSOutput main(VSInput vin)
{
    VSOutput vout;

    // 座標変換
    vout.Position = mul(float4(vin.Position, 1.0f), WorldViewProj);

    // 視線ベクトル
    float4 pos_ws = mul(float4(vin.Position, 1.0f), World);
    float3 eyeVector = EyePosition - pos_ws.xyz;

    // ワールド空間の法線ベクトル
    float3 worldNormal = normalize(mul(vin.Normal, (float3x3)WorldInverseTranspose));
    
    // ライトの計算
    ColorPair light = ComputeLight(worldNormal, eyeVector);

    // ディフューズ色
    vout.Diffuse = float4(EmissiveColor + light.Diffuse + light.Specular, 1.0f);
   
    // テクスチャ座標
    vout.TexCoord = vin.TextCoord;
    
    // 接線ベクトル
    vout.Tangent = vin.Tangent;
    
    return vout;
}

