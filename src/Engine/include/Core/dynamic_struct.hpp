#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class ENGINE_EXPORT DynamicStructInstance;
    class ENGINE_EXPORT DynamicStructBase;
    class ENGINE_EXPORT DynamicStructInstanceProxy;


    class ENGINE_EXPORT DynamicStructBase : public Object
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
                field.align  = DynamicStructBase::normalize_align(align, alignof(T));
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

    protected:
        Vector<DynamicStructInstanceProxy*> _M_instances;
        Vector<Index> _M_free_indexes;

        Vector<Field> _M_fields;
        std::size_t _M_size;
        std::size_t _M_requsted_align = 0;
        std::size_t _M_align          = 0;

        bool _M_destruct_stage = false;

        DynamicStructBase& recalculate_offsets();
        DynamicStructBase& recalculate_struct_size(ushort_t max_align = 0);
        static ushort_t normalize_align(ushort_t requested_align, ushort_t min_align);
        DynamicStructBase& unlink_instance(DynamicStructInstanceProxy* instance);

    public:
        DynamicStructBase& add_field(const Field& field, Index index = ~0);
        DynamicStructBase& remove_field(Index index = ~0);

        size_t size() const;
        const Vector<Field>& fields() const;
        ushort_t align() const;
        DynamicStructBase& align(ushort_t value);
        const Vector<DynamicStructInstanceProxy*>& instances() const;


        ~DynamicStructBase();
        friend class DynamicStructInstanceProxy;
    };

    using DynamicStructField = DynamicStructBase::Field;


    class ENGINE_EXPORT DynamicStructInstanceProxy
    {
    private:
        Index _M_instance_index;
        DynamicStructBase* _M_struct = nullptr;

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


    protected:
        virtual DynamicStructInstanceProxy& reallocate();

    public:
        DynamicStructInstanceProxy(DynamicStructBase* struct_instance, Index index);

        virtual DynamicStructInstanceProxy& begin_update();
        virtual DynamicStructInstanceProxy& end_update();
        DynamicStructBase* struct_instance() const;
        virtual byte* data()             = 0;
        virtual const byte* data() const = 0;

        byte* field(Index index);
        const byte* field(Index index) const;

        template<typename Type>
        Type get(Index index)
        {
            byte* instance_field = field(index);

            if (instance_field == nullptr)
            {
                return return_nullptr<Type>("Failed to get instance field");
            }
            return *reinterpret_cast<std::remove_reference_t<Type>*>(instance_field);
        }

        template<typename Type>
        const Type get(Index index) const
        {
            const byte* instance_field = field(index);

            if (instance_field == nullptr)
            {
                return return_nullptr<Type>("Failed to get instance field");
            }
            return *reinterpret_cast<const std::remove_reference_t<Type>*>(instance_field);
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

        virtual ~DynamicStructInstanceProxy();

        friend class DynamicStructBase;
    };


    class ENGINE_EXPORT DynamicStructInstance : public DynamicStructInstanceProxy
    {
    private:
        Vector<byte> _M_data;

    protected:
        DynamicStructInstanceProxy& reallocate() override;

    public:
        DynamicStructInstance(DynamicStructBase* strunct_instance, Index index);
        const Vector<byte>& vector() const;
        byte* data() override;
        const byte* data() const override;
    };

    template<typename InstanceProxy = DynamicStructInstance>
    class DynamicStruct : public DynamicStructBase
    {
    public:
        using Super = DynamicStructBase;

        InstanceProxy* create_instance()
        {
            InstanceProxy* result = nullptr;
            if (_M_free_indexes.empty())
            {
                result = new InstanceProxy(this, _M_instances.size());
                _M_instances.push_back(result);
            }
            else
            {
                Index index = _M_free_indexes.back();
                _M_free_indexes.pop_back();
                result              = new InstanceProxy(this, index);
                _M_instances[index] = result;
            }
            return result;
        }
    };


}// namespace Engine