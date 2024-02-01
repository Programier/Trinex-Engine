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
    struct HeaderEntry {
        Vector<Name> class_hierarchy;
        size_t offset            = 0;
        size_t uncompressed_size = 0;
        Object* object           = nullptr;
        Name object_name;

        Vector<byte> compressed_data;

        FORCE_INLINE Class* find_class()
        {
            Class* instance = nullptr;
            for (auto& entry : class_hierarchy)
            {
                if ((instance = Class::static_find(entry.to_string(), false)))
                {
                    return instance;
                }
            }
            return instance;
        }

        Object* load(Package* pkg, Archive& ar)
        {
            Vector<byte> compressed_buffer;
            Vector<byte> uncompressed_buffer;

            VectorReader uncompressed_reader = &uncompressed_buffer;
            Archive uncompressed_ar          = &uncompressed_reader;

            Class* object_class = find_class();
            if (object_class)
            {
                if (!(object = pkg->find_object(object_name, false)))
                {
                    object = object_class->create_object();
                    object->name(object_name);

                    pkg->add_object(object);
                    object->preload();
                }

                ar.reader()->position(offset);
                ar& compressed_buffer;

                uncompressed_buffer.resize(uncompressed_size);
                Compressor::decompress(compressed_buffer, uncompressed_buffer);

                uncompressed_reader.VectorReaderBase::position(0);
                object->archive_process(uncompressed_ar);

                object->postload();
            }

            return object;
        }
    };

    struct Header {
        Vector<HeaderEntry> entries;

        Header& build(const Package* package)
        {
            size_t current_entry = 0;
            auto& objects        = package->objects();
            entries.resize(objects.size());

            for (auto& [name, object] : objects)
            {
                HeaderEntry& entry    = entries[current_entry];
                entry.object          = object;
                entry.object_name     = object->name();
                entry.class_hierarchy = object->class_instance()->hierarchy(1);// Skip Object class

                // Make buffer
                Vector<byte> object_data;

                {
                    VectorWriter writer = &object_data;
                    Archive ar(&writer);

                    if (!object->archive_process(ar))
                    {
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

            current_entry = entries.size() - current_entry;

            if (current_entry > 0)
            {
                entries.resize(entries.size() - current_entry);
            }

            return *this;
        }

        BufferReader::ReadPos load(BufferReader* reader)
        {
            entries.clear();

            Archive ar(reader);

            if (!(ar & entries))
            {
                error_log("Package", "Failed to reader header!");
            }

            return reader->position();
        }

        BufferWriter::WritePos save(BufferWriter* writer)
        {
            Archive ar(writer);
            ar& entries;
            return writer->position();
        }

        bool empty() const
        {
            return entries.empty();
        }
    };

    ENGINE_EXPORT bool operator&(Archive& ar, HeaderEntry& entry)
    {
        ar& entry.class_hierarchy;
        ar& entry.offset;
        ar& entry.uncompressed_size;
        ar& entry.object_name;

        return ar;
    }

    static bool operator&(Archive& ar, Header& header)
    {
        ar& header.entries;
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

    Object* Package::find_object_private_no_recurce(const StringView& _name) const
    {
        Name object_name = _name;
        auto it          = _M_objects.find(object_name);
        if (it == _M_objects.end())
            return nullptr;
        return it->second;
    }

    Object* Package::find_object_private(StringView _name) const
    {
        const String& separator    = Constants::name_separator;
        const size_t separator_len = separator.length();
        size_t separator_index     = _name.find_first_of(separator);
        const Package* package     = this;

        while (separator_index != StringView::npos && package)
        {
            package         = package->find_object_checked<Package>(_name.substr(0, separator_index), false);
            _name           = _name.substr(separator_index + separator_len);
            separator_index = _name.find_first_of(separator);
        }

        return package ? package->find_object_private_no_recurce(_name) : nullptr;
    }

    Object* Package::find_object(const StringView& object_name, bool recursive) const
    {
        if (recursive)
            return find_object_private(object_name);
        return find_object_private_no_recurce(object_name);
    }

    const Package::ObjectMap& Package::objects() const
    {
        return _M_objects;
    }

    bool Package::contains_object(const Object* object) const
    {
        return object ? object->package() == this : false;
    }

    bool Package::contains_object(const StringView& name) const
    {
        return find_object(name, false) != nullptr;
    }


    template<typename Type>
    static Type* open_package_file(const Package* package, bool create_dir = false)
    {
        Path path = engine_config.resources_dir / package->filepath();
        if (create_dir)
        {
            FileManager::root_file_manager()->create_dir(FileManager::dirname_of(path));
        }

        Type* value = new Type(path);
        if (value->is_open())
        {
            return value;
        }

        delete value;

        if constexpr (std::is_base_of_v<BufferReader, Type>)
        {
            error_log("Package", "Failed to load package '%s': File '%s' not found!", package->full_name().c_str(), path.c_str());
        }
        else
        {
            error_log("Package", "Failed to save package '%s': Failed to create file '%s'!", package->full_name().c_str(),
                      path.c_str());
        }

        return nullptr;
    }

    bool Package::save(BufferWriter* writer) const
    {
        if (!flags(Object::IsSerializable))
        {
            error_log("Package", "Cannot save non-serializable package!");
            return false;
        }

        Header current_header;
        current_header.build(this);

        if (current_header.empty())
        {
            error_log("Package", "Cannot save package '%s', because header is empty!", full_name().c_str());
            return false;
        }

        bool need_destroy_writer = (writer == nullptr);

        if (need_destroy_writer)
        {
            writer = open_package_file<FileWriter>(this, true);
        }

        if (!writer)
            return false;

        Archive ar(writer);

        FileFlag flag = FileFlag::package_flag();
        ar& flag;
        auto header_position = writer->position();

        ar& current_header;

        for (auto& entry : current_header.entries)
        {
            entry.offset = writer->position();
            ar& entry.compressed_data;
        }

        writer->position(header_position);
        ar& current_header;

        if (need_destroy_writer)
        {
            delete writer;
        }

        return false;
    }

    bool Package::save(const Path& path) const
    {
        FileWriter writer = path;
        return save(&writer);
    }

    bool Package::load(BufferReader* reader, Flags flags)
    {
        bool need_destroy_reader = reader == nullptr;

        if (need_destroy_reader)
        {
            reader = open_package_file<FileReader>(this, false);
        }

        if (!reader)
            return false;

        Archive ar(reader);

        bool is_valid = true;

        if (need_destroy_reader)
        {
            FileFlag flag;
            ar& flag;

            is_valid = flag == FileFlag::package_flag();
            if (!is_valid)
            {
                error_log("Package", "Invalid package flag in file");
            }
        }

        if (is_valid)
        {
            Header header;
            header.load(reader);

            for (auto& entry : header.entries)
            {
                entry.load(this, ar);
            }
        }
        if (need_destroy_reader)
        {
            delete reader;
        }

        return true;
    }

    bool Package::load(const Path& path, Flags flags)
    {
        FileReader reader = path;
        return load(&reader, flags);
    }

    Object* Package::load_object(const StringView& name, Flags flags, BufferReader* reader)
    {
        Object* object = find_object(name);

        if (object)
            return object;

        bool need_delete_reader = reader == nullptr;

        if (need_delete_reader)
        {
            reader = open_package_file<FileReader>(this, false);

            if (!reader)
                return nullptr;
        }


        Archive ar(reader);

        if (need_delete_reader)
        {
            FileFlag flag;
            ar& flag;

            trinex_always_check(flag == FileFlag::package_flag(), "Invalid package flag in file");
        }

        Header header;
        header.load(reader);


        Object* result = nullptr;

        // TODO: Need optimize it
        for (auto& entry : header.entries)
        {
            if (entry.object_name == name)
            {
                entry.load(this, ar);
                result = entry.object;
                break;
            }
        }

        if (need_delete_reader)
        {
            delete reader;
        }

        return result;
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
                .method("Object@ find_object(const StringView& in, bool=false) const",
                        method_of<Object*, Package, const StringView&, bool>(&Package::find_object))
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


    ENGINE_EXPORT Package* Object::load_package(const StringView& name, Flags flags, class BufferReader* package_reader)
    {
        // Try to find package
        Package* package = Object::find_object_checked<Package>(name);

        if (package != nullptr)
            return package;

        package = find_package(name, true);

        if (package != nullptr && !package->load(package_reader, flags))
        {
            delete package;
            return nullptr;
        }

        return package;
    }

    ENGINE_EXPORT Object* Object::load_object(const StringView& name, Flags flags, class BufferReader* package_reader)
    {
        Object* object = Object::find_object(name);
        if (object)
            return object;

        return Package::find_package(Object::package_name_sv_of(name), true)->load_object(name, flags, package_reader);
    }
}// namespace Engine
