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

                name            = object->name();
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
            return (sizeof(size_t) * 3) + object->name().length() + sizeof(offset) + sizeof(object_size) +
                   object->class_instance()->name().length();
        }
    };


    Package::Package()
    {
        trinex_flag(TrinexObjectFlags::IsPackage, true);
    }

    Package::Package(const String& _name)
    {
        trinex_flag(TrinexObjectFlags::IsPackage, true);
        name(_name);
    }

    bool Package::add_object(Object* object, bool autorename)
    {
        if (!object)
            return false;

        for (const auto& pair : _M_filters.callbacks())
        {
            if (!pair.second(object))
            {
                error_log("Package", "The object '%s' does not match the filters", object->full_name().c_str());
                return false;
            }
        }

        if (object->trinex_flag(TrinexObjectFlags::IsNeedDelete))
        {
            error_log("Package", "Cannot add object to package, wich marked for delete");
            return false;
        }

        if (!autorename && contains_object(object->name()))
        {
            error_log("Package", "Cannot add object to package. Object with name '%s' already exist in package!",
                      object->name().c_str());
            return false;
        }

        if (object->_M_package)
        {
            object->_M_package->remove_object(object);
        }

        while (contains_object(object->name()))
        {
            object->name(object->name() + "_new");
        }

        object->_M_package          = this;
        object->_M_index_in_package = validate_index(lower_bound_of(object), object->hash_index());
        if (object->_M_index_in_package == _M_objects.size())
        {
            _M_objects.push_back(object);
        }
        else
        {
            for (Index i = object->_M_index_in_package, j = _M_objects.size(); i < j; i++)
            {
                _M_objects[i]->_M_index_in_package += 1;
            }

            _M_objects.insert(_M_objects.begin() + object->_M_index_in_package, object);
        }
        return true;
    }

    Package& Package::remove_object(Object* object)
    {
        if (!object || object->_M_package != this)
            return *this;

        if (object->_M_index_in_package < _M_objects.size() && _M_objects[object->_M_index_in_package] == object)
        {
            object->_M_package = nullptr;

            for (Index i = object->_M_index_in_package + 1, j = _M_objects.size(); i < j; i++)
            {
                _M_objects[i]->_M_index_in_package -= 1;
            }

            _M_objects.erase(_M_objects.begin() + object->_M_index_in_package);
            object->_M_index_in_package = Constants::index_none;
        }

        return *this;
    }

    Object* Package::get_object_by_name(const String& name) const
    {
        if (_M_objects.empty())
            return nullptr;

        Index index = lower_bound_of(name);
        if (index >= _M_objects.size())
        {
            return nullptr;
        }

        return _M_objects[index]->name() == name ? _M_objects[index] : nullptr;
    }


    Object* Package::find_object(const String& object_name, bool recursive) const
    {
        if (!recursive)
            return get_object_by_name(object_name);

        std::size_t prev_pos           = 0;
        std::size_t pos                = 0;
        const Package* current_package = this;

        while (current_package && (pos = object_name.find(Constants::name_separator, prev_pos)) != String::npos)
        {
            auto sub_name   = object_name.substr(prev_pos, pos - prev_pos);
            prev_pos        = pos + Constants::name_separator.length();
            Object* node    = current_package->get_object_by_name(sub_name);
            current_package = node->instance_cast<Package>();
        }

        return current_package ? current_package->get_object_by_name(
                                         object_name.substr(prev_pos, object_name.length() - prev_pos))
                               : nullptr;
    }

    const Vector<Object*>& Package::objects() const
    {
        return _M_objects;
    }

    bool Package::can_destroy(MessageList& messages)
    {
        bool status = true;
        status      = status && Object::can_destroy(messages);

        for (Object* object : _M_objects)
        {
            status = status && object->can_destroy(messages);
        }

        return status;
    }

    bool Package::save(BufferWriter* writer) const
    {
        if (!trinex_flag(TrinexObjectFlags::IsSerializable))
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
        for (Object* object : _M_objects)
        {
            if (object->trinex_flag(TrinexObjectFlags::IsSerializable))
            {
                HeaderEntry entry;

                entry.object = object;
                VectorOutputStream temporary_stream;
                BufferWriterWrapper<BufferWriter> temporary_buffer(temporary_stream);

                Archive ar(&temporary_buffer);

                if (!entry.object->archive_process(&ar))
                {
                    error_log("Package", "Failed to compress object '%s'", entry.object->name().c_str());
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
                error_log("Package", "Failed to write object '%s' to file!", entry.object->name().c_str());
                return status(false);
            }
        }

        return status(true);
    }

    static bool check_file(BufferReader* reader)
    {
        trinex_check(reader);

        uint_t flag;
        if (!reader->read(flag))
        {
            error_log("Package", "Failed to read flag to file!");
            return false;
        }

        if (flag != TRINEX_ENGINE_FLAG)
        {
            error_log("Package", "File is corrupted or is not supported!");
            return false;
        }

        return true;
    }

    bool Package::load_buffer(BufferReader* reader, Vector<char>& buffer)
    {
        if (this == root_package())
        {
            error_log("Package", "Cannot load root package!");
            return false;
        }

        bool is_created_reader = false;

        if (reader == nullptr)
        {
            String path = engine_config.resources_dir + Strings::replace_all(full_name(), "::", "/") +
                          Constants::package_extention;

            Path dirname = FileManager::dirname_of(path);
            FileManager::root_file_manager()->create_dir(dirname);

            reader = FileManager::root_file_manager()->create_file_reader(path);

            if (!reader)
            {
                error_log("Package", "Failed to create file '%s'", path.c_str());
                return false;
            }
            is_created_reader = true;
        }

        auto status = [&is_created_reader, &reader](bool flag) -> bool {
            if (is_created_reader)
                delete reader;
            return flag;
        };

        if (!check_file(reader))
        {
            return status(false);
        }


        size_t buffer_size = reader->size() - reader->position();
        buffer.clear();
        buffer.shrink_to_fit();
        buffer.resize(buffer_size);

        if (!reader->read(reinterpret_cast<byte*>(buffer.data()), buffer_size))
        {
            error_log("Package", "Failed to read compressed data from file!");
            return status(false);
        }

        return true;
    }


    bool Package::load_entry(void* entry_ptr, Archive* archive_ptr)
    {
        Archive& archive   = *archive_ptr;
        HeaderEntry& entry = *static_cast<HeaderEntry*>(entry_ptr);

        if (entry.object != nullptr)
            return true;

        archive.reader()->position(entry.offset);
        Vector<char> compressed_data(entry.compressed_size, 0);

        archive.reader()->read(reinterpret_cast<byte*>(compressed_data.data()), entry.compressed_size);

        Vector<char> original_data(entry.object_size, 0);
        Compressor::decompress(compressed_data, original_data);

        VectorInputStream stream(original_data);
        BufferReaderWrapper<BufferReader> reader_wrapper(stream);

        Archive ar(&reader_wrapper);

        Class* object_class = Class::static_find_class(entry.class_name);
        if (object_class == nullptr)
        {
            error_log("Package", "Cannot find class '%s', skip loading!", entry.class_name.c_str());
            return false;
        }

        entry.object = object_class->create_object();

        if (entry.object == nullptr)
        {
            error_log("Package", "Failed to create instance '%s'", entry.class_name.c_str());
            return false;
        }

        entry.object->name(entry.name);

        if (!entry.object->archive_process(&ar))
        {
            error_log("Package", "Failed to load object '%s'", entry.name.c_str());
            entry.object->mark_for_delete(true);
            return false;
        }

        add_object(entry.object);
        return true;
    }


    bool Package::load(BufferReader* reader, bool clean)
    {
        if (!trinex_flag(TrinexObjectFlags::IsSerializable))
        {
            error_log("Package", "Cannot load package '%s', because package is not serializable!", full_name().c_str());
            return false;
        }

        if (clean)
        {
            while (!_M_objects.empty())
            {
                (*_M_objects.begin())->mark_for_delete();
                (*_M_objects.begin())->remove_from_package();
            }
        }

        Vector<char> original_data;

        if (!load_buffer(reader, original_data))
        {
            return false;
        }

        VectorInputStream temporary_stream(original_data);
        BufferReaderWrapper<BufferReader> temporary_reader(temporary_stream);

        Archive ar(&temporary_reader);

        Vector<HeaderEntry> header;
        if (!(ar & header))
        {
            error_log("Package", "Failed to read header from file!");
            return false;
        }

        Pair<void*, BufferReader*> current_loader = {&header, &temporary_reader};

        _M_loader_data = &current_loader;

        bool result_status = true;

        for (HeaderEntry& entry : header)
        {
            if (!contains_object(entry.name))
            {
                bool loading_status = load_entry(&entry, &ar);
                result_status       = (result_status ? loading_status : result_status);
            }
        }

        Object::collect_garbage();

        _M_loader_data = nullptr;
        return result_status;
    }

    Identifier Package::add_filter(const Filter& filter)
    {
        return _M_filters.push(filter);
    }

    Package& Package::remove_filter(Identifier id)
    {
        _M_filters.remove(id);
        return *this;
    }

    const CallBacks<bool(Object*)>& Package::filters() const
    {
        return _M_filters;
    }

    bool Package::contains_object(const Object* object) const
    {
        return (object ? find_object(object->name(), false) : nullptr) != nullptr;
    }

    bool Package::contains_object(const String& name) const
    {
        return find_object(name, false) != nullptr;
    }


    Object* Package::load_object(const String& name, BufferReader* reader)
    {
        {
            Object* object = find_object(name, false);
            if (object)
            {
                return object;
            }
        }


        auto load_object_from_entry_list = [&]() -> Object* {
            Vector<HeaderEntry>& entry_list = *static_cast<Vector<HeaderEntry>*>(_M_loader_data->first);
            for (auto& entry : entry_list)
            {
                if (entry.name == name)
                {
                    Archive ar(_M_loader_data->second);
                    load_entry(&entry, &ar);
                    return entry.object;
                }
            }
            return nullptr;
        };

        if (_M_loader_data == nullptr)
        {
            Vector<char> original_data;

            if (!load_buffer(reader, original_data))
            {
                return nullptr;
            }

            VectorInputStream temporary_stream(original_data);
            BufferReaderWrapper<BufferReader> temporary_reader(temporary_stream);

            Archive ar(&temporary_reader);

            Vector<HeaderEntry> header;
            if (!(ar & header))
            {
                error_log("Package", "Failed to read header from file!");
                return nullptr;
            }

            Pair<void*, BufferReader*> current_loader = {&header, &temporary_reader};

            _M_loader_data = &current_loader;
            Object* object = load_object_from_entry_list();
            _M_loader_data = nullptr;
            return object;
        }

        return load_object_from_entry_list();
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

    Index Package::lower_bound_of(HashIndex hash) const
    {
        Object* noname  = Object::noname_object();
        noname->_M_hash = hash;
        return lower_bound_of(noname);
    }

    Index Package::lower_bound_of(const Object* object) const
    {
        auto it = std::lower_bound(_M_objects.begin(), _M_objects.end(), object, object_comparator);
        if (it == _M_objects.end())
        {
            return Constants::index_none;
        }

        return it - _M_objects.begin();
    }

    Index Package::lower_bound_of(const String& name) const
    {
        return lower_bound_of(Object::hash_of_name(name));
    }

    Index Package::validate_index(Index index, HashIndex hash) const
    {
        if (index < _M_objects.size())
            return index;
        if (_M_objects.empty())
            return 0;

        return hash > _M_objects[0]->hash_index() ? _M_objects.size() : 0;
    }


    implement_class(Package, "Engine");
    implement_initialize_class(Package)
    {}

}// namespace Engine
