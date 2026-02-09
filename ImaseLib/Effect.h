#pragma once

#include "IShader.h"

namespace Imase
{
    // フレームの最初に更新
    struct PerFrameCB
    {
        DirectX::XMMATRIX View;                     // ビュー行列
        DirectX::XMMATRIX Projection;               // プロジェクション行列

        DirectX::XMVECTOR LightDirection[3];        // 平行光源方向（正規化）
        DirectX::XMVECTOR LightDiffuseColor[3];     // ライトの色
        DirectX::XMVECTOR LightSpecularColor[3];    // スペキュラーの色

        DirectX::XMFLOAT3 EyePosition;              // カメラの位置
        float _pad_CameraPos;
    };

    // オブジェクト毎に更新
    struct PerObjectCB
    {
        DirectX::XMFLOAT3 DiffuseColor;     // ディフューズ色
        float _pad_Diffuse;

        DirectX::XMFLOAT3 EmissiveColor;    // エミッシブ色
        float _pad_Emissive;

        DirectX::XMFLOAT3 SpecularColor;    // スペキュラー色
        float SpecularPower;                // スペキュラーパワー

        uint32_t UseTexture;                // 0 or 1
        uint32_t UseNormalMap;              // 0 or 1
        DirectX::XMFLOAT2 _pad_Flags;

        DirectX::XMMATRIX World;                    // ワールド行列
        DirectX::XMMATRIX WorldInverseTranspose;    // ワールド行列の逆転置行列
    };

    // マテリアル
    struct Material
    {
        DirectX::XMFLOAT3 ambientColor;     // アンビエント色
        DirectX::XMFLOAT3 diffuseColor;     // ディフューズ色
        DirectX::XMFLOAT3 specularColor;    // スペキュラー色
        float specularPower;                // スペキュラーパワー
        DirectX::XMFLOAT3 emissiveColor;    // エミッシブ色

        ID3D11ShaderResourceView* pBaseColorSRV = nullptr;   // ベースカラー
        ID3D11ShaderResourceView* pNormalMapSRV = nullptr;   // 法線マップ
    };

	class Effect
	{
    private:

        // シェーダーへのポインタ
        Imase::IShader* m_pShader;

        // 定数バッファ（フレームの最初に更新用）
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_perFrameCB;

        // 定数バッファ（オブジェクト毎に更新用）
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_perObjectCB;

        // サンプラーステート
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

    public:

        // コンストラクタ
        Effect(ID3D11Device* device, Imase::IShader* pShader);

        // フレーム最初に１回呼び出す関数
        void BeginFrame(ID3D11DeviceContext* context, Imase::PerFrameCB& cb);

        // エフェクトを適応する関数
        void Apply(ID3D11DeviceContext* context, const Imase::PerObjectCB& cb, const Material* material);

    private:
    
        // 定数バッファ作成関数（フレーム更新時）
        void CreatePerFrameCB(ID3D11Device* device);

        // 定数バッファ更新関数（フレーム更新時）
        void UpdatePerFrameCB(ID3D11DeviceContext* context, const Imase::PerFrameCB& cb);

        // 定数バッファ作成関数（オブジェクト毎）
        void CreatePerObjectCB(ID3D11Device* device);

        // 定数バッファ更新関数（オブジェクト毎）
        void UpdatePerObjectCB(ID3D11DeviceContext* context, const Imase::PerObjectCB& cb);

    };
}