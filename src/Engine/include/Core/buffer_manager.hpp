#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <Core/export.hpp>
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


        template<typename Type>
        static bool basic_element_write(BufferWriter& writer, const Type& value)
        {
            using NoPointerType = std::remove_pointer_t<Type>;

            if constexpr (std::is_base_of_v<NoPointerType, SerializableObject>)
            {
                if constexpr (std::is_pointer_v<Type>)
                {
                    return value->serialize(&writer);
                }
                else
                {
                    return value.serialize(&writer);
                }
            }
            else
                return writer.write(reinterpret_cast<const byte*>(&value), sizeof(value));
        }

        template<typename Type>
        typename std::enable_if<!Engine::is_container<Type>::value, bool>::type
        write(const Type& data, bool (*write_func)(BufferWriter& writer, const Type&) = basic_element_write<Type>)
        {
            return write_func(*this, data);
        }

        template<typename Container>
        typename std::enable_if<Engine::is_container<Container>::value, bool>::type
        write(const Container& container,
              bool (*write_func)(BufferWriter& writer, const typename Container::value_type&) =
                      basic_element_write<typename Container::value_type>)
        {
            size_t size = container.size();
            if (!write(size))
            {
                return false;
            }

            for (const typename Container::value_type& element : container)
            {
                if (!write_func(*this, element))
                    return false;
            }
            return true;
        }
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


        template<typename Type>
        static bool basic_element_read(BufferReader& reader, Type& value)
        {
            using NoPointerType = std::remove_pointer_t<Type>;

            if constexpr (std::is_base_of_v<NoPointerType, SerializableObject>)
            {
                if constexpr (std::is_pointer_v<Type>)
                {
                    if (value == nullptr)
                    {
                        if constexpr (std::is_base_of_v<Object, NoPointerType>)
                        {
                            value = Object::new_instance<Type>();
                        }
                        else
                        {
                            return false;
                        }
                    }

                    return value->deserialize(&reader);
                }
                else
                {
                    return value.deserialize(&reader);
                }
            }
            else
                return reader.read(reinterpret_cast<byte*>(&value), sizeof(value));
        }

        template<typename Type>
        typename std::enable_if<!Engine::is_container<Type>::value, bool>::type
        read(Type& data, bool (*read_func)(BufferReader&, Type&) = basic_element_read<Type>)
        {
            return read_func(*this, data);
        }

        template<typename Container>
        typename std::enable_if<Engine::is_container<Container>::value, bool>::type
        read(Container& container,
             bool (*read_func)(BufferReader& reader,
                               typename Container::value_type&) = basic_element_read<typename Container::value_type>)
        {
            size_t size;
            if (!read(size))
            {
                return false;
            }

            container.clear();
            container.resize(size);

            for (typename Container::value_type& element : container)
            {
                if (!read_func(*this, element))
                {
                    return false;
                }
            }

            return true;
        }

        template<typename Type>
        typename std::enable_if<!Engine::is_container<Type>::value, Type>::type
        read_value(bool (*read_func)(BufferReader&, Type&) = basic_element_read<Type>)
        {
            Type result;
            if (!read_func(*this, result))
            {
                throw EngineException("Failed to read object!");
            }
            return result;
        }


        template<typename Container>
        typename std::enable_if<Engine::is_container<Container>::value, Container>::type read_value(
                bool (*read_func)(BufferReader&,
                                  typename Container::value_type&) = basic_element_read<typename Container::value_type>)
        {
            size_t size;
            if (!read(size))
            {
                throw EngineException("Failed to read object!");
            }

            Container result;
            result.resize(size);

            for (typename Container::value_type& element : result)
            {
                if (!read_func(*this, element))
                {
                    throw EngineException("Failed to read object!");
                }
            }

            return result;
        }


        template<typename Container>
        typename std::enable_if<Engine::is_container<Container>::value, bool>::type
        read(Container& container, size_t size,
             bool (*read_func)(BufferReader&,
                               typename Container::value_type&) = basic_element_read<typename Container::value_type>)
        {
            if (!is_open())
                return false;

            container.clear();
            container.resize(size);

            for (typename Container::value_type& element : container)
            {
                if (!read_func(*this, element))
                {
                    return false;
                }
            }
            return true;
        }
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
