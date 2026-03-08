//--------------------------------------------------------------------------------------
// File: Animator.cpp
//
// アニメーションの再生を管理するクラス
//
// Date: 2026.3.3
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Animator.h"
#include "Imdl.h"

using namespace DirectX;

// コンストラクタ
Imase::Animator::Animator(const Imase::Model& model)
	: m_model{ model }
    , m_nodes{ model.GetNodes() }
	, m_loop{ true }
    , m_playMode{ PlayMode::Single }
    , m_currentPoseState{ -1, 0.0f, std::vector<Transform>(model.GetNodes().size()) }
    , m_nextPoseState{ -1, 0.0f, std::vector<Transform>(model.GetNodes().size()) }
    , m_blendDuration{ 0.0f }
    , m_blendTimer{ 0.0f }
    , m_blendWeight{ 0.0f }
    , m_localMatrices{ std::vector<DirectX::XMFLOAT4X4>(model.GetNodes().size()) }
    , m_worldMatrices{ std::vector<DirectX::XMFLOAT4X4>(model.GetNodes().size()) }
{
    // ローカル行列を初期化
    for (auto& m : m_localMatrices)
    {
        m = SimpleMath::Matrix::Identity;
    }

    // ワールド行列を初期化
    for (auto& m : m_worldMatrices)
    {
        m = SimpleMath::Matrix::Identity;
    }

    // アニメションクリップ名を登録
    int i = 0;
    while (const Imase::AnimationClip* anim = m_model.GetAnimation(i))
    {
        m_animationIndexTable[anim->name] = i;
        m_animationNameTable[i] = anim->name;
        i++;
    }
}

// アニメションインデックスを取得する関数
int Imase::Animator::GetAnimationIndex(std::string animationName)
{

    return m_animationIndexTable.at(animationName);
}

// アニメーション名を取得する関数
std::string Imase::Animator::GetAnimationName(int animationIndex)
{
    return m_animationNameTable.at(animationIndex);
}

// 再生
void Imase::Animator::Play(std::string animationName, bool loop)
{
    Play(GetAnimationIndex(animationName), loop);
}

void Imase::Animator::Play(int animationIndex, bool loop)
{
    m_currentPoseState.m_clipIndex = animationIndex;
    m_currentPoseState.m_time = 0.0f;
    m_playMode = PlayMode::Single;
    m_loop = loop;
}

// 更新
void Imase::Animator::Update(float elapsedTime)
{
    // 再生時間を更新
    UpdateTime(elapsedTime);

    // 通常の再生
    if (m_playMode == PlayMode::Single)
    {
        const AnimationClip* clip = m_model.GetAnimation(m_currentPoseState.m_clipIndex);
        if (!clip) return;

        // 現在の時間のポーズを取得
        SamplePose(*clip, m_currentPoseState.m_time, m_currentPoseState.m_pose);
    }

    // アニメーションブレンド有りの場合
    else if (m_playMode == PlayMode::Blend)
    {
        const AnimationClip* clipA = m_model.GetAnimation(m_currentPoseState.m_clipIndex);
        const AnimationClip* clipB = m_model.GetAnimation(m_nextPoseState.m_clipIndex);
        if (!clipA || !clipB) return;

        // ブレンド元とブレンド先のポーズを取得
        SamplePose(*clipA, m_currentPoseState.m_time, m_currentPoseState.m_pose);
        SamplePose(*clipB, m_nextPoseState.m_time, m_nextPoseState.m_pose);

        // アニメーションブレンド
        Pose resultPose;
        BlendPose(m_currentPoseState.m_pose, m_nextPoseState.m_pose, m_blendWeight, resultPose);
        m_currentPoseState.m_pose = resultPose;
    }

    // 各ノードのローカル行列を生成する
    BuildLocalMatrices();

    // 親子合成をして各ノードのワールド行列を生成する
    BuildWorldMatrices();
}

// 各ノードのワールド行列を取得する関数
const std::vector<DirectX::XMFLOAT4X4>& Imase::Animator::GetWorldMatrices() const
{
    return m_worldMatrices;
}

// アニメーション名をアニメーションインデックス順で取得する関数
const std::vector<std::string> Imase::Animator::GetAnimationNames() const
{
    // mapの要素をpairのvectorにコピー
    std::vector<std::pair<std::string, int>> vec(m_animationIndexTable.begin(), m_animationIndexTable.end());

    // アニメーションインデックスの昇順でソート
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b)
        {
            return a.second < b.second; // 昇順
        }
    );

    // キー(アニメーション名)だけを抽出
    std::vector<std::string> keys;
    keys.reserve(vec.size());
    for (const auto& pair : vec)
    {
        keys.push_back(pair.first);
    }

    return keys;
}

