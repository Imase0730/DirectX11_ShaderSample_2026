//--------------------------------------------------------------------------------------
// File: Effect.cpp
//
// 描画に必要な設定を行うクラス
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Effect.h"

using namespace DirectX;
using namespace Imase;

// コンストラクタ
Imase::Effect::Effect(ID3D11Device* device, Imase::ShaderBase* pShader)
    : m_pShader{ pShader }
    , m_dirtyFlags{ 0xFFFFFFFF }
    , m_world{}
    , m_view{}
    , m_projection{}
    , m_ambientLightColor{}
    , m_textures{}
    , m_materials{}
    , m_materialIndex{}
    , m_lightStates{}
{
    // ----- サンプラーステート ----- //
    {
        // サンプラーテートの作成
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MaxLOD = FLT_MAX;
        DX::ThrowIfFailed(
            device->CreateSamplerState(&desc, m_samplerState.ReleaseAndGetAddressOf())
        );
    }

    // 定数バッファ作成
    CreatePerFrameCB(device);
    CreatePerObjectCB(device);
    CreatePerMaterialCB(device);

    // ディフォルトライトの設定
    EnableDefaultLighting();
}

// フレーム開始時の処理
void Imase::Effect::BeginFrame(ID3D11DeviceContext* context)
{
    // ビュー行列とプロジェクション行列が変更された
    if (m_dirtyFlags & EffectDirtyFlags::ViewProjection)
    {
        m_dirtyFlags &= ~EffectDirtyFlags::ViewProjection;
        m_dirtyFlags |= EffectDirtyFlags::ConstantBuffer_b0;
    }

    // ライトが変更された
    if (m_dirtyFlags & EffectDirtyFlags::Light)
    {
        m_dirtyFlags &= ~EffectDirtyFlags::Light;
        m_dirtyFlags |= EffectDirtyFlags::ConstantBuffer_b0;
    }

    // 変更があれば定数バッファを更新
    if (m_dirtyFlags & EffectDirtyFlags::ConstantBuffer_b0)
    {
        UpdatePerFrameCB(context);
        m_dirtyFlags &= ~EffectDirtyFlags::ConstantBuffer_b0;
    }
}

// エフェクトを適応する関数
void Imase::Effect::Apply(ID3D11DeviceContext* context)
{
    // ワールド行列が変更された
    if (m_dirtyFlags & EffectDirtyFlags::World)
    {
        m_dirtyFlags &= ~EffectDirtyFlags::World;
        m_dirtyFlags |= EffectDirtyFlags::ConstantBuffer_b1;
    }

    // マテリアルが変更された
    if (m_dirtyFlags & EffectDirtyFlags::Material)
    {
        m_dirtyFlags &= ~EffectDirtyFlags::Material;
        m_dirtyFlags |= EffectDirtyFlags::ConstantBuffer_b2;
    }

    // 変更があれば定数バッファを更新
    if (m_dirtyFlags & EffectDirtyFlags::ConstantBuffer_b1)
    {
        // 定数バッファを更新（オブジェクト毎）
        UpdatePerObjectCB(context);
        m_dirtyFlags &= ~EffectDirtyFlags::ConstantBuffer_b1;
    }
 
    // 変更があれば定数バッファを更新
    if (m_dirtyFlags & EffectDirtyFlags::ConstantBuffer_b2)
    {
        // 定数バッファを更新（マテリアル）
        UpdatePerMaterialCB(context);
        m_dirtyFlags &= ~EffectDirtyFlags::ConstantBuffer_b2;
    }

    // シェーダーをバインド
    m_pShader->Bind(context);

    // 定数バッファを設定
    ID3D11Buffer* cbBuffers[] = { m_perFrameCB.Get(), m_perObjectCB.Get(), m_perMaterialCB.Get() };
    context->VSSetConstantBuffers(0, 3, cbBuffers);
    context->PSSetConstantBuffers(0, 3, cbBuffers);

    // サンプラーステートの設定
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    // ベースカラー
    if (m_materials[m_materialIndex].baseColorTexIndex >= 0)
    {
        ID3D11ShaderResourceView* srv[] = { m_textures[m_materials[m_materialIndex].baseColorTexIndex].Get() };
        context->PSSetShaderResources(0, 1, srv);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[] = { nullptr };
        context->PSSetShaderResources(0, 1, nullSRV);
    }

    // 法線マップ
    if (m_materials[m_materialIndex].normalTexIndex >= 0)
    {
        ID3D11ShaderResourceView* srv[] = { m_textures[m_materials[m_materialIndex].normalTexIndex].Get() };
        context->PSSetShaderResources(1, 1, srv);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[] = { nullptr };
        context->PSSetShaderResources(1, 1, nullSRV);
    }
}

