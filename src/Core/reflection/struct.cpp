#include <Core/reflection/function.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>

namespace Trinex::Refl
{
	namespace
	{
		class TextReader
		{
		private:
			StringView m_value;
			usize m_position = 0;

		public:
			TextReader(StringView value) : m_value(value) {}

			void skip_whitespace()
			{
				while (m_position < m_value.size())
				{
					const char character = m_value[m_position];

					if (character != ' ' && character != '\t' && character != '\r' && character != '\n')
						break;

					++m_position;
				}
			}

			bool consume(char character)
			{
				skip_whitespace();

				if (m_position == m_value.size() || m_value[m_position] != character)
					return false;

				++m_position;
				return true;
			}

			bool read_identifier(StringView& value)
			{
				skip_whitespace();

				if (m_position == m_value.size())
					return false;

				auto is_identifier_start = [](char character) {
					return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || character == '_';
				};
				auto is_identifier_character = [is_identifier_start](char character) {
					return is_identifier_start(character) || (character >= '0' && character <= '9');
				};

				const usize begin = m_position;

				if (!is_identifier_start(m_value[m_position]))
					return false;

				while (m_position < m_value.size() && is_identifier_character(m_value[m_position]))
					++m_position;

				value = m_value.substr(begin, m_position - begin);
				return true;
			}

			StringView remaining() const { return m_value.substr(m_position); }
			usize position() const { return m_position; }
			bool advance(usize count)
			{
				if (count == 0 || count > m_value.size() - m_position)
					return false;

				m_position += count;
				return true;
			}
		};

		bool is_text_serializable(const Property* property)
		{
			return !property->is_read_only() && !property->is_transient();
		}

		bool format_struct_properties(const Struct* type, const void* self, String& value, bool& has_property)
		{
			for (auto& base : type->bases())
			{
				if (!format_struct_properties(base.value(), self, value, has_property))
				{
					return false;
				}
			}

			for (const Ref<Object>& member : type->members())
			{
				Property* property = member->cast<Property>();

				if (property == nullptr || !is_text_serializable(property))
					continue;

				const void* field = property->load(self);

				if (field == nullptr)
					return false;

				if (has_property)
					value += ", ";

				value += property->name().to_string();
				value += " = ";

				if (!property->type()->format(field, value))
					return false;

				has_property = true;
			}

			return true;
		}
	} // namespace

	namespace Native
	{
		usize parse_struct(const Type* type, void* self, StringView value)
		{
			TextReader reader(value);

			if (!reader.consume('{'))
				return 0;

			reader.skip_whitespace();

			if (reader.consume('}'))
				return reader.position();

			for (;;)
			{
				StringView name;

				if (!reader.read_identifier(name) || !reader.consume('='))
					return 0;

				Property* property = type->find<Property>(name);

				if (property == nullptr || !is_text_serializable(property))
					return 0;

				reader.skip_whitespace();
				void* field       = property->address(self);
				const usize count = field ? property->type()->parse(field, reader.remaining()) : 0;

				if (!reader.advance(count))
					return 0;

				reader.skip_whitespace();

				if (reader.consume('}'))
					return reader.position();

				if (!reader.consume(','))
					return 0;
			}
		}

		bool format_struct(const Type* type, const void* self, String& value)
		{
			const Struct* struct_type = static_cast<const Struct*>(type);
			value += "{ ";

			bool has_property = false;

			if (!format_struct_properties(struct_type, self, value, has_property))
				return false;

			if (has_property)
				value += ' ';

			value += '}';
			return true;
		}
	} // namespace Native

	Struct::Struct(Name name, u32 size, u32 alignment, u64 flags, Array<Ref<Struct>> bases, const VTable* vtable, Object* owner)
	    : Type(name, size, alignment, flags, vtable), m_bases(etl::move(bases))
	{
		if (owner)
		{
			Object::owner(owner);
		}
	}

	bool Struct::register_child(Object* object)
	{
		if (object == nullptr || object == this || m_members.contains(object->name()))
			return false;

		for (Object* scope = this; scope != nullptr;)
		{
			if (scope == object)
				return false;

			Ref<Object> parent = scope->owner();
			scope              = parent.value();
		}

		m_members.insert(retain_ref(object));
		return true;
	}

	bool Struct::unregister_child(Object* object)
	{
		if (object == nullptr)
			return false;

		if (auto it = m_members.find(object->name()); it != m_members.end())
		{
			m_members.erase(it);
			return true;
		}

		return false;
	}

	Object* Struct::find(Name name, const Class* base) const
	{
		if (auto it = m_members.find(name); it != m_members.end())
		{
			if (base)
			{
				return it->value()->is(base) ? it->value() : nullptr;
			}
			else
			{
				return it->value();
			}
		}

		for (auto& entry : m_bases)
		{
			if (Object* object = entry->find(name, base))
			{
				return object;
			}
		}

		return nullptr;
	}

	Object* Struct::resolve(const Class* base)
	{
		if (Object* object = Type::resolve(base))
		{
			return object;
		}

		for (auto& entry : m_bases)
		{
			if (Object* object = entry->resolve(base))
			{
				return object;
			}
		}

		return nullptr;
	}

	const Object* Struct::resolve(const Class* base) const
	{
		if (const Object* object = Type::resolve(base))
		{
			return object;
		}

		for (auto& entry : m_bases)
		{
			if (Object* object = entry->resolve(base))
			{
				return object;
			}
		}

		return nullptr;
	}

	bool Struct::is_child_of(const Struct* other) const
	{
		if (this == other)
		{
			return true;
		}

		for (auto& entry : m_bases)
		{
			if (entry->is_child_of(other))
			{
				return true;
			}
		}

		return false;
	}
} // namespace Trinex::Refl
