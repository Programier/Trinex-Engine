#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <Core/object.hpp>
#include <istream>
#include <ostream>

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


        bool write(const byte* data, size_t size);
        size_t size();
        WritePos position();
        BufferWriter& position(WritePos pos);
        BufferWriter& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current);
        operator bool();

        virtual Stream& stream()     = 0;
        virtual bool is_open() const = 0;
        virtual BufferWriter& clear();
        virtual ~BufferWriter()
        {}
    };


    class ENGINE_EXPORT BufferReader
    {
    public:
        using ReadPos   = size_t;
        using PosOffset = int64_t;
        using Stream    = std::istream;

        bool read(byte* data, size_t size);
        size_t size();
        ReadPos position();
        BufferReader& position(ReadPos pos);
        BufferReader& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current);
        operator bool();

        virtual Stream& stream()     = 0;
        virtual bool is_open() const = 0;
        virtual ~BufferReader()
        {}
    };

    // Text buffers
    class ENGINE_EXPORT TextBufferWriter : public BufferWriter
    {
        template<typename Type>
        friend TextBufferWriter& operator<<(TextBufferWriter& writer, const Type& value)
        {
            writer.stream() << value;
            return writer;
        }
    };

    class ENGINE_EXPORT TextBufferReader : public BufferReader
    {
    public:
        template<typename Type>
        friend TextBufferReader& operator>>(TextBufferReader& reader, Type& value)
        {
            reader.stream() >> value;
            return reader;
        }

        template<typename Type>
        Type parse_value()
        {
            Type result;
            stream() >> result;
            return result;
        }

        bool read_line(String& line, char delimiter = '\n');
        String read_line(char delimiter = '\n');
    };

    template<typename BufferClass>
    class BufferManagmentWrapper : public BufferClass
    {
    private:
        typename BufferClass::Stream& _M_stream;

    public:
        BufferManagmentWrapper(typename BufferClass::Stream& stream) : _M_stream(stream)
        {}

        typename BufferClass::Stream& stream() override
        {
            return _M_stream;
        }

        bool is_open() const override
        {
            return true;
        }
    };

    template<typename BufferClass>
    using BufferReaderWrapper = BufferManagmentWrapper<BufferClass>;

    template<typename BufferClass>
    using BufferWriterWrapper = BufferManagmentWrapper<BufferClass>;
}// namespace Engine
