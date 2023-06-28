#pragma once
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class ENGINE_EXPORT DynamicStructInstance;

    class ENGINE_EXPORT DynamicStruct final : public Object
    {
    public:
        using Super = Object;

        struct Field {
            ushort_t size;
            ushort_t offset;
            ushort_t align;

            template<typename T>
            static Field field_of(ushort_t align = alignof(T), ushort_t offset = 0)
            {
                Field field;
                field.offset = offset;
                field.size   = sizeof(T);
                field.align  = DynamicStruct::normalize_align(align, alignof(T));
                return field;
            }

            template<typename T>
            static Field packed_field_of(ushort_t offset = 0)
            {
                Field field;
                field.offset = offset;
                field.size   = sizeof(T);
                field.align  = 0;
                return field;
            }
        };

    private:
        Vector<DynamicStructInstance*> _M_instances;
        Vector<Index> _M_free_indexes;

        Vector<Field> _M_fields;
        std::size_t _M_size;
        std::size_t _M_requsted_align = 0;
        std::size_t _M_align          = 0;

        bool _M_destruct_stage = false;

        DynamicStruct& recalculate_offsets();
        DynamicStruct& recalculate_struct_size(ushort_t max_align = 0);
        static ushort_t normalize_align(ushort_t requested_align, ushort_t min_align);
        DynamicStruct& unlink_instance(DynamicStructInstance* instance);

    public:
        DynamicStruct& add_field(const Field& field, Index index = ~0);
        DynamicStruct& remove_field(Index index = ~0);

        size_t size() const;
        const Vector<Field>& fields() const;
        ushort_t align() const;
        DynamicStruct& align(ushort_t value);

        DynamicStructInstance* create_instance();
        const Vector<DynamicStructInstance*>& instances() const;

        ~DynamicStruct();

        friend class DynamicStructInstance;
    };


    class ENGINE_EXPORT DynamicStructInstance final
    {
    private:
        Vector<byte> _M_data;
        Index _M_instance_index;
        DynamicStruct* _M_struct = nullptr;


        DynamicStructInstance(DynamicStruct* struct_instance, Index instance_index);
        DynamicStructInstance& reallocate();

        template<typename Type>
        Type return_nullptr(const char* msg)
        {
            if constexpr (std::is_reference_v<Type> || !std::is_default_constructible_v<Type>)
            {
                throw EngineException(msg);
            }
            else
            {
                return Type();
            }
        }

        template<typename Type>
        const Type return_nullptr(const char* msg) const
        {
            if constexpr (std::is_reference_v<Type> || !std::is_default_constructible_v<Type>)
            {
                throw EngineException(msg);
            }
            else
            {
                return Type();
            }
        }

    public:
        DynamicStruct* struct_instance() const;
        const Vector<byte>& vector() const;
        byte* data();
        const byte* data() const;

        template<typename Type>
        Type get(Index index)
        {
            if (index >= _M_struct->_M_fields.size())
            {
                return return_nullptr<Type>("Invalid index of field");
            }

            DynamicStruct::Field& field = _M_struct->_M_fields[index];

            if (field.size != sizeof(Type))
            {
                return return_nullptr<Type>("Invalid type of field");
            }

            return *reinterpret_cast<std::remove_reference_t<Type>*>(_M_data.data() + field.offset);
        }

        template<typename Type>
        const Type get(Index index) const
        {
            if (index >= _M_struct->_M_fields.size())
            {
                return return_nullptr<Type>("Invalid index of field");
            }

            DynamicStruct::Field& field = _M_struct->_M_fields[index];

            if (field.size != sizeof(Type))
            {
                return return_nullptr<Type>("Invalid type of field");
            }

            return *reinterpret_cast<std::remove_reference_t<Type>*>(_M_data.data() + field.offset);
        }

        template<typename Type>
        Type& get_ref(Index index)
        {
            return get<Type&>(index);
        }

        template<typename Type>
        Type& get_ref(Index index) const
        {
            return get<Type&>(index);
        }

        ~DynamicStructInstance();

        friend class DynamicStruct;
    };

}// namespace Engine
