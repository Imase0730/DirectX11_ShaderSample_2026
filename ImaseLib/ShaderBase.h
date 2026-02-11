//--------------------------------------------------------------------------------------
// File: ShaderBase.h
//
// シェーダーの基底クラス
//
// Date: 2025.2.11
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

namespace Imase
{
    enum class ShaderStage : uint32_t
    {
        None = 0,
        VS = 1 << 0, 
        PS = 1 << 1,
        GS = 1 << 2,
        HS = 1 << 3,
        DS = 1 << 4,
        CS = 1 << 5,

        All = VS | PS | GS | HS | DS | CS
    };

    struct UserConstantBuffer
    {
        ID3D11Buffer* buffer;   // 定数バッファ
        UINT slot;              // スロット
        ShaderStage stage;      // 使用ステージ
    };

    class ShaderBase
    {
    public:

        virtual ~ShaderBase() = default;

        // シェーダー・入力レイアウトをバインド
        virtual void Bind(ID3D11DeviceContext* context) = 0;

        // UserCBを持つか？
        virtual bool HasUserCB() const { return false; }

        // UserCBのバッファを取得
        virtual void GetUserCBs(std::vector<UserConstantBuffer>& out) const
        {
            out.clear();
        }

        // UserCBの更新
        virtual void UpdateUserCBs(ID3D11DeviceContext* context)
        {
            UNREFERENCED_PARAMETER(context);
        }
    };
}
