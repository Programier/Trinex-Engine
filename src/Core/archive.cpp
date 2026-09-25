#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/etl/span.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/stream.hpp>
#include <Core/types/path.hpp>

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

	Archive::Archive() : m_stream(nullptr), m_mode(IOMode::Read), m_flags(ArchiveFlags::Undefined) {}

	Archive::Archive(PathView file, IOMode mode, ArchiveFlags flags) {}

	Archive::Archive(Ref<Stream> stream, IOMode mode, ArchiveFlags flags) : m_stream(stream), m_mode(mode), m_flags(flags)
	{
		trinex_assert(stream);
	}

	Archive::Archive(void* memory, usize size, IOMode mode, ArchiveFlags flags)
	    : Archive(Ref<MemoryStream>::make(memory, size), mode, flags)
	{}

	bool Archive::serialize_struct(Refl::Struct* self, void* obj)
	{
		return self->serialize(obj, *this);
	}

	bool Archive::is_saving() const
	{
		return m_mode == IOMode::Write;
	}

	bool Archive::is_reading() const
	{
		return m_mode == IOMode::Read;
	}

	Archive& Archive::write_data(const u8* data, usize size)
	{
		if (is_saving())
		{
			stream()->write(data, size);
		}

		return *this;
	}

	Archive& Archive::read_data(u8* data, usize size)
	{
		if (is_reading())
		{
			stream()->read(data, size);
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
		return stream()->offset();
	}

	Archive& Archive::position(usize position)
	{
		stream()->offset(position);
		return *this;
	}

	bool Archive::is_open() const
	{
		return m_stream != nullptr;
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
				trinex_error(Log::Core, "Cannot load object. Class '%s' not found!", hierarchy.front().c_str());
				return false;
			}

			object = self->create_object();

			if (object == nullptr)
			{
				trinex_error(Log::Core, "Cannot create object of class '%s'!", hierarchy.front().c_str());
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
				trinex_error(Log::Core, "Failed to load object");
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
