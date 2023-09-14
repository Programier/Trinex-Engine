#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>

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
                    max_align = std::max(max_align, field->align);
                }
            }


            _M_align = normalize_align(_M_requsted_align, max_align);
            _M_size  = normalize_align(_M_fields.back()->offset + _M_fields.back()->size, _M_align);
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

        for (Field* field : _M_fields)
        {
            if (prev_field == nullptr)
            {
                field->offset = 0;
            }
            else
            {
                field->offset = normalize_align(prev_field->offset + prev_field->size, field->align);
            }

            max_align  = std::max(max_align, field->align);
            prev_field = field;
        }


        return recalculate_struct_size(max_align);
    }

    DynamicStructBase& DynamicStructBase::add_field(const DynamicStructBase::Field& field, Index index)
    {
        Field* field_copy               = new Field(field);
        _M_fields_map[field_copy->name] = field_copy;

        if (index >= _M_fields.size())
        {
            _M_fields.push_back(field_copy);
        }
        else
        {
            _M_fields.insert(_M_fields.begin() + index, field_copy);
        }

        return recalculate_offsets();
    }

    DynamicStructBase& DynamicStructBase::remove_field(Index index)
    {
        if (index < _M_fields.size())
        {
            if (index == _M_fields.size() - 1)
            {
                _M_fields_map.erase(_M_fields.back()->name);
                delete _M_fields.back();
                _M_fields.pop_back();
            }
            else
            {
                _M_fields_map.erase(_M_fields[index]->name);
                delete _M_fields[index];
                _M_fields.erase(_M_fields.begin() + index);
            }
        }

        return recalculate_offsets();
    }

    size_t DynamicStructBase::size() const
    {
        return _M_size;
    }

    const DynamicStructBase::FieldsArray& DynamicStructBase::fields() const
    {
        return _M_fields;
    }

    const DynamicStructBase::FieldsMap& DynamicStructBase::fields_map() const
    {
        return _M_fields_map;
    }

    DynamicStructBase::Field* DynamicStructBase::find_field(const String& name) const
    {
        auto it = _M_fields_map.find(name);
        if (it == _M_fields_map.end())
        {
            return nullptr;
        }
        return it->second;
    }

    DynamicStructBase::Field* DynamicStructBase::find_field(Index index) const
    {
        if (index < _M_fields.size())
        {
            return _M_fields[index];
        }
        return nullptr;
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


    ENGINE_EXPORT bool operator&(Archive& ar, DynamicStructField*& field)
    {
        if (ar.is_reading() && field == nullptr)
        {
            field = new DynamicStructField();
        }

        if (field)
        {
            ar & field->size;
            ar & field->align;
            ar & field->name;
            ar & field->offset;
        }
        else
        {
            return false;
        }

        return static_cast<bool>(ar);
    }

    bool DynamicStructBase::archive_process(Archive* archive)
    {
        if (Super::archive_process(archive) == false)
            return false;


        Archive& ar = *archive;
        ar& _M_align;
        ar& _M_size;
        ar& _M_requsted_align;
        ar& _M_align;
        ar& _M_fields;

        if (ar.is_reading())
        {
            for (Field* field : _M_fields)
            {
                _M_fields_map[field->name] = field;
            }
        }

        bool status = static_cast<bool>(ar);
        if (status == false)
        {
            error_log("DynamicStructBase", "Failed to process DynamicStructBase!");
        }
        return status;
    }

    DynamicStructBase::~DynamicStructBase()
    {
        _M_destruct_stage = true;
        for (DynamicStructInstanceProxy* instance : _M_instances)
        {
            delete instance;
        }

        for (Field* field : _M_fields)
        {
            delete field;
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


    template<typename Result, typename Key>
    static Result find_field_data(const DynamicStructInstanceProxy* proxy, Key&& key)
    {
        DynamicStructField* field = proxy->struct_instance()->find_field(std::forward<Key>(key));
        if (field)
        {
            auto instance_data = const_cast<Result>(proxy->data());
            if (instance_data)
            {
                return reinterpret_cast<Result>(instance_data + field->offset);
            }
            return nullptr;
        }
        return nullptr;
    }

    byte* DynamicStructInstanceProxy::field_data(Index index)
    {
        return find_field_data<byte*>(this, index);
    }

    const byte* DynamicStructInstanceProxy::field_data(Index index) const
    {
        return find_field_data<const byte*>(this, index);
    }

    byte* DynamicStructInstanceProxy::field_data(const String& name)
    {
        return find_field_data<byte*>(this, name);
    }

    const byte* DynamicStructInstanceProxy::field_data(const String& name) const
    {
        return find_field_data<const byte*>(this, name);
    }

    DynamicStructInstanceProxy::~DynamicStructInstanceProxy()
    {
        _M_struct->unlink_instance(this);
    }

    implement_class(DynamicStructBase, "Engine");
    implement_default_initialize_class(DynamicStructBase);

}// namespace Engine
