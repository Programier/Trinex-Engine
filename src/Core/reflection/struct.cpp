#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/group.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine::Refl
{
	implement_reflect_type(Struct);

	Struct::Struct(Struct* parent, BitMask flags) : flags(flags), m_parent(parent)
	{
		if (parent)
		{
			parent->m_derived_structs.insert(this);
		}
	}

	Struct& Struct::construct()
	{
		Super::construct();

		if (is_scriptable() && is_native())
		{
			register_scriptable_instance();
		}
		return *this;
	}

	Struct& Struct::register_scriptable_instance()
	{
		return *this;
	}

	Struct& Struct::initialize()
	{
		Super::initialize();
		if (m_parent && !m_parent->is_initialized())
		{
			static_initialize(m_parent);
		}
		return *this;
	}

	Struct& Struct::unregister_subobject(Object* subobject)
	{
		Super::unregister_subobject(subobject);

		if (auto prop = instance_cast<Property>(subobject))
		{
			auto it = std::remove(m_properties.begin(), m_properties.end(), prop);

			if (it != m_properties.end())
				m_properties.erase(it);
		}

		return *this;
	}

	Struct& Struct::register_subobject(Object* subobject)
	{
		Super::register_subobject(subobject);

		if (auto prop = instance_cast<Property>(subobject))
		{
			m_properties.push_back(prop);
		}

		return *this;
	}

	void Struct::destroy_derived_structs()
	{
		while (!m_derived_structs.empty())
		{
			Struct* derived_struct = *m_derived_structs.begin();
			delete derived_struct;
		}
	}

	void* Struct::create_struct()
	{
		throw EngineException("Unimplemented method");
	}

	Struct& Struct::destroy_struct(void* obj)
	{
		throw EngineException("Unimplemented method");
		return *this;
	}

	size_t Struct::size() const
	{
		throw EngineException("Unimplemented method");
		return 0;
	}

	Struct* Struct::parent() const
	{
		return m_parent;
	}

	size_t Struct::abstraction_level() const
	{
		size_t level = 1;

		Struct* next = parent();

		while (next)
		{
			++level;
			next = next->parent();
		}

		return level;
	}

	Vector<Name> Struct::hierarchy(size_t offset) const
	{
		size_t level = abstraction_level();
		if (offset >= level)
			return {};

		level -= offset;

		Vector<Name> result;
		result.reserve(level);

		const Struct* current = this;
		while (current && level != 0)
		{
			result.emplace_back(current->full_name());
			current = current->parent();
			--level;
		}

		return result;
	}

	const Set<Struct*>& Struct::derived_structs() const
	{
		return m_derived_structs;
	}

	bool Struct::is_asset() const
	{
		return flags(Struct::IsAsset);
	}

	bool Struct::is_native() const
	{
		return flags(IsNative);
	}

	bool Struct::is_scriptable() const
	{
		return flags(IsScriptable);
	}

	bool Struct::is_a(const Struct* other) const
	{
		const Struct* current = this;
		while (current && current != other)
		{
			current = current->parent();
		}
		return current != nullptr;
	}

	const Vector<Property*>& Struct::properties() const
	{
		return m_properties;
	}

	class Property* Struct::find_property(StringView name)
	{
		const Struct* scope = this;

		while (scope)
		{
			if (auto prop = find<Property>(name))
			{
				return prop;
			}

			scope = scope->parent();
		}

		return nullptr;
	}

	static Vector<Property*> collect_serializable_properties(Refl::Struct* self)
	{
		Vector<Property*> result;
		result.reserve(20);

		for (auto& prop : self->properties())
		{
			if (prop->is_serializable())
			{
				result.push_back(prop);
			}
		}
		return result;
	}


	bool Struct::serialize_properties(void* object, Archive& ar)
	{
		if (ar.is_saving())
		{
			auto properties = collect_serializable_properties(this);

			size_t count = properties.size();
			ar & count;

			Vector<size_t> offsets(count + 1, 0);
			auto start_pos = ar.position();
			ar.write_data(reinterpret_cast<const byte*>(offsets.data()), offsets.size() * sizeof(size_t));

			count = 0;
			for (auto& prop : properties)
			{
				Name name      = prop->name();
				offsets[count] = ar.position() - start_pos;
				ar & name;
				prop->serialize(object, ar);
				++count;
			}

			auto end_pos   = ar.position();
			offsets[count] = end_pos - start_pos;

			ar.position(start_pos);
			ar.write_data(reinterpret_cast<const byte*>(offsets.data()), offsets.size() * sizeof(size_t));
			ar.position(end_pos);
		}
		else if (ar.is_reading())
		{
			size_t count = 0;
			ar & count;

			Vector<size_t> offsets(count + 1, 0);
			auto start_pos = ar.position();
			ar.read_data(reinterpret_cast<byte*>(offsets.data()), offsets.size() * sizeof(size_t));

			Name name;

			for (size_t i = 0; i < count; ++i)
			{
				ar.position(start_pos + offsets[i]);

				ar & name;
				Property* prop = find_property(name);

				if (prop && prop->is_serializable())
				{
					prop->serialize(object, ar);
				}
			}

			ar.position(start_pos + offsets.back());
		}

		auto scope = parent();

		if (scope)
		{
			return scope->serialize_properties(object, ar);
		}

		return ar;
	}

	bool Struct::serialize(void* object, Archive& ar)
	{
		return serialize_properties(object, ar);
	}

	Struct& Struct::group(class Group* group)
	{
		if (m_group)
		{
			m_group->remove_struct(this);
		}

		m_group = group;

		if (m_group)
		{
			m_group->add_struct(this);
		}
		return *this;
	}

	Group* Struct::group() const
	{
		return m_group;
	}

	Struct::~Struct()
	{
		m_properties.clear();

		destroy_derived_structs();

		if (m_parent)
		{
			m_parent->m_derived_structs.erase(this);
		}
	}

	static bool is_a_scriptable(const Struct* self, const Struct* other)
	{
		return self->is_a(other);
	}

	void Struct::register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast)
	{
		Super::register_layout(r, info, downcast);
		ReflectionInitializeController().require("Engine::Refl::Property").require("Engine::ScriptVector");

		using T = Struct;
		r.method("uint64 size() const", &T::size);
		r.method("uint64 abstraction_level() const", &T::parent);
		r.method("Vector<Name> hierarchy(uint64 offset = 0) const", &T::hierarchy);
		r.method("bool is_asset() const", &T::is_asset);
		r.method("bool is_native() const", &T::is_native);
		r.method("bool is_class() const", &T::is_class);
		r.method("bool is_scriptable() const", &T::is_scriptable);
		r.method("bool is_a(const Struct@ other) const", is_a_scriptable);
		r.method("const Vector<Property@>& properties() const", &T::properties);
		r.method("Property find_property(StringView name)", &T::find_property);
	}

	static void on_init()
	{
		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		auto r = ScriptClassRegistrar::reference_class("Engine::Refl::Struct", info);
		Struct::register_layout(r, Struct::static_refl_class_info(), script_downcast<Struct>);

		r.method("Struct@ parent() const", &Struct::parent);
		r.static_function("Struct@ static_find(Engine::StringView name, int flags = 0)", Struct::static_find<Struct>);
		r.static_function("Struct@ static_require(StringView name, int flags = 0)", Struct::static_require<Struct>);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Refl::Struct");
}// namespace Engine::Refl
