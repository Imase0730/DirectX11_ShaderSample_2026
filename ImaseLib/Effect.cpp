#include "pch.h"
#include "Effect.h"

using namespace DirectX;
using namespace Imase;

// コンストラクタ
Imase::Effect::Effect(ID3D11Device* device, Imase::IShader* pShader)
    : m_pShader{ pShader }
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
}

// フレーム開始時の処理
void Imase::Effect::BeginFrame(ID3D11DeviceContext* context, Imase::PerFrameCB& cb)
{
    // 定数バッファ更新（フレーム開始時）
    UpdatePerFrameCB(context, cb);
}

// エフェクトを適応する関数
void Imase::Effect::Apply(ID3D11DeviceContext* context, const Imase::PerObjectCB& cb, const Material* material)
{
    // 定数バッファを更新（オブジェクト毎）
    UpdatePerObjectCB(context, cb);

    // シェーダーをバインド
    m_pShader->Bind(context);

    // 定数バッファを設定
    ID3D11Buffer* cbBuffers[] = { m_perFrameCB.Get(), m_perObjectCB.Get()};
    context->VSSetConstantBuffers(0, 2, cbBuffers);
    context->PSSetConstantBuffers(0, 2, cbBuffers);

    // サンプラーステートの設定
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    // ベースカラー
    if (material->pBaseColorSRV)
    {
        context->PSSetShaderResources(0, 1, &material->pBaseColorSRV);
    }

    // 法線マップ
    if (material->pNormalMapSRV)
    {
        context->PSSetShaderResources(1, 1, &material->pNormalMapSRV);
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

// 定数バッファ更新関数（フレーム更新時）
void Imase::Effect::UpdatePerFrameCB(ID3D11DeviceContext* context, const Imase::PerFrameCB& cb)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    context->Map(m_perFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(m_perFrameCB.Get(), 0);
}

// 定数バッファ更新関数（モデル毎に更新）
void Imase::Effect::UpdatePerObjectCB(ID3D11DeviceContext* context, const Imase::PerObjectCB& cb)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    context->Map(m_perObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(m_perObjectCB.Get(), 0);
}

