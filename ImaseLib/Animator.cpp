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
    , m_timeA{ 0.0f }
    , m_timeB{ 0.0f }
    , m_clipA{ -1 }
    , m_clipB{ -1 }
    , m_blendDuration{ 0.0f }
    , m_blendTimer{ 0.0f }
    , m_blendWeight{ 0.0f }
{
    // ノード数を取得
    size_t nodeCount = model.GetNodes().size();

    // ワークを確保
    m_currentPose.transforms.resize(nodeCount);
    m_tempPoseA.transforms.resize(nodeCount);
    m_tempPoseB.transforms.resize(nodeCount);
    m_localMatrices.resize(nodeCount);
    m_worldMatrices.resize(nodeCount);
}

// 再生
void Imase::Animator::Play(uint32_t clipIndex, bool loop)
{
    m_clipA = clipIndex;
    m_timeA = 0.0f;
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
        const AnimationClip* clipA = m_model.GetAnimation(m_clipA);
        if (!clipA) return;

        ResetPoseToBind(m_currentPose);
        SamplePose(*clipA, m_timeA, m_currentPose);
    }

    // アニメーションブレンド有りの場合
    else if (m_playMode == PlayMode::Blend)
    {
        const AnimationClip* clipA = m_model.GetAnimation(m_clipA);
        const AnimationClip* clipB = m_model.GetAnimation(m_clipB);
        if (!clipA || !clipB) return;

        ResetPoseToBind(m_tempPoseA);
        SamplePose(*clipA, m_timeA, m_tempPoseA);
        ResetPoseToBind(m_tempPoseB);
        SamplePose(*clipB, m_timeB, m_tempPoseB);

        ResetPoseToBind(m_currentPose);
        BlendPose(m_tempPoseA, m_tempPoseB, m_blendWeight, m_currentPose);
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

// クロスフェードする関数
void Imase::Animator::CrossFade(int nextClip, float duration)
{
    m_clipB = nextClip;
    m_timeB = 0.0f;

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
    for (size_t i = 0; i < m_currentPose.transforms.size(); i++)
    {
        Transform& transform = m_currentPose.transforms[i];

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
    m_timeA += elapsedTime;

    const AnimationClip* clipA = m_model.GetAnimation(m_clipA);
    if (!clipA)
    {
        m_timeA = 0.0f;
        return;
    }

    // アニメーションの長さを超えた時の時間の処理
    if (clipA->duration > 0.0f)
    {
        if (m_loop)
        {
            m_timeA = fmod(m_timeA, clipA->duration);
            if (m_timeA < 0.0f)
                m_timeA += clipA->duration;
        }
        else
        {
            m_timeA = std::min(m_timeA, clipA->duration);
        }
    }
    else
    {
        m_timeA = 0.0f;
    }

    // ----- ブレンド ----- //
    if (m_playMode == PlayMode::Blend)
    {
        const AnimationClip* clipB = m_model.GetAnimation(m_clipB);
        if (!clipB)
        {
            m_playMode = PlayMode::Single;
            return;
        }

        m_timeB += elapsedTime;

        if (clipB->duration > 0.0f)
        {
            m_timeB = fmod(m_timeB, clipB->duration);
        }

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

        if (m_blendWeight >= 1.0f)
        {
            // 次のアニメーションへ
            m_clipA = m_clipB;
            m_timeA = m_timeB;
            m_playMode = PlayMode::Single;

            m_clipB = -1;
            m_blendTimer = 0.0f;
            m_blendWeight = 0.0f;
        }
    }
}

