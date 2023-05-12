#include <Core/class.hpp>
#include <Core/compressor.hpp>
#include <Core/config.hpp>
#include <Core/constants.hpp>
#include <Core/etl/vector_stream.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{

    struct HeaderEntry {
        ArrayIndex offset;
    };

    register_class(Engine::Package, Engine::Object)
            .register_method("add_object", &Package::add_object)
            .register_method("remove_object", &Package::remove_object);

    Package::Package()
    {}

    Package::Package(const String& _name)
    {
        name(_name);
    }

    bool Package::add_object(Object* object)
    {
        if (!object)
            return false;

        if (object->flag(ObjectFlags::OF_NeedDelete))
        {
            logger->error("Package: Cannot add object to package, wich marked for delete");
            return false;
        }

        if (object->_M_package)
            _M_package->remove_object(object);

        object->_M_package = this;
        _M_objects.insert_or_assign(object->name(), object);
        return true;
    }

    Package& Package::remove_object(Object* object)
    {
        if (!object || object->_M_package != this)
            return *this;

        object->_M_package = nullptr;
        _M_objects.erase(object->name());

        Object::root_package()->add_object(object);
        return *this;
    }

    Object* Package::get_object_by_name(const String& name) const
    {
        auto it = _M_objects.find(name);
        if (it == _M_objects.end())
            return nullptr;
        return (*it).second;
    }

    Object* Package::find_object_in_package(const String& object_name, bool recursive) const
    {
        if (!recursive)
            return get_object_by_name(object_name);

        std::size_t prev_pos           = 0;
        std::size_t pos                = 0;
        const Package* current_package = this;

        while (current_package && (pos = object_name.find_first_of("::", prev_pos)) != String::npos)
        {
            auto sub_name   = object_name.substr(prev_pos, pos - prev_pos);
            prev_pos        = pos + 2;
            Object* node    = current_package->get_object_by_name(sub_name);
            current_package = node->instance_cast<Package>();
        }

        return current_package ? current_package->get_object_by_name(
                                         object_name.substr(prev_pos, object_name.length() - prev_pos))
                               : nullptr;
    }

    const Package::ObjectMap& Package::objects() const
    {
        return _M_objects;
    }

    bool Package::can_destroy(MessageList& messages)
    {
        bool status = true;
        status      = status && Object::can_destroy(messages);

        for (auto& pair : _M_objects)
        {
            status = status && pair.second->can_destroy(messages);
        }

        return status;
    }

    bool Package::save(BufferWriter* writer) const
    {
        if (this == root_package())
        {
            logger->error("Package: Cannot save root package! Please, use different package for saving!");
            return false;
        }

        bool is_created_writer = false;
        if (writer == nullptr)
        {
            Path path = Path(engine_config.resources_dir) /
                        Path(Strings::replace_all(full_name(), "::", "/") + Constants::package_extention);

            Path dirname = FileManager::dirname_of(path);
            FileManager::root_file_manager()->create_dir(dirname);

            writer = FileManager::root_file_manager()->create_file_writer(path, true);

            if (!writer)
            {
                logger->error("Package: Failed to create file '%s'", path.c_str());
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
        if (!writer->write(flag))
        {
            logger->error("Package: Failed to write package to file!");
            return status(false);
        }

        // Creating temporary buffer
        VectorOutputStream temporary_stream;
        BufferWriterWrapper<BufferWriter> temporary_buffer(temporary_stream);


        // Creating header
        Vector<HeaderEntry> header(_M_objects.size());
        if (!temporary_buffer.write(header))
        {
            logger->error("Package: Failed to write header to file!");
            return status(false);
        }

        Index index = 0;
        for (auto& pair : _M_objects)
        {
            Object* object = pair.second;
            if (object->is_instance_of<Package>())
                continue;

            header[index++].offset = temporary_buffer.position();

            const String& class_name = object->class_instance()->name();

            if (!temporary_buffer.write(class_name))
            {
                logger->error("Package: Failed to serialize type of class!");
                return status(false);
            }

            if (!object->serialize(&temporary_buffer))
            {
                return status(false);
            }
        }

        temporary_buffer.position(0);
        if (!temporary_buffer.write(header))
        {
            logger->error("Package: Failed to update header in file!");
            return status(false);
        }

        Vector<char> result_buffer;
        Compressor::compress(temporary_stream.vector(), result_buffer);

        size_t input_size = temporary_stream.vector().size();
        if (!writer->write(input_size))
        {
            logger->error("Package: Failed to write original size to file!");
            return status(false);
        }

        writer->write(reinterpret_cast<const byte*>(result_buffer.data()), result_buffer.size());

        return status(true);
    }

    static bool check_file(BufferReader* reader)
    {
        check(reader);

        uint_t flag;
        if (!reader->read(flag))
        {
            logger->error("Package Failed to read flag to file!");
            return false;
        }

        if (flag != TRINEX_ENGINE_FLAG)
        {
            logger->error("Package: File is corrupted or is not supported!");
            return false;
        }

        return true;
    }

    bool Package::load(BufferReader* reader, bool clean)
    {
        if (this == root_package())
        {
            logger->error("Package: Cannot read root package!");
            return false;
        }

        if (clean)
        {
            while (!_M_objects.empty())
            {
                _M_objects.begin()->second->mark_for_delete();
                _M_objects.begin()->second->remove_from_package();
            }
        }

        bool is_created_writer = false;
        if (reader == nullptr)
        {
            String path = engine_config.resources_dir + Strings::replace_all(full_name(), "::", "/") +
                          Constants::package_extention;

            String dirname = FileManager::dirname_of(path);
            FileManager::root_file_manager()->create_dir(dirname);

            reader = FileManager::root_file_manager()->create_file_reader(path);

            if (!reader)
            {
                logger->error("Package: Failed to create file '%s'", path.c_str());
                return false;
            }
            is_created_writer = true;
        }

        auto status = [&is_created_writer, &reader](bool flag) -> bool {
            if (is_created_writer)
                delete reader;
            return flag;
        };

        if (!check_file(reader))
        {
            return status(false);
        }

        size_t original_size;
        if (!reader->read(original_size))
        {
            logger->error("Package: Failed to original buffer size from file!");
            return status(false);
        }

        size_t buffer_size = reader->size() - reader->position();
        Vector<char> compressed_data(buffer_size, 0);

        if (!reader->read(reinterpret_cast<byte*>(compressed_data.data()), buffer_size))
        {
            logger->error("Package: Failed to read compressed data from file!");
            return status(false);
        }

        Vector<char> original_data(original_size, 0);
        Compressor::decompress(compressed_data, original_data);


        VectorInputStream temporary_stream(original_data);
        BufferReaderWrapper<BufferReader> temporary_reader(temporary_stream);

        //        BufferReader& temporary_reader = *reader;

        Vector<HeaderEntry> header;
        if (!temporary_reader.read(header))
        {
            logger->error("Package: Failed to read header from file!");
            return status(false);
        }

        bool result_status = true;

        String class_name;
        for (HeaderEntry& entry : header)
        {
            temporary_reader.position(entry.offset);

            if (!temporary_reader.read(class_name))
            {
                logger->error("Package: Failed to read class name");
                result_status = false;
                continue;
            }

            Class* object_class = Class::find_class(class_name);
            if (object_class == nullptr)
            {
                logger->error("Package: Cannot find class '%s', skip loading!", class_name.c_str());
                result_status = false;
                continue;
            }

            Object* object = object_class->create();
            if (object == nullptr)
            {
                logger->error("Package: Failed to create instance '%s'", class_name.c_str());
                result_status = false;
                continue;
            }

            if (!object->deserialize(&temporary_reader))
            {
                object->mark_for_delete(true);
                result_status = false;
                continue;
            }

            add_object(object);
        }

        reader->offset(0, BufferSeekDir::End);
        Object::collect_garbage();

        return status(result_status);
    }

    Package::~Package()
    {
        Package* next_package = const_cast<Package*>(root_package());
        if (this == next_package)
            next_package = nullptr;

        for (auto& pair : _M_objects)
        {
            pair.second->_M_package = next_package;
        }
    }
}// namespace Engine
