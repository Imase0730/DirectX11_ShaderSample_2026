//--------------------------------------------------------------------------------------
// File: NormalMapShader.h
//
// 法線マップシェーダー
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "ShaderBase.h"

namespace Imase
{
	class NormalMapShader : public ShaderBase
	{
    public:

        // コンストラクタ
        NormalMapShader(ID3D11Device* device)
            : ShaderBase(device, L"Resources/Shaders/NormalMapVS.cso", L"Resources/Shaders/NormalMapPS.cso")
        {
        }

    };
}

