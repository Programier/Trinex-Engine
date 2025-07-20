#pragma once
#include <Core/enums.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>

namespace Engine
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
		class Object* load_object(const StringView& name, class Refl::Class* self);

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

		Archive& write_data(const byte* data, size_t size);
		Archive& read_data(byte* data, size_t size);
		Archive& serialize_memory(byte* data, size_t size);

		size_t position() const;
		Archive& position(size_t position);
		bool is_open() const;

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
				size_t size = sizeof(Type);
				byte* data  = reinterpret_cast<byte*>(&value);

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

		inline operator bool() { return m_process_status; }

		template<typename Type>
		typename std::enable_if<std::is_base_of_v<class Engine::Object, Type>, bool>::type serialize_reference(Type*& object)
		{
			if (is_saving())
			{
				String name = object ? object->full_name() : "";
				size_t size = name.length();
				serialize(size);
				write_data(reinterpret_cast<const byte*>(name.data()), size);
			}
			else if (is_reading())
			{
				String name;
				size_t size;
				serialize(size);
				name.resize(size);
				read_data(reinterpret_cast<byte*>(name.data()), size);

				if (name.empty())
				{
					object = nullptr;
				}
				else
				{
					object = reinterpret_cast<Type*>(load_object(name, Type::static_reflection()));
				}
			}

			return *this;
		}

		template<typename Type>
		FORCE_INLINE bool serialize_vector(Type& vector)
		{
			size_t size = vector.size();
			Archive& ar = *this;
			serialize(size);

			if (ar.is_reading())
			{
				vector.resize(size);
			}

			if constexpr (std::is_trivially_copyable_v<Type>)
			{
				byte* data   = reinterpret_cast<byte*>(vector.data());
				size_t bytes = size * sizeof(Type);

				if (ar.is_reading())
				{
					ar.read_data(data, bytes);
				}
				else if (ar.is_saving())
				{
					ar.write_data(data, bytes);
				}
			}
			else
			{
				for (auto& ell : vector)
				{
					serialize(ell);
				}
			}

			return ar;
		}

		template<typename Type>
		FORCE_INLINE bool serialize_set(Type& set)
		{
			size_t size = set.size();
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
		FORCE_INLINE bool serialize_map(Type& map)
		{
			size_t size = map.size();
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
		FORCE_INLINE bool serialize_container(Type& container)
		{
			size_t size = container.size();
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

		bool serialize_string(String& value);
	};

	template<>
	inline bool Archive::serialize<String>(String& value)
	{
		return serialize_string(value);
	}
}// namespace Engine
