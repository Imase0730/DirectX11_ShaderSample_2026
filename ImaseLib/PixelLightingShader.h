//--------------------------------------------------------------------------------------
// File: PixelLightingShader.h
//
// ピクセルライティングシェーダー
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "ShaderBase.h"

namespace Imase
{
	class PixelLightingShader : public ShaderBase
	{
    public:

        // コンストラクタ
        PixelLightingShader(ID3D11Device* device)
            : ShaderBase(device, L"Resources/Shaders/PixelLightingVS.cso", L"Resources/Shaders/PixelLightingPS.cso")
        {
        }

    };
}

