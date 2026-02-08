#include "pch.h"
#include "BasicShader.h"

using namespace DirectX;
using namespace Imase;

// コンストラクタ
Imase::BasicShader::BasicShader(ID3D11Device* device)
{
    const wchar_t* vs_name = L"Resources/Shaders/VertexShader.cso";
    const wchar_t* ps_name = L"Resources/Shaders/PixelShader.cso";

    // ----- 頂点シェーダー ＆ 入力レイアウト ----- //
    {
        // 頂点シェーダーの読み込み
        std::vector<uint8_t> data = DX::ReadData(vs_name);

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
        std::vector<uint8_t> data = DX::ReadData(ps_name);

        // ピクセルシェーダーの作成
        DX::ThrowIfFailed(
            device->CreatePixelShader(data.data(), data.size(), nullptr, m_pixelShader.ReleaseAndGetAddressOf())
        );
    }
}

// バインド関数
void Imase::BasicShader::Bind(ID3D11DeviceContext* context)
{
    // 頂点シェーダーの設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    // ピクセルシェーダーの設定
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 入力レイアウトの設定
    context->IASetInputLayout(m_inputLayout.Get());
}












    //// テクスチャの使用有無（0 : 未使用）
    //auto UseTexture = [&]()
    //{
    //    return (m_pMaterial->pBaseColorSRV == nullptr) ? 0 : 1;
    //};

    //// 定数バッファの更新
    //{
    //    ConstantBufferData data = {};

    //    // nullチェック
    //    assert(m_pMaterial);

    //    // マテリアル
    //    data.diffuseColor = XMLoadFloat3(&m_pMaterial->diffuseColor);   // ディフューズ色
    //    data.emissiveColor = XMLoadFloat3(&m_pMaterial->emissiveColor); // エミッシブ色
    //    data.specularColorAndPower = XMLoadFloat3(&m_pMaterial->specularColor); // スペキュラー色
    //    data.specularColorAndPower = XMVectorSetW(data.specularColorAndPower, m_pMaterial->specularPower);  // スペキュラーパワー

    //    // ワールド行列

    // ワールド行列
 //    data.world = XMMatrixTranspose(m_world);

 //    // ワールド行列×ビュー行列×プロジェクション行列
 //    XMMATRIX wvp = m_world * m_view * m_projection;
 //    data.worldViewProjection = XMMatrixTranspose(wvp);

 //    // ワールド行列の逆転置行列
 //    data.worldInverseTranspose = XMMatrixInverse(nullptr, m_world);

 //    // ライトの方向ベクトル
 //    data.lightDirection = XMLoadFloat3(&m_lightDirection);

 //    // カメラの位置をビュー行列から取得する
 //    XMMATRIX viewInverse = XMMatrixInverse(nullptr, m_view);
 //    XMStoreFloat3(&data.eyePosition, viewInverse.r[3]);

 //    // テクスチャの使用有無
 //    data.useTexture = UseTexture();

 //}

 //// 定数バッファの設定
 //ID3D11Buffer* cBuffers[] = { m_constantBuffer.Get() };
 //context->VSSetConstantBuffers(0, 1, cBuffers);
 //context->PSSetConstantBuffers(0, 1, cBuffers);

 //// サンプラーステートの設定
 //ID3D11SamplerState* samplers[] = { m_samplerState.Get() };
 //context->PSSetSamplers(0, 1, samplers);

 //// テクスチャの設定
 //if (m_pMaterial->pBaseColorSRV)
 //{
 //    // ディフューズ
 //    ID3D11ShaderResourceView* shaderResources[] = { m_pMaterial->pBaseColorSRV };
 //    context->PSSetShaderResources(0, 1, shaderResources);
 //}

 //if (m_pMaterial->pNormalMapSRV)
 //{
 //    // 法線マップ
 //    ID3D11ShaderResourceView* shaderResources[] = { m_pMaterial->pNormalMapSRV };
 //    context->PSSetShaderResources(1, 1, shaderResources);
 //}

//}

//UINT Imase::BasicShader::GetConstantBufferSize() const
//{
//    return sizeof(ConstantData);
//}
//
//const void* Imase::Shader::GetConstantData() const
//{
//    return &m_constantData;
//}
//
//void Imase::Shader::UpdatePerObject(ID3D11DeviceContext* context, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection)
//{
//    PerObjectCB cb = {};
//
//    cb.world = XMMatrixTranspose(world);
//    cb.worldViewProjection = XMMatrixTranspose(world * view * projection);
//    cb.worldInverseTranspose = XMMatrixInverse(nullptr, world);
//
//    // 定数バッファの更新
//    D3D11_MAPPED_SUBRESOURCE mapped;
//    context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
//    memcpy(mapped.pData, m_pShader->GetConstantData(), m_pShader->GetConstantBufferSize());
//    context->Unmap(m_constantBuffer.Get(), 0);
//
//    UpdateCnstants(context, cb);
//}
//
//void Imase::BasicShader::UpdateConstants(ID3D11DeviceContext* context, const void* data)
//{
//    context->UpdateSubresource(m_cb, 0, nullptr, data, 0, 0);
//}
