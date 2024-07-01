#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/file_flag.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
    struct HeaderEntry {
        Vector<Index> class_hierarchy;
        size_t offset            = 0;
        size_t uncompressed_size = 0;
        Index object_name        = 0;

        Vector<byte> compressed_data;
    };

    struct Header {
        Vector<String> names;
        Vector<HeaderEntry> entries;
        size_t header_size;
        size_t header_begin_offset;


        FORCE_INLINE Class* find_class(HeaderEntry* entry)
        {
            Class* instance = nullptr;
            for (Index index : entry->class_hierarchy)
            {
                if ((instance = Class::static_find(names[index], false)))
                {
                    return instance;
                }
            }
            return instance;
        }


        Object* preload_object(Package* pkg, HeaderEntry* entry)
        {
            Class* object_class = find_class(entry);
            Object* object      = nullptr;

            if (object_class)
            {
                if (!(object = pkg->find_object(names[entry->object_name], false)))
                {
                    object = object_class->create_object();
                    object->name(names[entry->object_name]);

                    pkg->add_object(object);
                    object->preload();
                }
            }

            return object;
        }

        void preload(Package* pkg, Archive& ar)
        {
            for (auto& entry : entries)
            {
                preload_object(pkg, &entry);
            }
        }

        Object* load(HeaderEntry* entry, Package* pkg, Archive& ar)
        {
            Vector<byte> compressed_buffer;
            Vector<byte> uncompressed_buffer;

            VectorReader uncompressed_reader = &uncompressed_buffer;
            Archive uncompressed_ar          = &uncompressed_reader;

            Object* object = preload_object(pkg, entry);

            if (object)
            {
                ar.reader()->position(header_begin_offset + header_size + entry->offset);
                ar & compressed_buffer;

                uncompressed_buffer.resize(entry->uncompressed_size);
                Compressor::decompress(compressed_buffer, uncompressed_buffer);

                uncompressed_reader.VectorReaderBase::position(0);
                object->archive_process(uncompressed_ar);
                object->postload();
            }
            return object;
        }

        size_t index_of(const StringView& name)
        {
            auto it = std::find_if(names.begin(), names.end(), [name](const String& ell) { return name == ell; });
            if (it == names.end())
            {
                names.emplace_back(name);
                return names.size() - 1;
            }

            return it - names.begin();
        }

        Header& build(const Package* package)
        {
            size_t current_entry = 0;
            auto& objects        = package->objects();
            entries.resize(objects.size());

            size_t offset = 0;
            for (auto& [name, object] : objects)
            {
                HeaderEntry& entry = entries[current_entry];
                entry.object_name  = index_of(object->name());

                auto hierarchy = object->class_instance()->hierarchy(1);// Skip Object class
                entry.class_hierarchy.reserve(hierarchy.size());

                for (const String& name : hierarchy)
                {
                    entry.class_hierarchy.push_back(index_of(name));
                }

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
                entry.offset = offset;
                offset += entry.compressed_data.size() + sizeof(size_t);
            }

            current_entry = entries.size() - current_entry;

            if (current_entry > 0)
            {
                entries.resize(entries.size() - current_entry);
            }

            return *this;
        }

        bool archive_process(Archive& ar, bool (*callback)(HeaderEntry& entry, void*) = nullptr, void* userdata = nullptr)
        {
            header_begin_offset = ar.position();

            ar & header_size;
            Buffer uncompressed;
            Buffer compressed;

            if (ar.is_saving())
            {
                VectorWriter writer = &uncompressed;
                Archive buffer_ar   = &writer;

                buffer_ar.process_vector(names);
                buffer_ar.process_vector(entries, callback, userdata);

                Compressor::compress(uncompressed, compressed);
                size_t size = uncompressed.size();

                ar & size;
                ar & compressed;
            }
            else if (ar.is_reading())
            {
                size_t size;
                ar & size;
                ar & compressed;

                uncompressed.resize(size);
                Compressor::decompress(compressed, uncompressed);

                VectorReader reader = &uncompressed;
                Archive buffer_ar   = &reader;

                buffer_ar.process_vector(names);
                buffer_ar.process_vector(entries, callback, userdata);
            }

            if (ar.is_saving())
            {
                header_size = ar.position() - header_begin_offset;
                ar.position(header_begin_offset);
                ar & header_size;
                ar.position(header_begin_offset + header_size);
            }

            return ar;
        }
    };

    ENGINE_EXPORT bool operator&(Archive& ar, HeaderEntry& entry)
    {
        ar & entry.class_hierarchy;
        ar & entry.offset;
        ar & entry.uncompressed_size;
        ar & entry.object_name;

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

        if (!autorename && contains_object(object->m_name))
        {
            error_log("Package", "Cannot add object to package. Object with name '%s' already exist in package!",
                      object->string_name().c_str());
            return false;
        }

        if (object->m_package)
        {
            object->m_package->remove_object(object);
        }

        {
            static size_t index    = 2;
            String new_object_name = object->m_name;
            while (contains_object(new_object_name))
            {
                new_object_name = object->m_name.to_string() + Strings::format(" {}", index++);
            }
            if (new_object_name != object->m_name.to_string())
                object->name(new_object_name);
        }

        object->m_package = this;
        m_objects.insert_or_assign(object->name(), object);
        return true;
    }

    Package& Package::remove_object(Object* object)
    {
        if (!object || object->m_package != this)
            return *this;

        m_objects.erase(object->name());
        object->m_package = nullptr;
        return *this;
    }

    Object* Package::find_object_private_no_recurse(const StringView& _name) const
    {
        Name object_name = _name;
        auto it          = m_objects.find(object_name);
        if (it == m_objects.end())
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

        return package ? package->find_object_private_no_recurse(_name) : nullptr;
    }

    Object* Package::find_object(const StringView& object_name, bool recursive) const
    {
        if (recursive)
            return find_object_private(object_name);
        return find_object_private_no_recurse(object_name);
    }

    const Package::ObjectMap& Package::objects() const
    {
        return m_objects;
    }

    bool Package::contains_object(const Object* object) const
    {
        return object ? object->package() == this : false;
    }

    bool Package::is_engine_resource() const
    {
        return true;
    }

    bool Package::contains_object(const StringView& name) const
    {
        return find_object(name, false) != nullptr;
    }

    bool Package::save(BufferWriter* writer, Flags<SerializationFlags> serialization_flags)
    {
        if (!flags(Object::IsSerializable))
        {
            error_log("Package", "Cannot save non-serializable package!");
            return false;
        }

        for (auto& [name, object] : m_objects)
        {
            if (!object->is_instance_of<Package>())
            {
                object->save(writer, serialization_flags);
            }
        }
        return false;
    }

    Package::~Package()
    {
        for (auto& [name, object] : m_objects)
        {
            object->m_package = nullptr;
        }
    }

    static void bind_to_script(ScriptClassRegistrar* registrar, Class* self)
    {
        registrar->method("bool add_object(Object@, bool = false)", &Package::add_object)
                .method("Package@ remove_object(Object@)", &Package::remove_object)
                .method("Object@ find_object(const StringView& in, bool=false) const",
                        method_of<Object*, const StringView&, bool>(&Package::find_object))
                /* .method("bool contains_object(const Object@) const",
                        method_of<bool, Package, const Object*>(&Package::contains_object))
                .method("bool contains_object(const string& in) const",
                        method_of<bool, Package, const String&>(&Package::contains_object))*/
                ;
    }

    implement_engine_class(Package, Class::IsScriptable)
    {
        static_class_instance()->set_script_registration_callback(bind_to_script);
    }
}// namespace Engine
