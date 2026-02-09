
// 定数バッファ：フレーム毎の更新
cbuffer PerFrameCB : register(b0)
{
    float4x4 View                   : packoffset(c0);   // ビュー行列
    float4x4 Projection             : packoffset(c4);   // プロジェクション行列

    float3 LightDirection[3]        : packoffset(c8);   // ライトの向き
    float3 LightDiffuseColor[3]     : packoffset(c11);  // ライトのディフューズ色
    float3 LightSpecularColor[3]    : packoffset(c14);  // ライトのスペキュラー色
    
    float3 EyePosition              : packoffset(c17);  // カメラの位置
};

// 定数バッファ：オブジェクト毎の更新
cbuffer PerObjectCB : register(b1)
{
    float3 DiffuseColor             : packoffset(c0);   // ディフューズ色
    float _pad_Diffuse              : packoffset(c0.w);

    float3 EmissiveColor            : packoffset(c1);   // エミッシブ色
    float _pad_Emissive             : packoffset(c1.w);

    float3 SpecularColor            : packoffset(c2);   // スペキュラー色
    float SpecularPower             : packoffset(c2.w);

    uint UseTexture                 : packoffset(c3.x); // テクスチャ使用の有無
    uint UseNormalMap               : packoffset(c3.y); // 法線マップ使用の有無
    float2 _pad_Flags               : packoffset(c3.z);

    float4x4 World                  : packoffset(c4);   // ワールド行列
    float4x4 WorldInverseTranspose  : packoffset(c8);   // ワールド行列の逆転置行列
};
