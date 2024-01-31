#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/render_resource.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_object.hpp>
#include <angelscript.h>
#include <cstring>
#include <typeinfo>


namespace Engine
{

    static thread_local bool _M_next_available_for_gc = false;

    implement_class(Object, Engine, Class::IsScriptable);

#define script_virtual_method(type, method)


    static FORCE_INLINE bool can_update_reference()
    {
        return engine_instance && !engine_instance->is_shuting_down();
    }

    static void add_object_reference(Object* object)
    {
        if (object && can_update_reference())
        {
            object->add_reference();
        }
    }

    static void remove_object_reference(Object* object)
    {
        if (object && can_update_reference())
        {
            object->remove_reference();
        }
    }

    static void register_object_to_script(ScriptClassRegistrar* registrar, Class* self)
    {
        String factory = Strings::format("{}@ f()", self->name().c_str());

        ScriptEngineInitializeController().require("Initialize bindings");

        if (!self->flags(Class::IsSingletone))
        {
            registrar->behave(ScriptClassBehave::Factory, factory.c_str(), self->static_constructor(), ScriptCallConv::CDECL);
        }

        registrar->require_type("Engine::Package");

        registrar->behave(ScriptClassBehave::AddRef, "void f()", add_object_reference, ScriptCallConv::CDECL_OBJFIRST)
                .behave(ScriptClassBehave::Release, "void f()", remove_object_reference, ScriptCallConv::CDECL_OBJFIRST)
                .method("const string& string_name() const", &Object::string_name)
                .method("Engine::ObjectRenameStatus name(StringView, bool = false)",
                        method_of<ObjectRenameStatus, Object, StringView, bool>(&Object::name))
                .static_function("Package@ root_package()", &Object::root_package)
                .method("string as_string() const", &Object::as_string)
                .method("bool add_to_package(Package@, bool)", &Object::add_to_package)
                .static_function("Package@ find_package(StringView, bool)",
                                 func_of<Package*(StringView, bool)>(&Object::find_package))
                .static_function("Object@ static_find_object(const StringView&)",
                                 func_of<Object*(const StringView&)>(&Object::find_object))
                .method("Object& remove_from_package()", &Object::remove_from_package)
                .method("const Name& name() const", method_of<const Name&, Object>(&Object::name))
                .method("string opConv() const", &Object::as_string)
                .virtual_method("Object@ preload()",
                                func_of<Object&(Object*)>([](Object* self) -> Object& { return self->preload(); }),
                                ScriptCallConv::CDECL_OBJFIRST)
                .virtual_method("Object@ postload()",
                                func_of<Object&(Object*)>([](Object* self) -> Object& { return self->postload(); }),
                                ScriptCallConv::CDECL_OBJFIRST)
                .virtual_method("Object@ destroy_script_object(?& in)",
                                func_of<Object&(Object * self, asIScriptObject * *obj)>(
                                        [](Object* self, asIScriptObject** obj) -> Object& {
                                            // Obj always must be descriptor!
                                            ScriptObject object = *obj;
                                            object.add_reference();
                                            return self->destroy_script_object(&object);
                                        }))
                .virtual_method("Engine::Class@ class_instance() const",
                                func_of<Class*(Object*)>([](Object* object) -> Class* { return object->class_instance(); }));
    }

    implement_initialize_class(Object)
    {
        ScriptEnumRegistrar("Engine::ObjectRenameStatus")
                .set("Skipped", static_cast<int_t>(ObjectRenameStatus::Skipped))
                .set("Success", static_cast<int_t>(ObjectRenameStatus::Success))
                .set("Failed", static_cast<int_t>(ObjectRenameStatus::Failed));

        static_class_instance()->set_script_registration_callback(register_object_to_script);
    }

    void Object::prepare_next_object_for_gc()
    {
        _M_next_available_for_gc = true;
    }

    static Vector<Index>& get_free_indexes_array()
    {
        static Vector<Index> array;
        return array;
    }

    static Vector<Object*>& get_instances_array()
    {
        static Vector<Object*> array;
        return array;
    }


    static Package* _M_root_package = nullptr;

    void Object::create_default_package()
    {
        if (_M_root_package == nullptr)
        {
            _M_root_package = new Package();
            _M_root_package->name("Root Package");
            _M_root_package->flags(IsSerializable, false);
        }
    }

    bool Object::private_check_instance(const Class* const check_class) const
    {
        const void* self = this;
        if (self == nullptr)
            return false;

        auto _class = class_instance();
        return _class != nullptr && _class->is_a(check_class);
    }


