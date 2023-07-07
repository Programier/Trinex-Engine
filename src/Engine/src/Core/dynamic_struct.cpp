#include <Core/class.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/exception.hpp>

namespace Engine
{
    ushort_t DynamicStructBase::normalize_align(ushort_t requested_align, ushort_t min_align)
    {
        if (requested_align <= min_align)
            return min_align;

        return requested_align % min_align != 0 ? (static_cast<ushort_t>(requested_align / min_align) + 1) * min_align
                                                : requested_align;
    }

    DynamicStructBase& DynamicStructBase::recalculate_struct_size(ushort_t max_align)
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

        for (DynamicStructInstanceProxy* instance : _M_instances)
        {
            if (instance)
            {
                instance->reallocate();
            }
        }

        return *this;
    }

    DynamicStructBase& DynamicStructBase::unlink_instance(DynamicStructInstanceProxy* instance)
    {
        if (!_M_destruct_stage)
        {
            _M_free_indexes.push_back(instance->_M_instance_index);
        }
        _M_instances[instance->_M_instance_index] = nullptr;
        return *this;
    }

    DynamicStructBase& DynamicStructBase::recalculate_offsets()
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

    DynamicStructBase& DynamicStructBase::add_field(const DynamicStructBase::Field& field, Index index)
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

    DynamicStructBase& DynamicStructBase::remove_field(Index index)
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

    size_t DynamicStructBase::size() const
    {
        return _M_size;
    }

    const Vector<DynamicStructBase::Field>& DynamicStructBase::fields() const
    {
        return _M_fields;
    }

    ushort_t DynamicStructBase::align() const
    {
        return _M_align;
    }

    DynamicStructBase& DynamicStructBase::align(ushort_t value)
    {
        _M_requsted_align = value;
        return recalculate_struct_size();
    }

    const Vector<DynamicStructInstanceProxy*>& DynamicStructBase::instances() const
    {
        return _M_instances;
    }

    DynamicStructBase::~DynamicStructBase()
    {
        _M_destruct_stage = true;
        for (DynamicStructInstanceProxy* instance : _M_instances)
        {
            delete instance;
        }
    }


    DynamicStructInstance::DynamicStructInstance(DynamicStructBase* struct_instance, Index instance_index)
        : DynamicStructInstanceProxy(struct_instance, instance_index)
    {
        reallocate();
    }

    DynamicStructInstanceProxy& DynamicStructInstance::reallocate()
    {
        _M_data.resize(this->struct_instance()->size());
        return *this;
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

    DynamicStructInstanceProxy::DynamicStructInstanceProxy(DynamicStructBase* struct_instance, Index index)
    {
        _M_struct         = struct_instance;
        _M_instance_index = index;
    }

    DynamicStructInstanceProxy& DynamicStructInstanceProxy::begin_update()
    {
        return *this;
    }

    DynamicStructInstanceProxy& DynamicStructInstanceProxy::end_update()
    {
        return *this;
    }

    DynamicStructBase* DynamicStructInstanceProxy::struct_instance() const
    {
        return _M_struct;
    }

    DynamicStructInstanceProxy& DynamicStructInstanceProxy::reallocate()
    {
        throw EngineException("Cannot reallocate instance");
    }

    byte* DynamicStructInstanceProxy::field(Index index)
    {
        if (index < _M_struct->_M_fields.size())
        {
            auto instance_data = data();
            if (instance_data)
            {
                return instance_data + _M_struct->_M_fields[index].offset;
            }
            return nullptr;
        }

        return nullptr;
    }

    const byte* DynamicStructInstanceProxy::field(Index index) const
    {
        if (index < _M_struct->_M_fields.size())
        {
            auto instance_data = data();
            if (instance_data)
            {
                return instance_data + _M_struct->_M_fields[index].offset;
            }
            return nullptr;
        }

        return nullptr;
    }

    DynamicStructInstanceProxy::~DynamicStructInstanceProxy()
    {
        _M_struct->unlink_instance(this);
    }


    register_class(Engine::DynamicStructBase);
}// namespace Engine