// 再生時間を取得する関数
float Imase::Animator::GetPlayTime() const
{
    return m_currentPoseState.m_time;
}

// アニメーションの長さ（所要時間）を取得する関数
float Imase::Animator::GetAnimationDuration(int animationIndex) const
{
    const AnimationClip* clip = m_model.GetAnimation(animationIndex);
    return clip->duration;
}

float Imase::Animator::GetRestTime() const
{
    float rest = GetAnimationDuration(m_currentPoseState.m_clipIndex) - GetPlayTime();

    if (rest < 0.0f)
        rest = 0.0f;

    return rest;
}

int Imase::Animator::GetCurrentAnimationIndex()
{
    return m_currentPoseState.m_clipIndex;
}

// クロスフェードする関数
void Imase::Animator::CrossFade(std::string nextAnimationName, float duration)
{
    CrossFade(GetAnimationIndex(nextAnimationName), duration);
}

void Imase::Animator::CrossFade(int animationIndex, float duration)
{
    m_nextPoseState.m_clipIndex = animationIndex;
    m_nextPoseState.m_time = 0.0f;

    m_blendDuration = duration;
    m_blendTimer = 0.0f;

    m_playMode = PlayMode::Blend;
}

// 指定時間の値を取得する関数（x,y,z）
DirectX::XMFLOAT3 Imase::Animator::SampleVec3(const Imase::AnimationChannelVec3& ch, float time)
{
    if (ch.times.empty() || ch.values.empty())
    {
        return XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    // 指定時間の値を返す（線形補間）
    for (size_t i = 0; i < ch.times.size() - 1; i++)
    {
        if ((time >= ch.times[i]) && (time <= ch.times[i + 1]))
        {
            float t = (time - ch.times[i]) / (ch.times[i + 1] - ch.times[i]);

            XMVECTOR a = XMLoadFloat3(&ch.values[i]);
            XMVECTOR b = XMLoadFloat3(&ch.values[i + 1]);

            XMVECTOR result = XMVectorLerp(a, b, t);

            XMFLOAT3 out;
            XMStoreFloat3(&out, result);
            return out;
        }
    }

    if (time <= ch.times.front())
        return ch.values.front();

    if (time >= ch.times.back())
        return ch.values.back();

    return ch.values.back();
}

// 指定時間の値を取得する関数（x,y,z,w）
DirectX::XMFLOAT4 Imase::Animator::SampleQuat(const Imase::AnimationChannelQuat& ch, float time)
{
    if (ch.times.empty() || ch.values.empty())
    {
        // 単位クォータニオンを返す
        return XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // 指定時間の値を返す（球面線形補間）
    for (size_t i = 0; i < ch.times.size() - 1; i++)
    {
        if (time >= ch.times[i] && time <= ch.times[i + 1])
        {
            float t = (time - ch.times[i]) / (ch.times[i + 1] - ch.times[i]);

            XMVECTOR a = XMLoadFloat4(&ch.values[i]);
            XMVECTOR b = XMLoadFloat4(&ch.values[i + 1]);

            XMVECTOR result = XMQuaternionSlerp(a, b, t);

            XMFLOAT4 out;
            XMStoreFloat4(&out, result);
            return out;
        }
    }

    if (time <= ch.times.front())
        return ch.values.front();

    if (time >= ch.times.back())
        return ch.values.back();

    return ch.values.back();
}

// 初期ポーズへリセットする関数
void Imase::Animator::ResetPoseToBind(Pose& pose)
{
    const auto& nodes = m_model.GetNodes();

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        pose.transforms[i].translation = nodes[i].defaultTranslation;
        pose.transforms[i].rotation = nodes[i].defaultRotation;
        pose.transforms[i].scale = nodes[i].defaultScale;
    }
}

// 再生時間のポーズを取得する関数
void Imase::Animator::SamplePose(const AnimationClip& clip, float time, Pose& outPose)
{
    // ポーズをリセット
    ResetPoseToBind(outPose);

    // 移動
    for (const auto& ch : clip.translations)
    {
        outPose.transforms[ch.nodeIndex].translation = SampleVec3(ch, time);
    }

    // 回転
    for (const auto& ch : clip.rotations)
    {
        outPose.transforms[ch.nodeIndex].rotation = SampleQuat(ch, time);
    }

    // スケール
    for (const auto& ch : clip.scales)
    {
        outPose.transforms[ch.nodeIndex].scale = SampleVec3(ch, time);
    }
}

// 各ノードのローカル行列を生成する関数
void Imase::Animator::BuildLocalMatrices()
{
    for (size_t i = 0; i < m_currentPoseState.m_pose.transforms.size(); i++)
    {
        Transform& transform = m_currentPoseState.m_pose.transforms[i];

        XMVECTOR t = XMLoadFloat3(&transform.translation);
        XMVECTOR r = XMLoadFloat4(&transform.rotation);
        XMVECTOR s = XMLoadFloat3(&transform.scale);

        XMMATRIX M = XMMatrixScalingFromVector(s) * XMMatrixRotationQuaternion(r) *  XMMatrixTranslationFromVector(t);

        XMStoreFloat4x4(&m_localMatrices[i], M);
    }
}

// 各ノードのワールド行列を計算する関数
void Imase::Animator::BuildWorldMatrices()
{
    for (size_t i = 0; i < m_nodes.size(); i++)
    {
        int parent = m_nodes[i].parentIndex;

        XMMATRIX local = XMLoadFloat4x4(&m_localMatrices[i]);

        if (parent >= 0)
        {
            XMMATRIX parentWorld = XMLoadFloat4x4(&m_worldMatrices[parent]);
            local = local * parentWorld;
        }

        XMStoreFloat4x4(&m_worldMatrices[i], local);
    }
}

// ポーズのブレンド関数
void Imase::Animator::BlendPose(const Pose& a, const Pose& b, float weight, Pose& outPose)
{
    size_t count = a.transforms.size();

    outPose.transforms.resize(count);

    for (size_t i = 0; i < count; ++i)
    {
        // Translation
        XMVECTOR tA = XMLoadFloat3(&a.transforms[i].translation);
        XMVECTOR tB = XMLoadFloat3(&b.transforms[i].translation);
        XMVECTOR t = XMVectorLerp(tA, tB, weight);
        XMStoreFloat3(&outPose.transforms[i].translation, t);

        // Scale
        XMVECTOR sA = XMLoadFloat3(&a.transforms[i].scale);
        XMVECTOR sB = XMLoadFloat3(&b.transforms[i].scale);
        XMVECTOR s = XMVectorLerp(sA, sB, weight);
        XMStoreFloat3(&outPose.transforms[i].scale, s);

        // Rotation
        XMVECTOR rA = XMLoadFloat4(&a.transforms[i].rotation);
        XMVECTOR rB = XMLoadFloat4(&b.transforms[i].rotation);

        // クォータニオンの補間の最短距離の選択
        if (XMVectorGetX(XMVector4Dot(rA, rB)) < 0.0f)
        {
            // 内積が０以下なので片方のクォータニオンを反転
            rB = XMVectorNegate(rB);
        }

        // 球面線形補間
        XMVECTOR r = XMQuaternionSlerp(rA, rB, weight);
        XMStoreFloat4(&outPose.transforms[i].rotation, r);
    }
}

// 再生時間を進める関数
void Imase::Animator::UpdateTime(float elapsedTime)
{
    m_currentPoseState.m_time += elapsedTime;

    const AnimationClip* clipA = m_model.GetAnimation(m_currentPoseState.m_clipIndex);
    if (!clipA)
    {
        m_currentPoseState.m_time = 0.0f;
        return;
    }

    // アニメーションの長さを超えた時の時間の処理
    if (clipA->duration > 0.0f)
    {
        if (m_loop)
        {
            m_currentPoseState.m_time = fmod(m_currentPoseState.m_time, clipA->duration);
            if (m_currentPoseState.m_time < 0.0f)
                m_currentPoseState.m_time += clipA->duration;
        }
        else
        {
            m_currentPoseState.m_time = std::min(m_currentPoseState.m_time, clipA->duration);
        }
    }
    else
    {
        m_currentPoseState.m_time = 0.0f;
    }

    // ----- ブレンド ----- //
    if (m_playMode == PlayMode::Blend)
    {
        const AnimationClip* clipB = m_model.GetAnimation(m_nextPoseState.m_clipIndex);
        if (!clipB)
        {
            m_playMode = PlayMode::Single;
            return;
        }

        // 時間を進める
        m_nextPoseState.m_time += elapsedTime;

        if (clipB->duration > 0.0f)
        {
            m_nextPoseState.m_time = fmod(m_nextPoseState.m_time, clipB->duration);
        }

        // ブレンドの割合を算出
        m_blendTimer += elapsedTime;

        if (m_blendDuration <= 0.0f)
        {
            m_blendWeight = 1.0f;
        }
        else
        {
            m_blendWeight = m_blendTimer / m_blendDuration;
            m_blendWeight = std::clamp(m_blendWeight, 0.0f, 1.0f);
        }

        // 次のアニメーションへ移行完了
        if (m_blendWeight >= 1.0f)
        {
            // 次のアニメーションへ
            m_currentPoseState.m_clipIndex = m_nextPoseState.m_clipIndex;
            m_currentPoseState.m_time = m_nextPoseState.m_time;
            m_playMode = PlayMode::Single;

            m_nextPoseState.m_clipIndex = -1;
            m_blendTimer = 0.0f;
            m_blendWeight = 0.0f;
        }
    }
}


