//--------------------------------------------------------------------------------------
// File: Animator.h
//
// アニメーションの再生を管理するクラス
//
// Date: 2026.3.3
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
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

        // アニメーションステート
        struct AnimationState
        {
            // アニメーションクリップインデックス
            int m_clipIndex = -1;

            // 再生アニメーションクリップの時間
            float m_time = 0.0f;

            // 姿勢
            Pose m_pose;
        };

        // モデルへの参照
        const Imase::Model& m_model;

        // ノード情報への参照
        const std::vector<NodeInfo>& m_nodes;
        
        // アニメションクリップ名とインデックスの対応表（名前→インデックス）
        std::unordered_map<std::string, int> m_animationIndexTable;

        // アニメションクリップ名とインデックスの対応表（インデックス→名前）
        std::unordered_map<int, std::string> m_animationNameTable;

        // 再生モード
        PlayMode m_playMode;

        // ループ（ON/OFF)
        bool m_loop;

        // 現在の姿勢
        AnimationState m_currentPoseState;

        // 次の姿勢
        AnimationState m_nextPoseState;

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

        // アニメーションチャンネルから指定時間の値を取得する関数（線形補間：移動、スケール用)
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

        // デストラクタ
        virtual ~Animator() = default;

        // 更新
        void Update(float elapsedTime);

        // 各ノードのワールド行列を取得する関数
        const std::vector<DirectX::XMFLOAT4X4>& GetWorldMatrices() const;

        // アニメーション名をアニメーションインデックス順で取得する関数
        const std::vector<std::string> GetAnimationNames() const;

        // ------------------------------------------------------------------- //
        
        // 再生
        void Play(std::string animationName, bool loop = true);
        void Play(int animationIndex, bool loop = true);

        // 次のアニメーションへのクロスフェードする関数
        void CrossFade(std::string nextAnimationName, float duration);
        void CrossFade(int animationIndex, float duration);

        // ------------------------------------------------------------------- //

        // アニメションインデックスを取得する関数
        int GetAnimationIndex(std::string animationName);

        // アニメーション名を取得する関数
        std::string GetAnimationName(int animationIndex);

        // アニメーションの長さ（所要時間）を取得する関数
        float GetAnimationDuration(int animationIndex) const;

        // ------------------------------------------------------------------- //

        // 再生時間を取得する関数
        float GetPlayTime() const;

        // アニメーションの終了するまでの残り時間を取得する関数
        float GetRestTime() const;

        // 再生中のアニメションインデックスを取得する関数
        int GetCurrentAnimationIndex();

    };

}
