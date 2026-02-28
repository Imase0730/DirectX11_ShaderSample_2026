#pragma once

#include "Model.h"

namespace Imase
{

	class Animator
	{
    public:

        enum class PlayMode
        {
            Single,     // 単一再生
            Blend       // ブレンド再生
        };

    private:

        // 移動、回転、スケール情報をまとめた構造体
        struct Transform
        {
            DirectX::XMFLOAT3 translation;
            DirectX::XMFLOAT4 rotation;
            DirectX::XMFLOAT3 scale;
        };

        // モデルのポーズ情報構造体
        struct Pose
        {
            std::vector<Transform> transforms;
        };

        // モデルへの参照
        const Imase::Model& m_model;

        // ノード情報への参照
        const std::vector<NodeInfo>& m_nodes;
        
        // 再生モード
        PlayMode m_playMode;

        // ループ（ON/OFF)
        bool m_loop;

        // 最終的な姿勢
        Pose m_currentPose;

        // 姿勢A
        Pose m_tempPoseA;

        // 姿勢B
        Pose m_tempPoseB;

        // 再生アニメーションクリップインデックス
        int m_clipA;

        // ブレンド用アニメーションクリップインデックス
        int m_clipB;

        // 再生アニメーションクリップの時間
        float m_timeA;

        // ブレンド用アニメーションクリップの時間
        float m_timeB;

        // ブレンド間隔
        float m_blendDuration;

        // ブレンド用タイマー
        float m_blendTimer;

        // ブレンド用ウエイト
        float m_blendWeight;
 
        // 各ノードのローカル行列
        std::vector<DirectX::XMFLOAT4X4> m_localMatrices;

        // 各ノードのワールド行列
        std::vector<DirectX::XMFLOAT4X4> m_worldMatrices;

    private:

        // アニメーションチャンネルから指定時間の値を取得する関数（線形補間：移動、スケール用3)
        DirectX::XMFLOAT3 SampleVec3(const Imase::AnimationChannelVec3& ch, float time);

        // アニメーションチャンネルから指定時間の値を取得する関数（球面線形補間：回転用)
        DirectX::XMFLOAT4 SampleQuat(const Imase::AnimationChannelQuat& ch, float time);
        
        // 初期ポーズへリセットする関数
        void ResetPoseToBind(Pose& pose);

        // 各ノードの移動、回転、スケールを計算する関数
        void SamplePose(const AnimationClip& clip, float time, Pose& outPose);

        // 各ノードのローカル行列を設定する関数
        void BuildLocalMatrices();

        // 各ノードのワールド行列を設定する関数
        void BuildWorldMatrices();

        // ポーズのブレンド関数
        void BlendPose(const Pose& a, const Pose& b, float weight, Pose& outPose);

        // 再生時間を進める関数
        void UpdateTime(float elapsedTime);

    public:

        // コンストラクタ
        Animator(const Imase::Model& model);

        // 再生
        void Play(uint32_t clipIndex, bool loop = true);

        // 更新
        void Update(float elapsedTime);

        // 次のアニメーションへのクロスフェードする関数
        void CrossFade(int nextClip, float duration);

        // 各ノードのワールド行列を取得する関数
        const std::vector<DirectX::XMFLOAT4X4>& GetWorldMatrices() const;

    };

}
