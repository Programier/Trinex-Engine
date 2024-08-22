#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	static FORCE_INLINE Vector<Class*>& get_asset_class_table()
	{
		static Vector<Class*> vector;
		return vector;
	}

	Class::Class(const Name& name, Class* parent, BitMask _flags) : Struct(name, parent)
	{
		m_size = 0;
		info_log("Class", "Created class instance '%s'", this->name().c_str());
		flags               = _flags;
		m_singletone_object = nullptr;
		m_destroy_func      = GarbageCollector::destroy_internal;

		if (is_asset())
		{
			get_asset_class_table().push_back(this);
		}

		if (parent)
		{
			parent->m_childs.insert(this);
		}
	}

	void Class::on_create_call(Object* object) const
	{
		if (Class* parent_class = parent())
		{
			parent_class->on_create_call(object);
		}

		on_create(object);
	}

	Class* Class::parent() const
	{
		return reinterpret_cast<Class*>(Struct::parent());
	}

	const Class::ChildsSet& Class::child_classes() const
	{
		return m_childs;
	}

	void (*Class::destroy_func() const)(Object*)
	{
		return m_destroy_func;
	}

	Class& Class::destroy_func(void (*func)(Object*))
	{
		trinex_always_check(func, "Destroy func can't be nullptr");
		m_destroy_func = func;
		return *this;
	}

	void* Class::create_struct() const
	{
		return create_object();
	}

	Object* Class::create_object(StringView name, Object* owner, const Class* class_overload) const
	{
		if (class_overload)
		{
			trinex_check(class_overload->is_a(this), "Overload class must be instance of this class");
		}
		else
		{
			class_overload = this;
		}

		if (flags(Class::IsSingletone))
		{
			if (m_singletone_object == nullptr)
			{
				Object::setup_next_object_info(const_cast<Class*>(this));
				m_singletone_object = m_static_constructor(const_cast<Class*>(this), name, owner);
				Object::reset_next_object_info();
				on_create_call(m_singletone_object);
			}

			return m_singletone_object;
		}

		Object::setup_next_object_info(const_cast<Class*>(this));
		Object* object = m_static_constructor(const_cast<Class*>(this), name, owner);

		on_create_call(object);
		return object;
	}

	Object* Class::create_placement_object(void* place, StringView name, Object* owner, const Class* class_overload) const
	{
		if (class_overload)
		{
			trinex_check(class_overload->is_a(this), "Overload class must be instance of this class");
		}
		else
		{
			class_overload = this;
		}

		if (flags(Class::IsSingletone))
		{
			if (m_singletone_object == nullptr)
			{
				Object::setup_next_object_info(const_cast<Class*>(class_overload));
				m_singletone_object = m_static_placement_constructor(const_cast<Class*>(this), place, name, owner);
				class_overload->on_create_call(m_singletone_object);
				Object::reset_next_object_info();
				return m_singletone_object;
			}

			return nullptr;
		}

		Object::setup_next_object_info(const_cast<Class*>(class_overload));
		Object* object = m_static_placement_constructor(const_cast<Class*>(this), place, name, owner);
		class_overload->on_create_call(object);
		return object;
	}

	size_t Class::sizeof_class() const
	{
		return m_size;
	}

	Class* Class::static_find(const StringView& name, bool required)
	{
		Struct* self = Struct::static_find(name, required);
		if (self && self->is_class())
		{
			return reinterpret_cast<Class*>(self);
		}

		return nullptr;
	}

	bool Class::is_scriptable() const
	{
		return flags(IsScriptable);
	}

	Class& Class::static_constructor(Object* (*new_static_constructor)(Class*, StringView, Object*) )
	{
		trinex_always_check(new_static_constructor, "Constructor can't be nullptr!");
		m_static_constructor = new_static_constructor;
		return *this;
	}

	Class& Class::static_placement_constructor(Object* (*new_static_placement_constructor)(Class*, void*, StringView, Object*) )
	{
		trinex_always_check(new_static_placement_constructor, "Constructor can't be nullptr!");
		m_static_placement_constructor = new_static_placement_constructor;
		return *this;
	}

	Object* Class::singletone_instance() const
	{
		return m_singletone_object;
	}

	Class& Class::post_initialize()
	{
		if (is_scriptable())
		{
			register_scriptable_class();
		}
		return *this;
	}

	bool Class::is_asset() const
	{
		return flags(Class::IsAsset);
	}

	bool Class::is_class() const
	{
		return true;
	}

	bool Class::is_native() const
	{
		return flags(IsNative);
	}

	const Vector<Class*>& Class::asset_classes()
	{
		return get_asset_class_table();
	}

	Class::~Class()
	{
		on_class_destroy(this);

		destroy_childs();

		if (Class* parent_class = parent())
		{
			parent_class->m_childs.erase(this);
		}
	}

	static void on_init()
	{

		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		ScriptClassRegistrar::reference_class("Engine::Class", info);

		ScriptBindingsInitializeController().push([]() {
			auto reg = ScriptClassRegistrar::existing_class("Engine::Class");
			reg.method("Class@ parent() const", &Class::parent);
			reg.method("const Name& name() const", &Class::name);
			reg.method("const Name& namespace_name() const", &Class::namespace_name);
			reg.method("const Name& base_name() const", &Class::base_name);
			reg.static_function("Class@ static_find(const StringView& in)", Class::static_find);
			reg.method("bool is_a(const Class@) const", method_of<bool, const Struct*>(&Struct::is_a));
			reg.method("uint64 sizeof_class() const", &Class::sizeof_class);
			reg.method("bool is_scriptable() const", &Class::is_scriptable);
			reg.method("Object@ singletone_instance() const", &Class::singletone_instance);
			reg.method("bool is_asset() const", &Class::is_asset);
			reg.method("bool is_native() const", &Class::is_native);
		});
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Class");
}// namespace Engine
