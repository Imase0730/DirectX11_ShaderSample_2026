//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include "ImaseLib/DebugCamera.h"
#include "ImaseLib/GridFloor.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

private:

    // コモンステート
    std::unique_ptr<DirectX::CommonStates> m_states;

    // プロジェクション行列
    DirectX::SimpleMath::Matrix m_proj;

    // デバッグカメラ
    std::unique_ptr<Imase::DebugCamera> m_debugCamera;

    // グリッドの床
    std::unique_ptr<Imase::GridFloor> m_gridFloor;

    // --------------------------------------------------------- //

    // 定数バッファの構造体
    struct ConstantBuffer
    {
        DirectX::XMMATRIX worldViewProjection;  // ワールド行列 × ビュー行列 × プロジェクション行列
    };

    // 頂点データの構造体
    struct VertexPositionTexture
    {
        DirectX::XMFLOAT3 position;     // 位置
        DirectX::XMFLOAT2 texCoord;     // テクスチャ座標
    };

    // 定数バッファ（IA）
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

    // 頂点バッファ（IA）
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

    // インデックスバッファ（IA）
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

    // 入力レイアウト（IA)
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    // 頂点シェーダー（VS）
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

    // ラスタライザーステート（RS)
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

    // ピクセルシェーダー（PS）
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

    // 深度ステンシルステート（OM）
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    // ブレンドステート（OM）
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

    // サンプラーステート（PS）
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

    // シェダーリソースビュー
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

};
