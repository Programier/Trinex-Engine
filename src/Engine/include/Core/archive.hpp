#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/serializable_object.hpp>


namespace Engine
{
    class BufferReader;
    class BufferWriter;

    class Archive
    {
    private:
        union
        {
            BufferReader* _M_reader;
            BufferWriter* _M_writer;
        };

        bool _M_is_saving      = false;
        bool _M_process_status = true;


        template<typename Type>
        FORCE_INLINE void process_serializable_object(Type& data)
        {
            if constexpr (std::is_pointer_v<Type>)
            {
                data->archive_process(this);
            }
            else
            {
                data.archive_process(this);
            }
        }


    public:
        Archive(BufferReader* reader);
        Archive(BufferWriter* writer);
        Archive(const Archive&)            = delete;
        Archive(Archive&&)                 = delete;
        Archive& operator=(const Archive&) = delete;
        Archive& operator=(Archive&&)      = delete;

        bool is_saving() const;
        bool is_reading() const;

        BufferReader* reader() const;
        BufferWriter* writer() const;

        Archive& write_data(const byte* data, size_t size);
        Archive& read_data(byte* data, size_t size);

        template<typename Type>
        bool operator&(Type& value)
        {
            if constexpr (std::is_base_of_v<std::decay_t<SerializableObject>, Type>)
            {
                process_serializable_object(value);
            }
            else
            {
                size_t size = sizeof(Type);
                byte* data  = reinterpret_cast<byte*>(&value);

                if (is_reading())
                {
                    read_data(data, size);
                }
                else if (is_saving())
                {
                    write_data(data, size);
                }
            }

            return *this;
        }

        inline operator bool()
        {
            return _M_process_status;
        }
    };


    ENGINE_EXPORT bool operator&(Archive&, String&);

    template<typename Type, typename AllocatorType>
    FORCE_INLINE bool operator&(Archive& ar, Vector<Type, AllocatorType>& vector)
    {
        size_t size = vector.size();

        ar& size;
        if (ar.is_reading())
        {
            vector.resize(size);
        }

        if constexpr (std::is_fundamental_v<Type>)
        {
            byte* data   = reinterpret_cast<byte*>(vector.data());
            size_t bytes = size * sizeof(Type);

            if (ar.is_reading())
            {
                ar.read_data(data, bytes);
            }
            else if (ar.is_saving())
            {
                ar.write_data(data, bytes);
            }
        }
        else
        {
            for (auto& ell : vector)
            {
                ar& ell;
            }
        }

        return ar;
    }
}// namespace Engine
