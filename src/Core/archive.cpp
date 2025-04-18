#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/object.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/struct.hpp>

namespace Engine
{
	Archive::Archive() : m_reader(nullptr), m_is_saving(false), m_process_status(false) {}

	Archive::Archive(BufferReader* reader) : m_is_saving(false)
	{
		m_reader = reader;
		if (reader == nullptr)
		{
			throw EngineException("Archive: Reader can't be nullptr!");
		}

		m_process_status = m_reader->is_open();
	}

	Archive::Archive(BufferWriter* writer) : m_is_saving(true)
	{
		m_writer = writer;
		if (writer == nullptr)
		{
			throw EngineException("Archive: Writer can't be nullptr!");
		}

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

	class Object* Archive::load_object(const StringView& name, class Refl::Class* self)
	{
		Object* object = Object::load_object(name);
		if (object && object->class_instance()->is_a(self))
			return object;
		return nullptr;
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

	Archive& Archive::write_data(const byte* data, size_t size)
	{
		if (is_saving())
		{
			m_writer->write(data, size);
		}

		return *this;
	}

	Archive& Archive::read_data(byte* data, size_t size)
	{
		if (is_reading())
		{
			m_reader->read(data, size);
		}

		return *this;
	}

	Archive& Archive::serialize_memory(byte* data, size_t size)
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

	size_t Archive::position() const
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

	Archive& Archive::position(size_t position)
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

	bool Archive::serialize_string(String& str)
	{
		size_t size = str.length();
		serialize(size);

		if (is_reading())
		{
			str.resize(size);
			read_data(reinterpret_cast<byte*>(str.data()), size);
		}
		else if (is_saving())
		{
			write_data(reinterpret_cast<byte*>(str.data()), size);
		}
		return *this;
	}
}// namespace Engine
