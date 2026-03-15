//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "A_anim.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    // デバッグカメラの作成
    m_debugCamera = std::make_unique<Imase::DebugCamera>(width, height);

    // ライトの方向の初期化
    m_lightDirection = SimpleMath::Vector3(0.0f, -1.0f, 0.0f);
    m_lightDirection.Normalize();

    const std::vector<std::string> name = m_animator->GetAnimationNames();

    m_animator->Play(AnimationIndex::walk, true);

}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    // デバッグカメラの更新
    m_debugCamera->Update();

    auto key = Keyboard::Get().GetState();

    if (key.A && m_animator->GetCurrentAnimationIndex() == AnimationIndex::walk)
    {
        m_animator->CrossFade(AnimationIndex::attack, 0.5f);
    }

    if ( m_animator->GetCurrentAnimationIndex() == AnimationIndex::attack
      && m_animator->GetRestTime() < 0.5f )
    {
        m_animator->CrossFade(AnimationIndex::walk, 0.5f);
    }

    // アニメーションの更新
    m_animator->Update(elapsedTime);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.

    // ビュー行列を取得する
    SimpleMath::Matrix view = m_debugCamera->GetCameraMatrix();

    // グリッドの床の描画
    m_gridFloor->Render(context, view, m_proj);

    // ------------------------------------------------------- //
    // モデル描画 
    // ------------------------------------------------------- //

    // ビュー行列とプロジェクション行列を設定
    Imase::Effect* effect = m_model->GetEffect();
    effect->SetViewProjection(view, m_proj);
    effect->BeginFrame(context);

    //effect->SetLightDirection(0, SimpleMath::Vector3(0, -1, -1));
    //effect->SetLightEnabled(1, false);
    //effect->SetLightEnabled(2, false);

    // モデルの描画
    SimpleMath::Matrix world;
    //m_model->Draw(context, world);
    m_model->Draw(context, world, &m_animator->GetWorldMatrices());
    // ------------------------------------------------------- //

    m_sp->Begin();

    //m_sp->Draw(effect->GetTexture(2), SimpleMath::Vector2(0, 0));

    m_sp->End();

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Initialize device dependent objects here (independent of window size).
    
    // コモンステートの作成
    m_states = std::make_unique<CommonStates>(device);

    // グリッドの床の作成
    m_gridFloor = std::make_unique<Imase::GridFloor>(device, context, m_states.get());

    // -------------------------------------------------------------------------------------- //

    // シェーダーの作成
    m_shader = std::make_unique<Imase::BasicShader>(device);
    m_Pshader = std::make_unique<Imase::PixelLightingShader>(device);
    m_Nshader = std::make_unique<Imase::NormalMapShader>(device);

    // エフェクトの作成
    m_effect = std::make_unique<Imase::Effect>(device, m_Nshader.get());

    m_effect->LoadIrradianceTexture(device, L"Resources/Textures/Irradiance.dds");
    m_effect->LoadPrefilterTexture(device, L"Resources/Textures/prefilter.dds");
    m_effect->LoadBrdfTexture(device, L"Resources/Textures/brdf.dds");

    // モデルの作成
    m_model = Imase::Model::CreateFromImdl(device, L"Resources/Models/Dice.imdl", m_effect.get());

    // アニメーターの作成
    m_animator = std::make_unique<Imase::Animator>(*m_model.get());

    m_sp = std::make_unique<SpriteBatch>(context);

    //CreateDDSTextureFromFile(device, L"Textures/horn-koppe_spring_4k.dds", nullptr, m_cubeMap.ReleaseAndGetAddressOf());

    //{
    //    // Texture2D作成
    //    D3D11_TEXTURE2D_DESC desc = {};
    //    desc.Width = 512;
    //    desc.Height = 512;
    //    desc.MipLevels = 1; // <- MipMap生成はしない
    //    desc.ArraySize = 6;
    //    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

    //    desc.SampleDesc.Count = 1;
    //    desc.SampleDesc.Quality = 0;

    //    desc.BindFlags =
    //        D3D11_BIND_RENDER_TARGET |
    //        D3D11_BIND_SHADER_RESOURCE;

    //    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    //    device->CreateTexture2D(
    //        &desc,
    //        nullptr,
    //        m_cubeTex.ReleaseAndGetAddressOf());


    //    // RTV作成
    //    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    //    rtvDesc.Format = desc.Format;
    //    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;

    //    rtvDesc.Texture2DArray.MipSlice = 0;
    //    rtvDesc.Texture2DArray.ArraySize = 6;
    //    rtvDesc.Texture2DArray.FirstArraySlice = 0;

    //    device->CreateRenderTargetView(
    //        m_cubeTex.Get(),
    //        &rtvDesc,
    //        m_cubeRTV.ReleaseAndGetAddressOf());

    //    // SRV作成
    //    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

    //    srvDesc.Format = desc.Format;
    //    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

    //    srvDesc.TextureCube.MipLevels = 1;
    //    srvDesc.TextureCube.MostDetailedMip = 0;

    //    device->CreateShaderResourceView(
    //        m_cubeTex.Get(),
    //        &srvDesc,
    //        m_cubeSRV.ReleaseAndGetAddressOf());
    //}

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    
    // 画面サイズを取得する
    int w, h;
    GetDefaultSize(w, h);

    // プロジェクション行列の作成
    m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f),
        static_cast<float>(w) / static_cast<float>(h),
        0.1f,
        100.0f
    );
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