// ビュー行列とプロジェクション行列を設定する関数
void Imase::Effect::SetViewProjection(DirectX::XMMATRIX view, DirectX::XMMATRIX projection)
{
    m_view = view;
    m_projection = projection;
    m_dirtyFlags |= EffectDirtyFlags::ViewProjection;
}

// 指定ライトの有効・無効を設定する関数
void Imase::Effect::SetLightEnabled(int lightNo, bool value)
{
    ValidateLightIndex(lightNo);
    m_lightStates.lights[lightNo].enable = value;
    m_dirtyFlags |= EffectDirtyFlags::Light;
}

// 指定ライトの方向を設定する関数
void Imase::Effect::SetLightDirection(int lightNo, DirectX::XMFLOAT3 direction)
{
    ValidateLightIndex(lightNo);
    m_lightStates.lights[lightNo].direction = direction;
    m_dirtyFlags |= EffectDirtyFlags::Light;
}

// 指定ライトのディフューズ色を設定する関数
void Imase::Effect::SetLightDiffuseColor(int lightNo, DirectX::XMVECTOR diffuseColor)
{
    ValidateLightIndex(lightNo);
    XMStoreFloat4(&m_lightStates.lights[lightNo].diffuse, diffuseColor);
    m_dirtyFlags |= EffectDirtyFlags::Light;
}

// 指定ライトのスペキュラ色を設定する関数
void Imase::Effect::SetLightSpecularColor(int lightNo, DirectX::XMVECTOR specularColor)
{
    ValidateLightIndex(lightNo);
    XMStoreFloat4(&m_lightStates.lights[lightNo].specular, specularColor);
    m_dirtyFlags |= EffectDirtyFlags::Light;
}

// ワールド行列を設定する関数
void Imase::Effect::SetWorld(const DirectX::XMMATRIX& world)
{
    m_world = world;
    m_dirtyFlags |= EffectDirtyFlags::World;
}

// グローバルアンビエント色を設定する関数
void Imase::Effect::SetAmbientLightColor(DirectX::XMVECTOR ambientColor)
{
    XMStoreFloat4(&m_ambientLightColor, ambientColor);
    m_dirtyFlags |= EffectDirtyFlags::Light;
}

// テクスチャのシェダーリソースを作成して登録する関数
void Imase::Effect::RegisterTextures(ID3D11Device* device, std::vector<TextureEntry>& textures)
{
    m_textures.resize(textures.size());
    for (size_t i = 0; i < textures.size(); i++)
    {
        DX::ThrowIfFailed(
            CreateDDSTextureFromMemory(
                device,
                textures[i].data.data(), textures[i].data.size(),
                nullptr,
                m_textures[i].ReleaseAndGetAddressOf())
        );
    }
}

// マテリアルを登録する関数
void Imase::Effect::RegisterMaterials(const std::vector<MaterialInfo>& materials)
{
    for (size_t i = 0; i < materials.size(); i++)
    {
        m_materials.emplace_back(materials[i]);
    }
}

// マテリアルを設定する関数
void Imase::Effect::SetMaterialIndex(uint32_t materialIndex)
{
    m_materialIndex = materialIndex;
    m_dirtyFlags |= EffectDirtyFlags::Material;
}

// ディフォルトライトの設定関数
void Imase::Effect::EnableDefaultLighting()
{
    static const XMFLOAT3 defaultDirections[LIGHT_MAX] =
    {
        { -0.5265408f, -0.5735765f, -0.6275069f  },
        {  0.7198464f,  0.3420201f,  0.6040227f  },
        {  0.4545195f, -0.7660444f,  0.4545195f  },
    };

    static const XMVECTOR defaultDiffuse[LIGHT_MAX] =
    {
        {  1.0000000f, 0.9607844f, 0.8078432f, },
        {  0.9647059f, 0.7607844f, 0.4078432f, },
        {  0.3231373f, 0.3607844f, 0.3937255f, },
    };

    static const XMVECTOR defaultSpecular[LIGHT_MAX] =
    {
        { 1.0000000f, 0.9607844f, 0.8078432f, },
        { 0.0000000f, 0.0000000f, 0.0000000f, },
        { 0.3231373f, 0.3607844f, 0.3937255f, },
    };

    static const XMVECTOR defaultAmbient = { 0.05333332f, 0.09882354f, 0.1819608f };

    SetAmbientLightColor(defaultAmbient);

    for (int i = 0; i < LIGHT_MAX; i++)
    {
        SetLightEnabled(i, true);
        SetLightDirection(i, defaultDirections[i]);
        SetLightDiffuseColor(i, defaultDiffuse[i]);
        SetLightSpecularColor(i, defaultSpecular[i]);
    }
}

