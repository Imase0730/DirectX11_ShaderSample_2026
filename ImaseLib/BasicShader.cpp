//--------------------------------------------------------------------------------------
// File: BasicShader.cpp
//
// ベーシックシェーダー
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "BasicShader.h"

using namespace DirectX;
using namespace Imase;

// コンストラクタ
Imase::BasicShader::BasicShader(ID3D11Device* device)
{
    const wchar_t* vs_name = L"Resources/Shaders/BasicVS.cso";
    const wchar_t* ps_name = L"Resources/Shaders/BasicPS.cso";

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

