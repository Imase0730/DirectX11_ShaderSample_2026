//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

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

    // -------------------------------------------------------------------------------------- //

    SimpleMath::Matrix world;

    //world = SimpleMath::Matrix::CreateTranslation(0, -1, 0)
    //    * SimpleMath::Matrix::CreateRotationX(XMConvertToRadians(90))
    //    * SimpleMath::Matrix::CreateTranslation(0, 1, 0);
    //world *= SimpleMath::Matrix::CreateRotationY(static_cast<float>(m_timer.GetTotalSeconds() * 0.5f));

    //world = SimpleMath::Matrix::Identity;

    //world = SimpleMath::Matrix::CreateRotationY(XMConvertToRadians(45.0f));
    SimpleMath::Matrix rotY = SimpleMath::Matrix::CreateRotationY(m_timer.GetTotalSeconds());
    m_lightDirection = SimpleMath::Vector3::Transform(SimpleMath::Vector3(0,-1,-1), rotY);

    //m_model->UpdateEffect([&](Imase::Effect* effect)
    //    {
    //        effect->SetLightDirection(m_lightDirection);
    //    }
    //);
   //effect->SetAmbientLightColor(Colors::Black);

    //m_effect->SetLightDirection(0, SimpleMath::Vector3(0, -1, -1));
    //effect->SetLightDirection(0, m_lightDirection);
    ////effect->SetLightDiffuseColor(0, Colors::White);
    //m_effect->SetLightEnabled(0, false);
    //m_effect->SetLightEnabled(1, false);
    //m_effect->SetLightEnabled(2, false);
    //m_effect->SetLightDiffuseColor(2, Colors::Black);

    //m_model->SetDiffuseColorByName(L"Material", XMFLOAT3(1,1,0));

    // ------------------------------------------------------- //
    // モデル描画 
    // ------------------------------------------------------- //

    Imase::Effect* effect{};

    // ------------------------------------------------------- //

    // （フレームで頻繁に更新しない定数バッファを更新している）
    // ビュー行列とプロジェクション行列を設定
    effect = m_model_Basic->GetEffect();
    effect->SetViewProjection(view, m_proj);
    effect->BeginFrame(context);

    // モデルの描画
    world = SimpleMath::Matrix::CreateRotationY(static_cast<float>(m_timer.GetTotalSeconds() * 0.5f));
    //world *= SimpleMath::Matrix::CreateScale(2.0f);
    world *= SimpleMath::Matrix::CreateTranslation(-2.0f, 0.0f, 0.0f);
    m_model_Basic->Draw(context, world);

    // ------------------------------------------------------- //

    // （フレームで頻繁に更新しない定数バッファを更新している）
    // ビュー行列とプロジェクション行列を設定
    effect = m_model_NormalMap->GetEffect();
    effect->SetViewProjection(view, m_proj);
    effect->BeginFrame(context);

    // モデルの描画
    world = SimpleMath::Matrix::CreateRotationY(static_cast<float>(m_timer.GetTotalSeconds() * 0.5f));
    //world *= SimpleMath::Matrix::CreateScale(2.0f);
    world *= SimpleMath::Matrix::CreateTranslation(2.0f, 0.0f, 0.0f);
    //m_model_NormalMap->Draw(context, world);
    
    // ------------------------------------------------------- //
    effect = m_model_PixelLighting->GetEffect();
    effect->SetViewProjection(view, m_proj);
    effect->BeginFrame(context);

    // モデルの描画
    world = SimpleMath::Matrix::CreateRotationY(static_cast<float>(m_timer.GetTotalSeconds() * 0.5f));
    //world *= SimpleMath::Matrix::CreateScale(2.0f);
    world *= SimpleMath::Matrix::CreateTranslation(2.0f, 0.0f, 0.0f);
    m_model_PixelLighting->Draw(context, world);





    //effect = m_model_Basic->GetEffect();


    //m_sp->Begin();

    //m_sp->Draw(effect->GetTexture(1), SimpleMath::Vector2(0, 0));

    //m_sp->End();

    // ------------------------------------------------------- //

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
    m_shader_Basic = std::make_unique<Imase::BasicShader>(device);
    m_shader_NormalMap = std::make_unique<Imase::NormalMapShader>(device);
    m_shader_PixelLighting = std::make_unique<Imase::PixelLightingShader>(device);

    // エフェクトの作成
    m_effect_Basic = std::make_unique<Imase::Effect>(device, m_shader_Basic.get());
    m_effect_NormalMap = std::make_unique<Imase::Effect>(device, m_shader_NormalMap.get());
    m_effect_PixelLighting = std::make_unique<Imase::Effect>(device, m_shader_PixelLighting.get());

    // モデルの作成
    m_model_Basic = Imase::Model::CreateFromImdl(device, L"Resources/Models/Cube.imdl", m_effect_Basic.get());
    m_model_NormalMap = Imase::Model::CreateFromImdl(device, L"Resources/Models/Cube.imdl", m_effect_NormalMap.get());
    m_model_PixelLighting = Imase::Model::CreateFromImdl(device, L"Resources/Models/Cube.imdl", m_effect_PixelLighting.get());

    m_sp = std::make_unique<SpriteBatch>(context);
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
