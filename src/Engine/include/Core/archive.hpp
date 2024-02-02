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
                data->archive_process(*this);
            }
            else
            {
                data.archive_process(*this);
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

        size_t position() const;
        Archive& position(size_t position);

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

        template<typename Type>
        FORCE_INLINE bool process_vector(Type& vector,
                                         bool (*on_item_processed)(typename Type::value_type&, void* userdata) = nullptr,
                                         void* userdata                                                        = nullptr)
        {
            size_t size = vector.size();
            Archive& ar = *this;

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
                if (on_item_processed)
                {
                    for (auto& ell : vector)
                    {
                        ar& ell;
                        if (!on_item_processed(ell, userdata))
                        {
                            return ar;
                        }
                    }
                }
                else
                {
                    for (auto& ell : vector)
                    {
                        ar& ell;
                    }
                }
            }

            return ar;
        }

        template<typename Type>
        FORCE_INLINE bool process_set(Type& set, bool (*on_item_processed)(typename Type::value_type&, void* userdata) = nullptr,
                                      void* userdata = nullptr)
        {
            size_t size = set.size();
            Archive& ar = *this;

            ar& size;

            if (ar.is_reading())
            {
                set.clear();
                typename Type::value_type value;
                if (on_item_processed)
                {
                    while (size-- > 0)
                    {
                        ar& value;
                        on_item_processed(value, userdata);
                        if (!set.insert(std::move(value)))
                        {
                            return ar;
                        }
                    }
                }
                else
                {
                    while (size-- > 0)
                    {
                        ar& value;
                        set.insert(std::move(value));
                    }
                }
            }
            else if (ar.is_saving())
            {
                if (on_item_processed)
                {
                    for (const typename Type::value_type& ell : set)
                    {
                        ar& const_cast<Type::value_type&>(ell);
                        if (!on_item_processed(const_cast<Type::value_type&>(ell), userdata))
                        {
                            return ar;
                        }
                    }
                }
                else
                {
                    for (const typename Type::value_type& ell : set)
                    {
                        ar& const_cast<Type::value_type&>(ell);
                    }
                }
            }

            return *this;
        }


        template<typename Type>
        FORCE_INLINE bool write_map(Type& map)
        {
            size_t size = map.size();
            Archive& ar = *this;

            ar& size;

            if (ar.is_reading())
            {
                map.clear();
                std::remove_const_t<typename Type::key_type> key;
                typename Type::mapped_type value;

                while (size-- > 0)
                {
                    ar& key;
                    ar& value;
                    map.insert_or_assign(std::move(key), std::move(value));
                }
            }
            else if (ar.is_saving())
            {
                for (auto& [key, value] : map)
                {
                    ar& const_cast<Type::key_type&>(key);
                    ar& value;
                }
            }

            return ar;
        }

        template<typename Type>
        FORCE_INLINE bool write_container(Type& container)
        {
            size_t size = container.size();
            Archive& ar = *this;

            ar& size;
            if (ar.is_reading())
            {
                container.resize(size);
            }

            for (auto& ell : container)
            {
                ar& ell;
            }

            return ar;
        }
    };


    ENGINE_EXPORT bool operator&(Archive&, String&);
    ENGINE_EXPORT bool operator&(Archive&, Path&);

    template<typename Type, typename AllocatorType>
    FORCE_INLINE bool operator&(Archive& ar, Vector<Type, AllocatorType>& vector)
    {
        return ar.process_vector(vector, nullptr);
    }


    template<typename Type, typename AllocatorType>
    FORCE_INLINE bool operator&(Archive& ar, List<Type, AllocatorType>& list)
    {
        return ar.write_container(list);
    }

    template<typename Type, typename AllocatorType = Allocator<Type>>
    FORCE_INLINE bool operator&(Archive& ar, ForwardList<Type, AllocatorType>& list)
    {
        return ar.write_container(list);
    }

    template<typename Type, typename HashType = Hash<Type>, typename Pred = std::equal_to<Type>,
             typename AllocatorType = Allocator<Type>>
    FORCE_INLINE bool operator&(Archive& ar, Set<Type, HashType, Pred, AllocatorType>& set)
    {
        return ar.process_set(set);
    }

    template<typename Type, typename Compare = std::less<Type>, typename AllocatorType = Allocator<Type>>
    FORCE_INLINE bool operator&(Archive& ar, TreeSet<Type, Compare, AllocatorType>& set)
    {
        return ar.process_set(set);
    }

    template<typename Key, typename Value, typename HashType = Hash<Key>, typename Pred = std::equal_to<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    FORCE_INLINE bool operator&(Archive& ar, Map<Key, Value, HashType, Pred, AllocatorType>& map)
    {
        return ar.write_map(map);
    }

    template<typename Key, typename Value, typename Compare = std::less<Key>,
             typename AllocatorType = Allocator<Pair<const Key, Value>>>
    FORCE_INLINE bool operator&(Archive& ar, TreeMap<Key, Value, Compare, AllocatorType>& map)
    {
        return ar.write_map(map);
    }
}// namespace Engine
