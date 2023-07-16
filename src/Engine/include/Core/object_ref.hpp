#pragma once
#include <Core/serializable_object.hpp>
#include <Core/buffer_manager.hpp>

namespace Engine
{
    struct ObjectReferenceBase : public SerializableObject {
    protected:
        String _M_name;

        bool serialize_name(BufferWriter* writer, const String& name) const;
        bool deserialize_name(BufferReader* reader, String& name);
    };

    template<typename Type>
    class ObjectReference : public ObjectReferenceBase
    {
        Type* _M_instance = nullptr;

    public:
        Type* instance()
        {
            if (_M_instance == nullptr && !_M_name.empty())
            {
                _M_instance = Object::load_object(_M_name)->template instance_cast<Type>();
            }

            return _M_instance;
        }

        inline bool archive_process(Archive* archive) override
        {
            if (archive->is_saving())
            {
                if (_M_instance)
                    return serialize_name(archive->writer(), reinterpret_cast<Object*>(_M_instance)->full_name());
                if (!_M_name.empty())
                    return serialize_name(archive->writer(), _M_name);
                return false;
            }
            else
            {
                return deserialize_name(archive->reader(), _M_name);
            }
        }

        Type* operator()() const
        {
            return _M_instance;
        }

        ObjectReference& operator()(Type* obj)
        {
            instance(obj);
            return *this;
        }

        ObjectReference& instance(Type* instance)
        {
            _M_instance = instance;
            _M_name = _M_instance ? _M_instance->name() : "";
         }
    };
}
