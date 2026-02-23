//--------------------------------------------------------------------------------------
// File: BasicShader.h
//
// ベーシックシェーダー
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include "ShaderBase.h"

namespace Imase
{
	class BasicShader : public ShaderBase
	{
    public:

        // コンストラクタ
        BasicShader(ID3D11Device* device)
            : ShaderBase(device, L"Resources/Shaders/BasicVS.cso", L"Resources/Shaders/BasicPS.cso")
        {
        }

    };
}

