
// 定数バッファ：フレーム毎の更新
cbuffer PerFrameCB : register(b0)
{
    float4x4 View : packoffset(c0);
    float4x4 Projection : packoffset(c4);

    float3 LightDirection : packoffset(c8);
    float _pad_LightDir : packoffset(c8.w);

    float3 CameraPosition : packoffset(c9);
    float _pad_CameraPos : packoffset(c9.w);
};

// 定数バッファ：オブジェクト毎の更新
cbuffer PerObjectCB : register(b1)
{
    float3 DiffuseColor : packoffset(c0);
    float _pad_Diffuse : packoffset(c0.w);

    float3 EmissiveColor : packoffset(c1);
    float _pad_Emissive : packoffset(c1.w);

    float3 SpecularColor : packoffset(c2);
    float SpecularPower : packoffset(c2.w);

    uint UseTexture : packoffset(c3.x);
    uint UseNormalMap : packoffset(c3.y);
    float2 _pad_Flags : packoffset(c3.z);

    float4x4 World : packoffset(c4);
    float4x4 WorldInverseTranspose : packoffset(c8);
};
