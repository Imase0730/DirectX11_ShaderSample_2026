//--------------------------------------------------------------------------------------
// File: Effect.h
//
// 描画に必要な設定を行うクラス
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "ShaderBase.h"

namespace Imase
{
    // 定数バッファ変更フラグ
    namespace EffectDirtyFlags
    {
        constexpr uint32_t ConstantBuffer_b0 = 1 << 0;  // 定数バッファの役割（フレームで1回）
        constexpr uint32_t ConstantBuffer_b1 = 1 << 1;  // 定数バッファの役割（オブジェクト毎）

        constexpr uint32_t ViewProjection   = 1 << 2;   // b0
        constexpr uint32_t Light            = 1 << 3;   // b0
        constexpr uint32_t World            = 1 << 4;   // b1
        constexpr uint32_t Material         = 1 << 5;   // b1
    }

    // ライトの最大数
    static constexpr int LIGHT_MAX = 3;

    // フレームの最初に更新（b0）
    struct PerFrameCB
    {
        DirectX::XMMATRIX View;
        DirectX::XMMATRIX Projection;

        DirectX::XMFLOAT4 AmbientLightColor;

        DirectX::XMFLOAT4 LightDirection[LIGHT_MAX];
        DirectX::XMFLOAT4 LightDiffuseColor[LIGHT_MAX];
        DirectX::XMFLOAT4 LightSpecularColor[LIGHT_MAX];

        DirectX::XMFLOAT4 EyePosition;
    };

    // オブジェクト毎に更新（b1）
    struct PerObjectCB
    {
        DirectX::XMFLOAT4 DiffuseColor;
        DirectX::XMFLOAT4 EmissiveColor;
        DirectX::XMFLOAT4 SpecularColor;
        DirectX::XMFLOAT4 MaterialParams;   // x = SpecularPower, y = UseTexture, z = UseNormalMap

        DirectX::XMMATRIX World;
        DirectX::XMMATRIX WorldInverseTranspose;
    };

    // マテリアル
    struct Material
    {
        DirectX::XMFLOAT4 diffuseColor;     // ディフューズ色
        DirectX::XMFLOAT4 emissiveColor;    // エミッシブ色
        DirectX::XMFLOAT4 specularColor;    // スペキュラー色
        float specularPower;                // スペキュラーパワー

        ID3D11ShaderResourceView* pBaseColorSRV = nullptr;   // ベースカラー
        ID3D11ShaderResourceView* pNormalMapSRV = nullptr;   // 法線マップ
    };

    // ライト
    struct LightState
    {
        bool enable;
        DirectX::XMFLOAT3 direction;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular;
    };

    // ライトの状態
    struct LightingStates
    {
        LightState lights[LIGHT_MAX];
    };

	class Effect
	{
    private:

        // シェーダーへのポインタ
        Imase::ShaderBase* m_pShader;

        // 変更された情報を保持するフラグ
        uint32_t m_dirtyFlags;

        // ワールド行列
        DirectX::XMMATRIX m_world;

        // ビュー行列
        DirectX::XMMATRIX m_view;

        // プロジェクション行列
        DirectX::XMMATRIX m_projection;

        // マテリアル
        Material m_material;

        // グローバルアンビエント色
        DirectX::XMFLOAT4 m_ambientLightColor;

        // ライト
        Imase::LightingStates m_lightStates;

        // 定数バッファ（フレームの最初に更新用）
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_perFrameCB;

        // 定数バッファ（オブジェクト毎に更新用）
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_perObjectCB;

        // サンプラーステート
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

    public:

        // コンストラクタ
        Effect(ID3D11Device* device, Imase::ShaderBase* pShader);

        // フレーム最初に１回呼び出す関数
        void BeginFrame(ID3D11DeviceContext* context);

        // エフェクトを適応する関数
        void Apply(ID3D11DeviceContext* context);

        // ビュー行列とプロジェクション行列を設定する関数
        void SetViewProjection(DirectX::XMMATRIX view, DirectX::XMMATRIX projection);

        // 指定ライトの有効・無効を設定する関数
        void SetLightEnabled(int lightNo, bool value);

        // 指定ライトの方向を設定する関数
        void SetLightDirection(int lightNo, DirectX::XMFLOAT3 direction);

        // 指定ライトのディフューズ色を設定する関数
        void SetLightDiffuseColor(int lightNo, DirectX::XMVECTOR diffuseColor);

        // 指定ライトのスペキュラ色を設定する関数
        void SetLightSpecularColor(int lightNo, DirectX::XMVECTOR specularColor);

        // ワールド行列を設定する関数
        void SetWorld(const DirectX::XMMATRIX& world);

        // グローバルアンビエント色を設定する関数
        void SetAmbientLightColor(DirectX::XMVECTOR ambientColor);

        // マテリアルを設定する関数
        void SetMaterial(const Imase::Material& material);

        // ディフォルトライトの設定関数
        void EnableDefaultLighting();

    private:
    
        // 定数バッファ作成関数（フレーム更新時）
        void CreatePerFrameCB(ID3D11Device* device);

        // 定数バッファ更新関数（フレーム更新時）
        void UpdatePerFrameCB(ID3D11DeviceContext* context);

        // 定数バッファ作成関数（オブジェクト毎）
        void CreatePerObjectCB(ID3D11Device* device);

        // 定数バッファ更新関数（オブジェクト毎）
        void UpdatePerObjectCB(ID3D11DeviceContext* context);

        // ライトの番号を検証する関数
        void ValidateLightIndex(int lightNo);
    };
}