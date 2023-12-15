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

    static bool object_comparator(const Object* o1, const Object* o2)
    {
        return o1->hash_index() < o2->hash_index();
    }

    struct HeaderEntry : SerializableObject {
        String name       = "";
        String class_name = "";
        ArrayIndex offset = 0;
        size_t object_size;
        size_t compressed_size;
        Object* object = nullptr;
        Vector<char> compressed_data;

        bool archive_process(Archive* archive) override
        {

            if (archive->is_saving())
            {
                if (object == nullptr)
                    return false;

                name            = object->string_name();
                class_name      = object->class_instance()->name();
                compressed_size = compressed_data.size();
            }

            if (!((*archive) & name))
            {
                error_log("PackageHeader", "Failed to process name of object!");
                return false;
            }

            if (!((*archive) & offset))
            {
                error_log("PackageHeader", "Failed to process offset of object!");
                return false;
            }

            if (!((*archive) & object_size))
            {
                error_log("PackageHeader", "Failed to process size of object!");
                return false;
            }

            if (!((*archive) & class_name))
            {
                error_log("PackageHeader", "Failed to process class name!");
                return false;
            }

            if (!((*archive) & compressed_size))
            {
                error_log("PackageHeader", "Failed to process compressed size!");
                return false;
            }

            return true;
        }

        size_t size() const
        {
            if (object == nullptr)
                return 0;
            return (sizeof(size_t) * 3) + object->string_name().length() + sizeof(offset) + sizeof(object_size) +
                   object->class_instance()->name().length();
        }
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

    bool Package::save(BufferWriter* writer) const
    {
        if (!flag(Object::IsSerializable))
        {
            error_log("Package", "Package '%s' is not serializable!", full_name().c_str());
            return false;
        }

        if (this == root_package())
        {
            error_log("Package", "Cannot save root package! Please, use different package for saving!");
            return false;
        }

        // Creating header
        Vector<HeaderEntry> header;
        header.reserve(_M_objects.size());

        // Write objects into buffers

        size_t offset = sizeof(size_t);
        for (auto& [name, object] : _M_objects)
        {
            if (object->flag(Object::IsSerializable))
            {
                HeaderEntry entry;

                entry.object = object;
                VectorOutputStream temporary_stream;
                BufferWriterWrapper<BufferWriter> temporary_buffer(temporary_stream);

                Archive ar(&temporary_buffer);

                if (!entry.object->archive_process(&ar))
                {
                    error_log("Package", "Failed to compress object '%s'", entry.object->string_name().c_str());
                    continue;
                }

                // Compressing data
                entry.object_size = temporary_stream.vector().size();
                Compressor::compress(temporary_stream.vector(), entry.compressed_data);

                header.push_back(std::move(entry));

                offset += entry.size();
            }
        }

        if (header.empty())
            return false;

        // Update offsets

        for (auto& entry : header)
        {
            entry.offset = offset;
            offset += entry.compressed_data.size();
        }

        bool is_created_writer = false;
        if (writer == nullptr)
        {
            Path path = Path(engine_config.resources_dir) /
                        Path(Strings::replace_all(full_name(), Constants::name_separator, "/") +
                             Constants::package_extention);

            Path dirname = FileManager::dirname_of(path);
            FileManager::root_file_manager()->create_dir(dirname);

            writer = FileManager::root_file_manager()->create_file_writer(path, true);

            if (!writer)
            {
                error_log("Package", "Failed to create file '%s'", path.c_str());
                return false;
            }
            is_created_writer = true;
        }

        auto status = [&is_created_writer, &writer](bool flag) -> bool {
            if (is_created_writer)
                delete writer;
            return flag;
        };

        uint_t flag = TRINEX_ENGINE_FLAG;
        Archive ar(writer);

        if (!(ar & flag))
        {
            error_log("Package", "Failed to write flag to file!");
            return status(false);
        }

        if (!(ar & header))
        {
            error_log("Package", "Failed to write header to file!");
            return status(false);
        }

        for (auto& entry : header)
        {
            if (!writer->write(reinterpret_cast<const byte*>(entry.compressed_data.data()),
                               entry.compressed_data.size()))
            {
                error_log("Package", "Failed to write object '%s' to file!", entry.object->string_name().c_str());
                return status(false);
            }
        }

        return status(true);
    }

    //    static bool check_file(BufferReader* reader)
    //    {
    //        trinex_check(reader, "Reader can't be nullptr");

    //        uint_t flag;
    //        if (!reader->read(flag))
    //        {
    //            error_log("Package", "Failed to read flag to file!");
    //            return false;
    //        }

    //        if (flag != TRINEX_ENGINE_FLAG)
    //        {
    //            error_log("Package", "File is corrupted or is not supported!");
    //            return false;
    //        }

    //        return true;
    //


    bool Package::load(BufferReader* reader, bool clean)
    {
        unimplemented_method_exception();
        return false;
    }


    bool Package::contains_object(const Object* object) const
    {
        return object ? object->package() == this : false;
    }

    bool Package::contains_object(const String& name) const
    {
        return find_object(name, false) != nullptr;
    }


    Object* Package::load_object(const String& name, BufferReader* reader)
    {
        unimplemented_method_exception();
        return nullptr;
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
