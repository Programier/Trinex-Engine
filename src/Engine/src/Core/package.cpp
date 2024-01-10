#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_flag.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <cstring>

namespace Engine
{

    ENGINE_EXPORT bool operator&(Archive& ar, Package::HeaderEntry& entry)
    {
        ar& entry.class_name;
        ar& entry.offset;
        ar& entry.uncompressed_size;
        ar& entry.object_name;

        return ar;
    }

    Package::Package()
    {
        flags(Object::IsPackage, true);
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

        {
            static size_t index    = 2;
            String new_object_name = object->_M_name;
            while (contains_object(new_object_name))
            {
                new_object_name = object->_M_name.to_string() + Strings::format(" {}", index++);
            }
            if (new_object_name != object->_M_name.to_string())
                object->name(new_object_name);
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
        const char* separator      = Strings::strnstr(_name, name_len, Constants::name_separator.c_str(), separator_len);
        const Package* package     = this;


        while (separator && package)
        {
            size_t current_len = separator - _name;
            package            = package->find_object_checked<Package>(_name, current_len, false);
            _name              = separator + separator_len;
            separator          = Strings::strnstr(_name, end_name - _name, Constants::name_separator.c_str(), separator_len);
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

    const Package::Header& Package::header() const
    {
        return _M_header;
    }

    bool Package::contains_object(const Object* object) const
    {
        return object ? object->package() == this : false;
    }

    bool Package::contains_object(const String& name) const
    {
        return find_object(name, false) != nullptr;
    }


    const Package& Package::build_header(Header& header) const
    {
        size_t current_entry = 0;
        header.resize(_M_objects.size());

        for (auto& [name, object] : _M_objects)
        {
            HeaderEntry& entry = header[current_entry];
            entry.object       = object;
            entry.object_name  = object->name();
            entry.class_name   = object->class_instance()->name();

            // Make buffer
            Vector<byte> object_data;

            {
                VectorWriter writer = &object_data;
                Archive ar(&writer);

                if (!object->archive_process(ar))
                {
                    error_log("Package", "Cannot serialize object '%s'", object->full_name().c_str());
                    continue;
                }
                else
                {
                    ++current_entry;
                }
            }

            entry.uncompressed_size = object_data.size();

            // Compress data
            Compressor::compress(object_data, entry.compressed_data);
        }

        current_entry = header.size() - current_entry;

        if (current_entry > 0)
        {
            header.resize(header.size() - current_entry);
        }

        return *this;
    }

    Package::Header& Package::load_header_private(class BufferReader* reader)
    {
        _M_header.clear();

        FileFlag flag;
        Archive ar(reader);

        ar& flag;

        if (flag != FileFlag::package_flag())
        {
            error_log("Package", "File is not trinex package!");
            return _M_header;
        }

        if (!(ar & _M_header))
        {
            error_log("Package", "Failed to reader header!");
        }

        return _M_header;
    }

    bool Package::save() const
    {

        if (!flags(Object::IsSerializable))
        {
            error_log("Package", "Cannot save non-serializable package!");
            return false;
        }

        Header current_header;
        build_header(current_header);

        if (current_header.empty())
        {
            error_log("Package", "Cannot save package '%s', because header is empty!", full_name().c_str());
            return false;
        }


        Path path = engine_config.resources_dir / filepath();
        FileManager::root_file_manager()->create_dir(FileManager::dirname_of(path));
        BufferWriter* writer = FileManager::root_file_manager()->create_file_writer(path);


        Archive ar(writer);

        FileFlag flag = FileFlag::package_flag();
        ar& flag;
        auto header_position = writer->position();

        ar& current_header;

        for (auto& entry : current_header)
        {
            entry.offset = writer->position();
            ar& entry.compressed_data;
        }

        writer->position(header_position);
        ar& current_header;

        delete writer;

        return false;
    }

    bool Package::load()
    {
        _M_objects.clear();

        Path path = engine_config.resources_dir / filepath();
        FileManager::root_file_manager()->create_dir(FileManager::dirname_of(path));
        BufferReader* reader = FileManager::root_file_manager()->create_file_reader(path);
        if (reader == nullptr)
        {
            error_log("Package", "Failed to load package '%s': File '%s' not found!", full_name().c_str(), path.c_str());
            return false;
        }

        Archive ar(reader);

        Vector<byte> compressed_buffer;
        Vector<byte> uncompressed_buffer;

        VectorReader uncompressed_reader = &uncompressed_buffer;
        Archive uncompressed_ar          = &uncompressed_reader;

        load_header_private(reader);

        for (auto& entry : _M_header)
        {
            Class* object_class = Class::static_find(entry.class_name);
            if (object_class)
            {
                entry.object = object_class->create_object();
                entry.object->name(entry.object_name);

                add_object(entry.object);
                entry.object->preload();

                reader->position(entry.offset);
                ar& compressed_buffer;

                uncompressed_buffer.resize(entry.uncompressed_size);
                Compressor::decompress(compressed_buffer, uncompressed_buffer);

                uncompressed_reader.VectorReaderBase::position(0);
                entry.object->archive_process(uncompressed_ar);

                entry.object->postload();
            }
        }

        return true;
    }

    Package::Header& Package::load_header()
    {
        _M_header.clear();
        Path path = engine_config.resources_dir / filepath();
        FileManager::root_file_manager()->create_dir(FileManager::dirname_of(path));
        BufferReader* reader = FileManager::root_file_manager()->create_file_reader(path);

        if (reader == nullptr)
        {
            error_log("Package", "Cannot open file '%s'", path.string().c_str());
            return _M_header;
        }

        load_header_private(reader);
        delete reader;
        return _M_header;
    }


    Package::~Package()
    {
        for (auto& [name, object] : _M_objects)
        {
            object->_M_package = nullptr;
        }
    }


    static void bind_to_script(ScriptClassRegistrar* registrar, Class* self)
    {
        registrar->method("bool add_object(Object@, bool = false)", &Package::add_object)
                .method("Package@ remove_object(Object@)", &Package::remove_object)
                .method("Object@ find_object(const string& in, bool) const",
                        method_of<Object*, Package, const String&, bool>(&Package::find_object))
                /* .method("bool contains_object(const Object@) const",
                        method_of<bool, Package, const Object*>(&Package::contains_object))
                .method("bool contains_object(const string& in) const",
                        method_of<bool, Package, const String&>(&Package::contains_object))*/
                ;
    }

    implement_class(Package, Engine, Class::IsScriptable);
    implement_initialize_class(Package)
    {
        static_class_instance()->set_script_registration_callback(bind_to_script);
    }

}// namespace Engine
