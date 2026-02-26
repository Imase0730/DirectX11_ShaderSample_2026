#pragma once

#include "Model.h"

namespace Imase
{

	class Animator
	{
        // モデルへのポインタ
        const Imase::Model* m_pModel;

        // 再生アニメーションクリップ
        int m_currentClip = -1;

        // 再生時間
        float m_currentTime = 0.0f;

        // ループ（ON/OFF)
        bool m_loop = true;

    public:

        // 更新関数
        void Update(float deltaTime);
    };

}