    Object::Object() : _M_package(nullptr), _M_references(0), _M_instance_index(Constants::index_none)
    {
        _M_owner                   = nullptr;
        ObjectArray& objects_array = get_instances_array();
        _M_instance_index          = objects_array.size();

        if (!get_free_indexes_array().empty())
        {
            _M_instance_index = get_free_indexes_array().back();
            get_free_indexes_array().pop_back();
            objects_array[_M_instance_index] = this;
        }
        else
        {
            objects_array.push_back(this);
        }


        flags(Flag::IsSerializable, true);
        flags(Flag::IsEditable, true);
        flags(Flag::IsAvailableForGC, _M_next_available_for_gc);
        _M_next_available_for_gc = false;
    }

    ENGINE_EXPORT HashIndex Object::hash_of_name(const StringView& name)
    {
        return memory_hash_fast(name.data(), name.length(), 0);
    }

    HashIndex Object::hash_index() const
    {
        return _M_name.hash();
    }

    ENGINE_EXPORT Package* Object::load_package(const StringView& name)
    {
        // Try to find package
        Package* package = Object::find_object_checked<Package>(name);

        if (package != nullptr)
            return package;

        package = find_package(name, true);

        if (package != nullptr && !package->load())
        {
            delete package;
            return nullptr;
        }

        return package;
    }

    ENGINE_EXPORT String Object::package_name_of(const StringView& name)
    {
        auto index = name.find_last_of(Constants::name_separator);
        if (index == String::npos)
            return "";

        index -= 1;
        return String(name.substr(0, index));
    }

    ENGINE_EXPORT String Object::object_name_of(const StringView& name)
    {
        auto pos = name.find_last_of(Constants::name_separator);
        if (pos == String::npos)
        {
            return String(name);
        }

        pos += Constants::name_separator.length() - 1;
        return String(name.substr(pos, name.length() - pos));
    }

    const Object& Object::remove_from_instances_array() const
    {
        if (_M_instance_index < get_instances_array().size())
        {
            get_instances_array()[_M_instance_index] = nullptr;
            if (!engine_instance->is_shuting_down())
            {
                get_free_indexes_array().push_back(_M_instance_index);
            }
            _M_instance_index = Constants::max_size;
        }
        return *this;
    }


    ENGINE_EXPORT const ObjectArray& Object::all_objects()
    {
        return get_instances_array();
    }

    Object::~Object()
    {
        if (flags(Flag::IsDestructed) == false)
        {
            if (_M_package)
            {
                _M_package->remove_object(this);
                _M_package = nullptr;
            }
            remove_from_instances_array();
            flags(Flag::IsDestructed, true);
        }
    }

    const String& Object::string_name() const
    {
        return _M_name.to_string();
    }


    ObjectRenameStatus Object::name(StringView new_name, bool autorename)
    {
        if (_M_name == new_name)
            return ObjectRenameStatus::Skipped;

        Package* package_backup = _M_package;
        Name name_backup        = _M_name;

        auto restore_object_name = [&package_backup, &name_backup, this]() {
            _M_name = name_backup;
            if (package_backup)
            {
                package_backup->add_object(this);
            }
        };


        Package* package = _M_package;
        if (package)
        {
            package->remove_object(this);
        }

        // Find package
        const String& separator_text = Constants::name_separator;
        size_t separator_index       = new_name.find_first_of(separator_text);

        if (separator_index != StringView::npos)
        {
            package = root_package();
        }

        while (separator_index != StringView::npos && package)
        {
            StringView package_name = new_name.substr(0, separator_index);
            Package* next_package   = package->find_object_checked<Package>(package_name, false);

            if (next_package == nullptr)
            {
                next_package = Object::new_instance<Package>();
                next_package->name(package_name);
                if (!package->add_object(next_package, false))
                {
                    restore_object_name();
                    return ObjectRenameStatus::Failed;
                }
            }

            package  = next_package;
            new_name = new_name.substr(separator_index + separator_text.length());

            separator_index = new_name.find(separator_text);
        }

        // Apply new object name
        _M_name = new_name;

        if (package)
        {
            if (!package->add_object(this, autorename))
            {
                restore_object_name();
                return ObjectRenameStatus::Failed;
            }
        }

        return ObjectRenameStatus::Success;
    }


    const Name& Object::name() const
    {
        return _M_name;
    }

    Object* Object::copy()
    {
        if (flags(Flag::IsDestructed))
            return nullptr;

        Object* object = nullptr;

        {
            object        = class_instance()->create_object();
            object->flags = flags;
        }

        return object;
    }

