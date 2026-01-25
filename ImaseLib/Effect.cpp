#include "pch.h"
#include "Effect.h"

using namespace DirectX;
using namespace Imase;

// コンストラクタ
Imase::Effect::Effect(ID3D11Device* device)
    : m_world{}
    , m_view{}
    , m_projection{}
    , m_lightDirection{ 0.0f, 0.0f, -1.0f }
    , m_pMaterial{ nullptr }
{
    // ----- 定数バッファ ----- //
    {
        // 定数バッファの作成
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(ConstantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        DX::ThrowIfFailed(
            device->CreateBuffer(&desc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf())
        );
    }

    // ----- 頂点シェーダー ＆ 入力レイアウト ----- //
    {
        // 頂点シェーダーの読み込み
        std::vector<uint8_t> data = DX::ReadData(L"Resources/Shaders/VertexShader.cso");

        // 頂点シェーダーの作成
        DX::ThrowIfFailed(
            device->CreateVertexShader(data.data(), data.size(), nullptr, m_vertexShader.ReleaseAndGetAddressOf())
        );

        // 入力レイアウトの作成
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        DX::ThrowIfFailed(
            device->CreateInputLayout(layout, ARRAYSIZE(layout), data.data(), data.size(), m_inputLayout.ReleaseAndGetAddressOf())
        );
    }

    // ----- ピクセルシェーダー ----- //
    {
        // ピクセルシェーダーの読み込み
        std::vector<uint8_t> data = DX::ReadData(L"Resources/Shaders/PixelShader.cso");

        // ピクセルシェーダーの作成
        DX::ThrowIfFailed(
            device->CreatePixelShader(data.data(), data.size(), nullptr, m_pixelShader.ReleaseAndGetAddressOf())
        );
    }

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
}

// エフェクトを適応する関数
void Imase::Effect::Apply(ID3D11DeviceContext* context)
{
    // テクスチャの使用有無
    auto UseTexture = [&]()
    {
        if (m_pMaterial->textureIndex < 0) return 0;
        return 1;
    };

    // 定数バッファの更新
    {
        ConstantBufferData data = {};

        // nullチェック
        assert(m_pMaterial);

        // マテリアル
        data.diffuseColor = XMLoadFloat3(&m_pMaterial->diffuseColor);   // ディフューズ色
        data.emissiveColor = XMLoadFloat3(&m_pMaterial->emissiveColor); // エミッシブ色
        data.specularColorAndPower = XMLoadFloat3(&m_pMaterial->specularColor); // スペキュラー色
        data.specularColorAndPower = XMVectorSetW(data.specularColorAndPower, m_pMaterial->specularPower);  // スペキュラーパワー

        // ワールド行列
        data.world = XMMatrixTranspose(m_world);

        // ワールド行列×ビュー行列×プロジェクション行列
        XMMATRIX wvp = m_world * m_view * m_projection;
        data.worldViewProjection = XMMatrixTranspose(wvp);

        // ワールド行列の逆転置行列
        data.worldInverseTranspose = XMMatrixInverse(nullptr, m_world);

        // ライトの方向ベクトル
        data.lightDirection = XMLoadFloat3(&m_lightDirection);

        // カメラの位置をビュー行列から取得する
        XMMATRIX viewInverse = XMMatrixInverse(nullptr, m_view);
        XMStoreFloat3(&data.eyePosition, viewInverse.r[3]);

        // テクスチャの使用有無
        data.useTexture = UseTexture();

        D3D11_MAPPED_SUBRESOURCE mapped;
        context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

        // CPU側のバッファを書き換える
        memcpy(mapped.pData, &data, sizeof(data));

        context->Unmap(m_constantBuffer.Get(), 0);
    }

    // 定数バッファの設定
    ID3D11Buffer* cBuffers[] = { m_constantBuffer.Get() };
    context->VSSetConstantBuffers(0, 1, cBuffers);
    context->PSSetConstantBuffers(0, 1, cBuffers);

    // 頂点シェーダーの設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    // ピクセルシェーダーの設定
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 入力レイアウトの設定
    context->IASetInputLayout(m_inputLayout.Get());

    // サンプラーステートの設定
    ID3D11SamplerState* samplers[] = { m_samplerState.Get() };
    context->PSSetSamplers(0, 1, samplers);

    // テクスチャの使用
    if (UseTexture())
    {
        // シェーダーリソースの設定
        ID3D11ShaderResourceView* shaderResources[] = { m_textures[m_pMaterial->textureIndex].Get()};
        context->PSSetShaderResources(0, 1, shaderResources);
    }
}

// フルパス作成関数
static std::wstring MakeFullPath(const std::wstring& path, const std::wstring& fname)
{
    if (path.empty()) return fname;

    wchar_t sep = L'\\'; // Windows前提

    if (path.back() == L'\\' || path.back() == L'/')
    {
        return path + fname;
    }
    else
    {
        return path + sep + fname;
    }
}

// テクスチャハンドル登録関数
void Imase::Effect::SetTexture(ID3D11Device* device, const wchar_t* fname)
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

    // フルパス名を作成
    std::wstring fullPath = MakeFullPath(m_path, fname);

    // テクスチャ読み込み
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, fullPath.c_str(), nullptr, texture.GetAddressOf())
    );
    m_textures.push_back(std::move(texture));
}

