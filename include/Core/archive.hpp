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

		size_t position() const;
		Archive& position(size_t position);
		bool is_open() const;

		template<typename Type>
		bool operator&(Type& value)
		{
			using DecayType = std::decay_t<std::remove_pointer_t<Type>>;

			if constexpr (Concepts::is_serializable<DecayType>)
			{
				address_of(value)->serialize(*this);
			}
			else if constexpr (Concepts::is_reflected_struct<DecayType>)
			{
				serialize_struct(DecayType::static_struct_instance(), address_of(value));
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

		inline operator bool()
		{
			return m_process_status;
		}

		template<typename Type>
		typename std::enable_if<std::is_base_of_v<class Engine::Object, Type>, bool>::type serialize_reference(Type*& object)
		{
			if (is_saving())
			{
				String name = object ? object->full_name() : "";
				size_t size = name.length();
				(*this) & size;
				write_data(reinterpret_cast<const byte*>(name.data()), size);
			}
			else if (is_reading())
			{
				String name;
				size_t size;
				(*this) & size;
				name.resize(size);
				read_data(reinterpret_cast<byte*>(name.data()), size);

				if (name.empty())
				{
					object = nullptr;
				}
				else
				{
					object = reinterpret_cast<Type*>(load_object(name, Type::static_class_instance()));
				}
			}

			return *this;
		}

		template<typename Type>
		FORCE_INLINE bool process_vector(Type& vector,
		                                 bool (*on_item_processed)(typename Type::value_type&, void* userdata) = nullptr,
		                                 void* userdata                                                        = nullptr)
		{
			size_t size = vector.size();
			Archive& ar = *this;

			ar & size;
			if (ar.is_reading())
			{
				vector.resize(size);
			}

			if constexpr (std::is_fundamental_v<Type>)
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
				if (on_item_processed)
				{
					for (auto& ell : vector)
					{
						ar & ell;
						if (!on_item_processed(ell, userdata))
						{
							return ar;
						}
					}
				}
				else
				{
					for (auto& ell : vector)
					{
						ar & ell;
					}
				}
			}

			return ar;
		}

		template<typename Type>
		FORCE_INLINE bool process_set(Type& set, bool (*on_item_processed)(typename Type::value_type&, void* userdata) = nullptr,
		                              void* userdata = nullptr)
		{
			size_t size = set.size();
			Archive& ar = *this;

			ar & size;

			if (ar.is_reading())
			{
				set.clear();
				typename Type::value_type value;
				if (on_item_processed)
				{
					while (size-- > 0)
					{
						ar & value;
						on_item_processed(value, userdata);
						if (!set.insert(std::move(value)))
						{
							return ar;
						}
					}
				}
				else
				{
					while (size-- > 0)
					{
						ar & value;
						set.insert(std::move(value));
					}
				}
			}
			else if (ar.is_saving())
			{
				if (on_item_processed)
				{
					for (const typename Type::value_type& ell : set)
					{
						ar& const_cast<typename Type::value_type&>(ell);
						if (!on_item_processed(const_cast<typename Type::value_type&>(ell), userdata))
						{
							return ar;
						}
					}
				}
				else
				{
					for (const typename Type::value_type& ell : set)
					{
						ar& const_cast<typename Type::value_type&>(ell);
					}
				}
			}

			return *this;
		}


		template<typename Type>
		FORCE_INLINE bool write_map(Type& map)
		{
			size_t size = map.size();
			Archive& ar = *this;

			ar & size;

			if (ar.is_reading())
			{
				map.clear();
				std::remove_const_t<typename Type::key_type> key;
				typename Type::mapped_type value;

				while (size-- > 0)
				{
					ar & key;
					ar & value;
					map.insert_or_assign(std::move(key), std::move(value));
				}
			}
			else if (ar.is_saving())
			{
				for (auto& [key, value] : map)
				{
					ar& const_cast<typename Type::key_type&>(key);
					ar & value;
				}
			}

			return ar;
		}

		template<typename Type>
		FORCE_INLINE bool write_container(Type& container)
		{
			size_t size = container.size();
			Archive& ar = *this;

			ar & size;
			if (ar.is_reading())
			{
				container.resize(size);
			}

			for (auto& ell : container)
			{
				ar & ell;
			}

			return ar;
		}
	};

	ENGINE_EXPORT bool operator&(Archive&, String&);
}// namespace Engine