    bool Object::add_to_package(Package* package, bool autorename)
    {
        return package->add_object(this, autorename);
    }

    Object& Object::remove_from_package()
    {
        if (_M_package && _M_package != _M_root_package)
            _M_package->remove_object(this);
        return *this;
    }

    Object* Object::noname_object()
    {
        static thread_local byte data[sizeof(Object)];
        return reinterpret_cast<Object*>(data);
    }

    Package* Object::package() const
    {
        return _M_package;
    }


    String Object::full_name() const
    {
        static auto object_name_of = [](const Object* object) -> String {
            if (object->_M_name.is_valid())
                return object->_M_name.to_string();

            return Strings::format("Noname object {}", object->_M_instance_index);
        };

        static auto parent_object_of = [](const Object* object) -> Object* {
            Object* parent = object->owner();
            if (parent)
                return parent;

            parent = object->package();
            if (parent != Object::root_package())
                return parent;

            return nullptr;
        };

        String result         = object_name_of(this);
        const Object* current = parent_object_of(this);

        while (current)
        {
            result  = Strings::format("{}{}{}",
                                      (current->_M_name.is_valid()
                                               ? _M_name.to_string()
                                               : Strings::format("Noname object {}", current->_M_instance_index)),
                                      Constants::name_separator, result);
            current = parent_object_of(current);
        }

        return result;
    }

    Counter Object::references() const
    {
        return _M_references;
    }

    void Object::add_reference()
    {
        ++_M_references;
    }

    void Object::remove_reference()
    {
        --_M_references;
    }

    bool Object::is_noname() const
    {
        return !_M_name.is_valid();
    }


    ENGINE_EXPORT const Package* root_package()
    {
        return _M_root_package;
    }

    ENGINE_EXPORT Object* Object::find_object(const StringView& object_name)
    {
        return _M_root_package->find_object(object_name);
    }

    Object& Object::preload()
    {
        return *this;
    }

    Object& Object::postload()
    {
        return *this;
    }

    Object& Object::reload()
    {
        return *this;
    }

    Object* Object::owner() const
    {
        return _M_owner;
    }

    Object& Object::owner(Object* new_owner)
    {
        _M_owner = new_owner;
        return *this;
    }

    Object& Object::destroy_script_object(class ScriptObject* object)
    {
        return *this;
    }

    static FORCE_INLINE Package* find_next_package(Package* package, const StringView& name, bool create)
    {
        Package* next_package = package->find_object_checked<Package>(name, false);
        if (next_package == nullptr && create)
        {
            next_package = Object::new_instance<Package>();
            next_package->name(name);
            package->add_object(next_package);
        }
        return next_package;
    }

    Package* Object::find_package(StringView name, bool create)
    {
        Package* package = const_cast<Package*>(root_package());

        const String& separator_text = Constants::name_separator;
        const size_t separator_len   = separator_text.length();

        size_t separator = name.find_first_of(separator_text);

        while (separator != StringView::npos && package)
        {
            package   = find_next_package(package, name.substr(0, separator), create);
            name      = name.substr(separator + separator_len);
            separator = name.find_first_of(separator_text);
        }

        return package ? find_next_package(package, name, create) : nullptr;
    }

    String Object::as_string() const
    {
        return Strings::format("{}: {}", class_instance()->name().c_str(),
                               _M_name.is_valid() ? _M_name.to_string().c_str() : "NoName");
    }

    Index Object::instance_index() const
    {
        return _M_instance_index;
    }

    bool Object::archive_process(Archive& archive)
    {
        if (!SerializableObject::archive_process(archive))
        {
            return false;
        }

        return flags(Flag::IsSerializable);
    }

    bool Object::is_valid() const
    {
        if (flags(IsUnreachable))
            return false;
        return true;
    }

    Path Object::filepath() const
    {
        const Package* pkg = instance_cast<Package>();
        if (!pkg)
        {
            pkg = _M_package;
        }

        if (!pkg || pkg == root_package())
            return {};

        Path path = pkg->string_name() + Constants::package_extention;
        pkg       = pkg->package();

        while (pkg && pkg != root_package())
        {
            path = Path(pkg->string_name()) / path;
            pkg  = pkg->package();
        }

        return path;
    }

    bool Object::is_editable() const
    {
        return flags(IsEditable);
    }

    Package* Object::root_package()
    {
        Object::create_default_package();
        return _M_root_package;
    }
}// namespace Engine
