#pragma once
#include <Core/enums.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>

namespace Trinex
{
	class BufferReader;
	class BufferWriter;

	class ENGINE_EXPORT Archive
	{
	private:
		union
		{
			BufferReader* m_reader;
			BufferWriter* m_writer;
		};

		bool m_is_saving      = false;
		bool m_process_status = true;

		template<typename Type>
		static auto address_of(Type& value)
		{
			if constexpr (std::is_pointer_v<Type>)
			{
				return value;
			}
			else
			{
				return std::addressof(value);
			}
		}

		bool serialize_struct(Refl::Struct* self, void* obj);

	public:
		Flags<SerializationFlags> flags;

		Archive();
		Archive(BufferReader* reader);
		Archive(BufferWriter* writer);
		Archive(const Archive&) = delete;
		Archive(Archive&&);
		Archive& operator=(const Archive&) = delete;
		Archive& operator=(Archive&&);

		bool is_saving() const;
		bool is_reading() const;

		BufferReader* reader() const;
		BufferWriter* writer() const;

		Archive& write_data(const u8* data, usize size);
		Archive& read_data(u8* data, usize size);
		Archive& serialize_memory(u8* data, usize size);

		usize position() const;
		Archive& position(usize position);
		bool is_open() const;

		bool begin_chunk(u32& offset);
		bool end_chunk(u32 offset);

		bool serialize_string(String& value);
		bool serialize_object(Object*& object, StringView name = "", Object* owner = nullptr);
		bool serialize_object_ref(Object*& object);

		inline operator bool() { return m_process_status; }

		template<typename Type>
		bool serialize(Type& value)
		{
			using DecayType = std::decay_t<std::remove_pointer_t<Type>>;

			if constexpr (Concepts::is_serializable<DecayType>)
			{
				return address_of(value)->serialize(*this);
			}
			else if constexpr (Concepts::is_serializable<Serializer<DecayType>, DecayType&>)
			{
				Serializer<DecayType> serializer;
				return serializer.serialize(*this, *address_of(value));
			}
			else if constexpr (Concepts::is_reflected_struct<DecayType>)
			{
				return serialize_struct(DecayType::static_reflection(), address_of(value));
			}
			else
			{
				usize size = sizeof(Type);
				u8* data   = reinterpret_cast<u8*>(&value);

				if (is_reading())
				{
					read_data(data, size);
				}
				else if (is_saving())
				{
					write_data(data, size);
				}
			}

			return *this;
		}

		template<typename... Types>
		bool serialize(Types&... values)
		{
			return (serialize(values) && ...);
		}

		template<typename T>
		bool serialize_object(T*& object, StringView name = "", Object* owner = nullptr)
		{
			Object* self = object;
			bool valid   = serialize_object(self, name, owner);

			if (valid && is_reading())
			{
				object = static_cast<T*>(self);
			}

			return valid;
		}

		template<typename T>
		bool serialize_object_ref(T*& object)
		{
			Object* self = object;
			bool valid   = serialize_object_ref(self);

			if (valid && is_reading())
			{
				object = static_cast<T*>(self);
			}

			return valid;
		}

		template<typename T>
		inline bool serialize_childs(Object* owner, T** begin = nullptr, T** end = nullptr)
		{
			u32 size = end - begin;
			serialize(size);

			if (m_is_saving)
			{
				while (begin != end)
				{
					T* instance = *(begin++);

					u32 offset;
					begin_chunk(offset);
					serialize_object(instance);
					end_chunk(offset);
				}
			}
			else
			{
				T* instance = nullptr;

				while (size--)
				{
					u32 offset;
					begin_chunk(offset);
					serialize_object(instance, "", owner);
					end_chunk(offset);
				}
			}

			return *this;
		}

		template<typename Type>
		inline bool serialize_vector(Type& vector)
		{
			usize size  = vector.size();
			Archive& ar = *this;
			serialize(size);

			if (ar.is_reading())
			{
				vector.resize(size);
			}

			for (auto& ell : vector)
			{
				serialize(ell);
			}

			return ar;
		}

		template<typename Type>
		inline bool serialize_memory(Type& vector)
		{
			u32 size    = vector.size();
			Archive& ar = *this;
			serialize(size);

			if (ar.is_reading())
			{
				vector.resize(size);
			}

			u8* data    = reinterpret_cast<u8*>(vector.data());
			usize bytes = size * sizeof(typename Type::value_type);
			return serialize_memory(data, bytes);
		}

		template<typename Type>
		inline bool serialize_set(Type& set)
		{
			usize size  = set.size();
			Archive& ar = *this;

			serialize(size);

			if (ar.is_reading())
			{
				set.clear();
				typename Type::value_type value;

				while (size-- > 0)
				{
					serialize(value);
					set.insert(std::move(value));
				}
			}
			else if (ar.is_saving())
			{
				for (const typename Type::value_type& ell : set)
				{
					serialize(const_cast<typename Type::value_type&>(ell));
				}
			}

			return *this;
		}

		template<typename Type>
		inline bool serialize_map(Type& map)
		{
			usize size = map.size();
			serialize(size);

			if (is_reading())
			{
				map.clear();
				std::remove_const_t<typename Type::key_type> key;
				typename Type::mapped_type value;

				while (size-- > 0)
				{
					serialize(key);
					serialize(value);
					map.insert_or_assign(std::move(key), std::move(value));
				}
			}
			else if (is_saving())
			{
				for (auto& [key, value] : map)
				{
					serialize(const_cast<typename Type::key_type&>(key));
					serialize(value);
				}
			}

			return *this;
		}

		template<typename Type>
		inline bool serialize_container(Type& container)
		{
			usize size = container.size();
			serialize(size);
			if (is_reading())
			{
				container.resize(size);
			}

			for (auto& ell : container)
			{
				serialize(ell);
			}

			return *this;
		}
	};

	template<>
	inline bool Archive::serialize<String>(String& value)
	{
		return serialize_string(value);
	}
}// namespace Trinex
