//--------------------------------------------------------------------------------------
// File: BinaryReader.h
//
// データから指定サイズのデータを取得するクラス
//
// ※内部でデータを読み取ると読み取り位置を移動しています
//
// Date: 2026.2.16
// Author: Hideyasu Imase
//--------------------------------------------------------------------------------------
#pragma once

#include <vector>

namespace Imase
{
    class BinaryReader
    {
    public:

        // コンストラクタ
        BinaryReader(const std::vector<uint8_t>& data)
            : m_buffer(data)
            , m_offset(0)
        {
        }

        // 【uint32_t】を読み取る関数
        uint32_t ReadUInt32()
        {
            uint32_t v;
            ReadRaw(&v, sizeof(v));
            return v;
        }

        // 【int32_t】を読み取る関数
        int32_t ReadInt32()
        {
            int32_t v;
            ReadRaw(&v, sizeof(v));
            return v;
        }

        // 【float】を読み取る関数
        float ReadFloat()
        {
            float v;
            ReadRaw(&v, sizeof(v));
            return v;
        }

        // 【1byte】を読み取る関数
        void ReadBytes(void* dst, size_t size)
        {
            ReadRaw(dst, size);
        }

        // 【T】を指定個数を読み取る関数
        // 【uint32_t】(count) + 【T】 * count
        template<typename T>
        std::vector<T> ReadVector()
        {
            uint32_t count = ReadUInt32();
            std::vector<T> vec(count);
            if (count > 0)
            {
                ReadRaw(vec.data(), sizeof(T) * count);
            }
            return vec;
        }

        // データが終端まで読み終わったらtrueを返す関数
        bool End() const
        {
            return m_offset >= m_buffer.size();
        }

    private:

        // 読み取り用バッファ
        const std::vector<uint8_t>& m_buffer;

        // 現在の読み取り位置
        size_t m_offset;

        // 指定サイズのデータをバッファから読み取る関数
        void ReadRaw(void* dst, size_t size)
        {
            if (m_offset + size > m_buffer.size())
            {
                throw std::runtime_error("BinaryReader overflow");
            }

            std::memcpy(dst, m_buffer.data() + m_offset, size);
            m_offset += size;
        }
    };
}

