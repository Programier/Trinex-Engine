#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class BufferSeekDir : EnumerateType
    {
        Current = 0,
        Begin   = 1,
        End     = 2,
    };


    class ENGINE_EXPORT BufferWriter
    {
    public:
        using Stream    = std::ostream;
        using WritePos  = size_t;
        using PosOffset = int64_t;


        size_t size();

        virtual bool write(const byte* data, size_t size)                                          = 0;
        virtual WritePos position()                                                                = 0;
        virtual BufferWriter& position(WritePos pos)                                               = 0;
        virtual BufferWriter& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) = 0;
        virtual bool is_open() const                                                               = 0;
        virtual BufferWriter& clear()                                                              = 0;

        virtual ~BufferWriter()
        {}
    };


    class ENGINE_EXPORT BufferReader
    {
    public:
        using ReadPos   = size_t;
        using PosOffset = int64_t;
        using Stream    = std::istream;

        size_t size();
        virtual bool read(byte* data, size_t size)                                                 = 0;
        virtual ReadPos position()                                                                 = 0;
        virtual BufferReader& position(ReadPos pos)                                                = 0;
        virtual BufferReader& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) = 0;
        virtual bool is_open() const                                                               = 0;

        virtual ~BufferReader()
        {}
    };


}// namespace Engine
