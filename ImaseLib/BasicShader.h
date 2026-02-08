#pragma once

#include "IShader.h"

namespace Imase
{
	class BasicShader : public IShader
	{
    private:

        // 入力レイアウト
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        // 頂点シェーダー
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

        // ピクセルシェーダー
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

    public:

        // コンストラクタ
        BasicShader(ID3D11Device* device);

        // バインド関数
        void Bind(ID3D11DeviceContext* context) override;

    };
}

