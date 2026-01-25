#pragma once

namespace Imase
{
    // マテリアル
    struct Material
    {
        DirectX::XMFLOAT3 ambientColor;     // アンビエント色
        DirectX::XMFLOAT3 diffuseColor;     // ディフューズ色
        DirectX::XMFLOAT3 specularColor;    // スペキュラー色
        float specularPower;                // スペキュラーパワー
        DirectX::XMFLOAT3 emissiveColor;    // エミッシブ色
        int32_t textureIndex;               // テクスチャインデックス
    };

	class Effect
	{
    private:

        // 定数バッファのデータ
        struct ConstantBufferData
        {
            DirectX::XMVECTOR diffuseColor;             // ディフューズ色
            DirectX::XMVECTOR emissiveColor;            // エミッシブ色
            DirectX::XMVECTOR specularColorAndPower;    // スペキュラーとスペキュラーパワー

            DirectX::XMMATRIX world;                    // ワールド行列
            DirectX::XMMATRIX worldViewProjection;      // ワールド行列×ビュー行列×プロジェクション行列
            DirectX::XMMATRIX worldInverseTranspose;    // ワールド行列の逆転置行列

            DirectX::XMVECTOR lightDirection;           // ライトの方向ベクトル
            DirectX::XMFLOAT3 eyePosition;              // カメラの位置

            uint32_t useTexture;                        // 0 = 使わない, 1 = 使う
        };

        // 定数バッファ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

        // 入力レイアウト
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        // 頂点シェーダー
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

        // ピクセルシェーダー
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

        // サンプラーステート
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // ワールド行列
        DirectX::XMMATRIX m_world;

        // ビュー行列
        DirectX::XMMATRIX m_view;

        // プロジェクション行列
        DirectX::XMMATRIX m_projection;

        // ライトの方向
        DirectX::XMFLOAT3 m_lightDirection;

        // マテリアルへのポインタ
        const Imase::Material* m_pMaterial;

        // テクスチャハンドルの配列
        std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;

        // テクスチャのパス
        std::wstring m_path;

    public:

        // コンストラクタ
        Effect(ID3D11Device* device);

        // エフェクトを適応する関数
        void Apply(ID3D11DeviceContext* context);

        // ワールド行列を設定する関数
        void SetWorld(DirectX::XMMATRIX m) { m_world = m; }

        // ビュー行列を設定する関数
        void SetView(DirectX::XMMATRIX m) { m_view = m; }

        // プロジェクション行列を設定する関数
        void SetProjection(DirectX::XMMATRIX m) { m_projection = m; }

        // ライトの方向を設定する関数
        void SetLightDirection(DirectX::XMFLOAT3 v) { m_lightDirection = v; }

        // マテリアルを設定する関数
        void SetMaterial(const Imase::Material* pMaterial) { m_pMaterial = pMaterial; }
 
        // テクスチャ登録関数
        void SetTexture(ID3D11Device* device, const wchar_t* fname);

        // テクスチャのパスを設定する関数
        void SetDirectory(std::wstring path) { m_path = path; }

    };
}