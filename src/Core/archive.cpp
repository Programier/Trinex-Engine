#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/struct.hpp>

namespace Trinex
{
	static FORCE_INLINE Refl::Class* find_class(const Vector<Name>& hierarchy)
	{
		Refl::Class* instance = nullptr;

		for (Name name : hierarchy)
		{
			if ((instance = Refl::Class::static_find(name)))
			{
				return instance;
			}
		}

		return instance;
	}

	Archive::Archive() : m_reader(nullptr), m_is_saving(false), m_process_status(false) {}

	Archive::Archive(BufferReader* reader) : m_is_saving(false)
	{
		trinex_assert(reader);

		m_reader         = reader;
		m_process_status = m_reader->is_open();
	}

	Archive::Archive(BufferWriter* writer) : m_is_saving(true)
	{
		trinex_assert(writer);

		m_writer         = writer;
		m_process_status = m_writer->is_open();
	}

	Archive::Archive(Archive&& other)
	{
		(*this) = std::move(other);
	}

	Archive& Archive::operator=(Archive&& other)
	{
		if (this == &other)
			return *this;

		m_reader         = other.m_reader;
		m_process_status = other.m_process_status;
		m_is_saving      = other.m_is_saving;

		other.m_process_status = false;
		other.m_reader         = nullptr;
		other.m_is_saving      = false;

		return *this;
	}

	bool Archive::serialize_struct(Refl::Struct* self, void* obj)
	{
		return self->serialize(obj, *this);
	}

	bool Archive::is_saving() const
	{
		return m_is_saving && m_writer;
	}

	bool Archive::is_reading() const
	{
		return !m_is_saving && m_reader;
	}

	BufferReader* Archive::reader() const
	{
		return m_is_saving ? nullptr : m_reader;
	}

	BufferWriter* Archive::writer() const
	{
		return m_is_saving ? m_writer : nullptr;
	}

	Archive& Archive::write_data(const u8* data, usize size)
	{
		if (is_saving())
		{
			m_writer->write(data, size);
		}

		return *this;
	}

	Archive& Archive::read_data(u8* data, usize size)
	{
		if (is_reading())
		{
			m_reader->read(data, size);
		}

		return *this;
	}

	Archive& Archive::serialize_memory(u8* data, usize size)
	{
		if (is_reading())
		{
			return read_data(data, size);
		}

		if (is_saving())
		{
			return write_data(data, size);
		}
		return *this;
	}

	usize Archive::position() const
	{
		if (is_saving())
		{
			return writer()->position();
		}
		else if (is_reading())
		{
			return reader()->position();
		}

		return 0;
	}

	Archive& Archive::position(usize position)
	{
		if (is_saving())
		{
			writer()->position(position);
		}
		else if (is_reading())
		{
			reader()->position(position);
		}
		return *this;
	}

	bool Archive::is_open() const
	{
		if (is_saving())
		{
			return writer()->is_open();
		}
		else if (is_reading())
		{
			return reader()->is_open();
		}

		return false;
	}

	bool Archive::begin_chunk(u32& offset)
	{
		if (is_saving())
			offset = position();

		return serialize(offset);
	}

	bool Archive::end_chunk(u32 offset)
	{
		if (is_saving())
		{
			u32 end     = position();
			bool status = position(offset).serialize(end);
			position(end);
			return status;
		}
		else
		{
			position(offset);
		}

		return *this;
	}

	bool Archive::serialize_string(String& str)
	{
		usize size = str.length();
		serialize(size);

		if (is_reading())
		{
			str.resize(size);
			read_data(reinterpret_cast<u8*>(str.data()), size);
		}
		else if (is_saving())
		{
			write_data(reinterpret_cast<u8*>(str.data()), size);
		}
		return *this;
	}

	bool Archive::serialize_object(Object*& object, StringView name, Object* owner)
	{
		if (is_saving())
		{
			auto hierarchy = object->class_instance()->hierarchy(1);
			serialize(hierarchy);
			return object->serialize(*this);
		}
		else
		{
			Vector<Name> hierarchy;
			serialize(hierarchy);

			Refl::Class* self = find_class(hierarchy);

			if (self == nullptr)
			{
				error_log("Archive", "Cannot load object. Class '%s' not found!", hierarchy.front().c_str());
				return false;
			}

			object = self->create_object();

			if (object == nullptr)
			{
				error_log("Archive", "Cannot create object of class '%s'!", hierarchy.front().c_str());
				return false;
			}

			if (name.empty())
			{
				object->owner(owner);
			}
			else
			{
				object->rename(name, owner);
			}

			object->preload();
			bool valid = object->serialize(*this);

			if (!valid)
			{
				error_log("Object", "Failed to load object");
				trx_delete object;
				object = nullptr;
			}
			else
			{
				object->postload();
			}
			return *this;
		}
	}

	bool Archive::serialize_object_ref(Object*& object)
	{
		if (is_saving())
		{
			String name = object ? object->full_name() : "";
			usize size  = name.length();
			serialize(size);
			write_data(reinterpret_cast<const u8*>(name.data()), size);
		}
		else if (is_reading())
		{
			String name;
			usize size;
			serialize(size);
			name.resize(size);
			read_data(reinterpret_cast<u8*>(name.data()), size);

			if (name.empty())
			{
				object = nullptr;
			}
			else
			{
				object = Object::load_object(name);
			}
		}

		return *this;
	}
}// namespace Trinex
