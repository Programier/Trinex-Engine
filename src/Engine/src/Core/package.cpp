#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/engine_config.hpp>
#include <Core/etl/vector_stream.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{

    struct HeaderEntry {
    };

    Package::Package()
    {
        flag(Object::IsPackage, true);
    }

    bool Package::add_object(Object* object, bool autorename)
    {
        if (!object)
            return false;

        if (object->is_noname())
        {
            error_log("Package", "Cannot add no name object to package!");
            return false;
        }

        if (!object->is_valid())
        {
            error_log("Package", "Cannot add invalid object to package");
            return false;
        }

        if (!autorename && contains_object(object->_M_name))
        {
            error_log("Package", "Cannot add object to package. Object with name '%s' already exist in package!",
                      object->string_name().c_str());
            return false;
        }

        if (object->_M_package)
        {
            object->_M_package->remove_object(object);
        }

        while (contains_object(object->_M_name))
        {
            object->name(object->string_name() + "_new");
        }

        object->_M_package = this;
        _M_objects.insert_or_assign(object->name(), object);
        return true;
    }

    Package& Package::remove_object(Object* object)
    {
        if (!object || object->_M_package != this)
            return *this;

        _M_objects.erase(object->name());
        object->_M_package = nullptr;
        return *this;
    }

    Object* Package::find_object_private_no_recurce(const char* _name, size_t name_len) const
    {
        Name object_name(_name, name_len);
        auto it = _M_objects.find(object_name);
        if (it == _M_objects.end())
            return nullptr;
        return it->second;
    }

    Object* Package::find_object_private(const char* _name, size_t name_len) const
    {
        const char* end_name       = _name + name_len;
        const size_t separator_len = Constants::name_separator.length();
        const char* separator  = Strings::strnstr(_name, name_len, Constants::name_separator.c_str(), separator_len);
        const Package* package = this;


        while (separator && package)
        {
            size_t current_len = separator - _name;
            package            = package->find_object_checked<Package>(_name, current_len, false);
            _name              = separator + separator_len;
            separator = Strings::strnstr(_name, end_name - _name, Constants::name_separator.c_str(), separator_len);
        }

        return package ? package->find_object_private_no_recurce(_name, end_name - _name) : nullptr;
    }

    Object* Package::find_object(const String& object_name, bool recursive) const
    {
        if (recursive)
            return find_object_private(object_name.c_str(), object_name.length());
        return find_object_private_no_recurce(object_name.c_str(), object_name.length());
    }

    Object* Package::find_object(const char* object_name, bool recursive) const
    {
        if (recursive)
            return find_object_private(object_name, std::strlen(object_name));
        return find_object_private_no_recurce(object_name, std::strlen(object_name));
    }

    Object* Package::find_object(const char* object_name, size_t name_len, bool recursive) const
    {
        if (recursive)
            return find_object_private(object_name, name_len);
        return find_object_private_no_recurce(object_name, name_len);
    }

    const Package::ObjectMap& Package::objects() const
    {
        return _M_objects;
    }


    bool Package::contains_object(const Object* object) const
    {
        return object ? object->package() == this : false;
    }

    bool Package::contains_object(const String& name) const
    {
        return find_object(name, false) != nullptr;
    }


    static void create_header(Vector<HeaderEntry>& header)
    {}

    bool Package::save(BufferWriter* writer) const
    {

        if (!flag(Object::IsSerializable))
        {
            error_log("Package", "Cannot save non-serializable package!");
            return false;
        }


        // Compress data
        VectorOutputStream stream;


        const bool need_delete_writer = writer == nullptr;

        if (need_delete_writer)
        {
            writer = new FileWriter(filepath());
        }

        Archive ar(writer);

        uint_t flag = TRINEX_ENGINE_FLAG;
        ar& flag;

        if (need_delete_writer)
        {
            delete writer;
        }

        return false;
    }

    bool Package::load(BufferReader* reader, bool clean)
    {
        unimplemented_method_exception();
        return false;
    }


    Package::~Package()
    {
        //        Package* next_package = const_cast<Package*>(root_package());
        //        if (this == next_package)
        //            next_package = nullptr;

        //        for (Object* object : _M_objects)
        //        {
        //            object->_M_package = next_package;
        //        }
    }

    implement_class(Package, "Engine");
    implement_initialize_class(Package)
    {}

}// namespace Engine