// 定数バッファの作成関数（フレーム更新時）
void Imase::Effect::CreatePerFrameCB(ID3D11Device* device)
{
    // 定数バッファの作成
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(Imase::PerFrameCB);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    DX::ThrowIfFailed(
        device->CreateBuffer(&desc, nullptr, m_perFrameCB.ReleaseAndGetAddressOf())
    );
}

// 定数バッファの作成関数（モデル毎に更新）
void Imase::Effect::CreatePerObjectCB(ID3D11Device* device)
{
    // 定数バッファの作成
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(Imase::PerObjectCB);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    DX::ThrowIfFailed(
        device->CreateBuffer(&desc, nullptr, m_perObjectCB.ReleaseAndGetAddressOf())
    );
}

// 定数バッファの作成関数（マテリアル）
void Imase::Effect::CreatePerMaterialCB(ID3D11Device* device)
{
    // 定数バッファの作成
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(Imase::PerMaterialCB);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    DX::ThrowIfFailed(
        device->CreateBuffer(&desc, nullptr, m_perMaterialCB.ReleaseAndGetAddressOf())
    );
}

// 定数バッファ更新関数（フレーム更新時）
void Imase::Effect::UpdatePerFrameCB(ID3D11DeviceContext* context)
{
    Imase::PerFrameCB cb = {};

    // ビュー行列
    cb.View = m_view;

    // 射影行列
    cb.Projection = m_projection;

    // カメラの位置
    XMMATRIX viewInverse = XMMatrixInverse(nullptr, m_view);
    XMStoreFloat4(&cb.EyePosition, viewInverse.r[3]);

    // グローバルアンビエント色
    cb.AmbientLightColor = m_ambientLightColor;
 
    // ライト
    for (int i = 0; i < LIGHT_MAX; i++)
    {
        if (m_lightStates.lights[i].enable)
        {
            XMVECTOR dir = XMLoadFloat3(&m_lightStates.lights[i].direction);
            dir = XMVector3Normalize(dir);
            XMStoreFloat4(&cb.LightDirection[i], XMVectorSetW(dir, 0.0f));
            cb.LightDiffuseColor[i] = m_lightStates.lights[i].diffuse;
            cb.LightSpecularColor[i] = m_lightStates.lights[i].specular;
        }
        else
        {
            cb.LightDirection[i] = { 0.0f, 0.0f, 0.0f, 0.0f };
            cb.LightDiffuseColor[i] = { 0.0f, 0.0f, 0.0f, 0.0f };
            cb.LightSpecularColor[i] = { 0.0f, 0.0f, 0.0f, 0.0f };
        }
    }

    // 定数バッファ更新(b0)
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    DX::ThrowIfFailed(
        context->Map(m_perFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)
    );
    memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(m_perFrameCB.Get(), 0);
}

// 定数バッファ更新関数（モデル毎に更新）
void Imase::Effect::UpdatePerObjectCB(ID3D11DeviceContext* context)
{
    Imase::PerObjectCB cb = {};

    // ワールド行列
    cb.World = m_world;
    // ワールド行列の逆転置行列
    cb.WorldInverseTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, m_world));

    // 定数バッファ更新(b1)
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    DX::ThrowIfFailed(
        context->Map(m_perObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)
    );
    memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(m_perObjectCB.Get(), 0);
}

// 定数バッファ更新関数（マテリアル）
void Imase::Effect::UpdatePerMaterialCB(ID3D11DeviceContext* context)
{
    Imase::PerMaterialCB cb = {};

    const MaterialInfo& m = m_materials[m_materialIndex];

    cb.BaseColor = m.diffuseColor;
    cb.EmissiveColor = m.emissiveColor;
    cb.Metallic = m.metallicFactor;
    cb.Roughness = m.roughnessFactor;
    if (m.baseColorTexIndex >= 0) cb.Flags |= FLAG_BASECOLOR_TEX;
    if (m.normalTexIndex >= 0) cb.Flags |= FLAG_NORMALMAP_TEX;

    // 定数バッファ更新(b2)
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    DX::ThrowIfFailed(
        context->Map(m_perMaterialCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)
    );
    memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(m_perMaterialCB.Get(), 0);
}

// ライトの番号を検証する関数
void Imase::Effect::ValidateLightIndex(int lightNo)
{
    if (lightNo < 0 || lightNo >= LIGHT_MAX)
    {
        throw std::invalid_argument("lightNo invalid");
    }
}

