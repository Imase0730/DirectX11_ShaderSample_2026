//--------------------------------------------------------------------------------------
// File: ShaderBase.h
//
// シェーダーの基底クラス
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

namespace Imase
{
    enum class ShaderStage : uint32_t
    {
        None = 0,
        VS = 1 << 0, 
        PS = 1 << 1,
        GS = 1 << 2,
        HS = 1 << 3,
        DS = 1 << 4,
        CS = 1 << 5,

        All = VS | PS | GS | HS | DS | CS
    };

    struct UserConstantBuffer
    {
        ID3D11Buffer* buffer;   // 定数バッファ
        UINT slot;              // スロット
        ShaderStage stage;      // 使用ステージ
    };

    class ShaderBase
    {
    public:

        // コンストラクタ
        ShaderBase(
            ID3D11Device* device,
            const wchar_t* vsFile,
            const wchar_t* psFile
        )
        {
            // 頂点シェーダー作成
            std::vector<uint8_t> vsData = DX::ReadData(vsFile);

            DX::ThrowIfFailed(
                device->CreateVertexShader(
                    vsData.data(),
                    vsData.size(),
                    nullptr,
                    m_vertexShader.ReleaseAndGetAddressOf())
            );

            // 入力レイアウト作成
            CreateInputLayout(device, vsData);

            // ピクセルシェーダー作成
            std::vector<uint8_t> psData = DX::ReadData(psFile);

            DX::ThrowIfFailed(
                device->CreatePixelShader(
                    psData.data(),
                    psData.size(),
                    nullptr,
                    m_pixelShader.ReleaseAndGetAddressOf())
            );
        }

        // デストラクタ
        virtual ~ShaderBase() = default;

        // シェーダー・入力レイアウトをバインド
        virtual void Bind(ID3D11DeviceContext* context)
        {
            context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
            context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
            context->IASetInputLayout(m_inputLayout.Get());
        }

    protected:

        // 入力レイアウト作成
        void CreateInputLayout(
            ID3D11Device* device,
            const std::vector<uint8_t>& vsData
        )
        {
            static const D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            DX::ThrowIfFailed(
                device->CreateInputLayout(
                    layout,
                    ARRAYSIZE(layout),
                    vsData.data(),
                    vsData.size(),
                    m_inputLayout.ReleaseAndGetAddressOf()));
        }

    protected:

        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;

    public:

        // UserCBを持つか？
        virtual bool HasUserCB() const { return false; }

        // UserCBのバッファを取得
        virtual void GetUserCBs(std::vector<UserConstantBuffer>& out) const
        {
            out.clear();
        }

        // UserCBの更新
        virtual void UpdateUserCBs(ID3D11DeviceContext* context)
        {
            UNREFERENCED_PARAMETER(context);
        }
    };
}
