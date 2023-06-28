#include <Core/class.hpp>
#include <Core/dynamic_struct.hpp>

namespace Engine
{
    ushort_t DynamicStruct::normalize_align(ushort_t requested_align, ushort_t min_align)
    {
        if (requested_align <= min_align)
            return min_align;

        return requested_align % min_align != 0 ? (static_cast<ushort_t>(requested_align / min_align) + 1) * min_align
                                                : requested_align;
    }

    DynamicStruct& DynamicStruct::recalculate_struct_size(ushort_t max_align)
    {
        if (_M_fields.empty())
        {
            _M_align = _M_requsted_align;
            _M_size  = 0;
        }
        else
        {
            if (max_align == 0)
            {
                for (auto& field : _M_fields)
                {
                    max_align = std::max(max_align, field.align);
                }
            }


            _M_align = normalize_align(_M_requsted_align, max_align);
            _M_size  = normalize_align(_M_fields.back().offset + _M_fields.back().size, _M_align);
        }

        for (DynamicStructInstance* instance : _M_instances)
        {
            if (instance)
            {
                instance->reallocate();
            }
        }

        return *this;
    }

    DynamicStruct& DynamicStruct::unlink_instance(DynamicStructInstance* instance)
    {
        if (!_M_destruct_stage)
        {
            _M_free_indexes.push_back(instance->_M_instance_index);
        }
        _M_instances[instance->_M_instance_index] = nullptr;
        return *this;
    }


    DynamicStruct& DynamicStruct::recalculate_offsets()
    {
        Field* prev_field = nullptr;

        ushort_t max_align = 0;

        for (auto& field : _M_fields)
        {
            if (prev_field == nullptr)
            {
                field.offset = 0;
            }
            else
            {
                field.offset = normalize_align(prev_field->offset + prev_field->size, field.align);
            }

            max_align  = std::max(max_align, field.align);
            prev_field = &field;
        }


        return recalculate_struct_size(max_align);
    }

    DynamicStruct& DynamicStruct::add_field(const DynamicStruct::Field& field, Index index)
    {
        if (index >= _M_fields.size())
        {
            _M_fields.push_back(field);
        }
        else
        {
            _M_fields.insert(_M_fields.begin() + index, field);
        }

        return recalculate_offsets();
    }

    DynamicStruct& DynamicStruct::remove_field(Index index)
    {
        if (index < _M_fields.size())
        {
            if (index == _M_fields.size() - 1)
            {
                _M_fields.pop_back();
            }
            else
            {
                _M_fields.erase(_M_fields.begin() + index);
            }
        }

        return recalculate_offsets();
    }

    size_t DynamicStruct::size() const
    {
        return _M_size;
    }

    const Vector<DynamicStruct::Field>& DynamicStruct::fields() const
    {
        return _M_fields;
    }

    ushort_t DynamicStruct::align() const
    {
        return _M_align;
    }

    DynamicStruct& DynamicStruct::align(ushort_t value)
    {
        _M_requsted_align = value;
        return recalculate_struct_size();
    }

    DynamicStructInstance* DynamicStruct::create_instance()
    {
        DynamicStructInstance* instance = nullptr;
        if (_M_free_indexes.empty())
        {
            instance = new DynamicStructInstance(this, _M_instances.size());
            _M_instances.push_back(instance);
        }
        else
        {
            Index index = _M_free_indexes.back();
            _M_free_indexes.pop_back();
            instance            = new DynamicStructInstance(this, index);
            _M_instances[index] = instance;
        }

        return instance;
    }

    const Vector<DynamicStructInstance*>& DynamicStruct::instances() const
    {
        return _M_instances;
    }

    DynamicStruct::~DynamicStruct()
    {
        _M_destruct_stage = true;
        for (DynamicStructInstance* instance : _M_instances)
        {
            delete instance;
        }
    }


    DynamicStructInstance::DynamicStructInstance(DynamicStruct* struct_instance, Index instance_index)
        : _M_instance_index(instance_index), _M_struct(struct_instance)
    {
        reallocate();
    }

    DynamicStructInstance& DynamicStructInstance::reallocate()
    {
        _M_data.resize(_M_struct->_M_size);
        return *this;
    }

    DynamicStruct* DynamicStructInstance::struct_instance() const
    {
        return _M_struct;
    }

    const Vector<byte>& DynamicStructInstance::vector() const
    {
        return _M_data;
    }

    byte* DynamicStructInstance::data()
    {
        return _M_data.data();
    }

    const byte* DynamicStructInstance::data() const
    {
        return _M_data.data();
    }

    DynamicStructInstance::~DynamicStructInstance()
    {
        _M_struct->unlink_instance(this);
    }


    register_class(Engine::DynamicStruct);
}// namespace Engine
