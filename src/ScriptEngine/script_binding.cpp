#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Trinex::ScriptBinding
{
	namespace
	{
		class ScopedNamespace final
		{
			String m_prev_namespace;

		public:
			ScopedNamespace(const String& namespace_name) : m_prev_namespace(ScriptEngine::default_namespace())
			{
				ScriptEngine::default_namespace(namespace_name);
			}

			~ScopedNamespace() { ScriptEngine::default_namespace(m_prev_namespace); }
		};

		static asDWORD create_call_conv(const FunctionPointer& func, ScriptCallConv conv, bool is_method = false)
		{
			if (conv == ScriptCallConv::Auto)
			{
				// 1 = generic, 2 = global func, 3 = method
				switch (func.reference().flag)
				{
					case 1: return asCALL_GENERIC;
					case 2: return is_method ? asCALL_CDECL_OBJFIRST : asCALL_CDECL;
					case 3: return is_method ? asCALL_THISCALL : asCALL_CDECL;
					default: trinex_unreachable_fmt("Unsupported script function flag '%d'!", func.reference().flag);
				}
			}

			switch (conv)
			{
				case ScriptCallConv::CDecl: return asCALL_CDECL;
				case ScriptCallConv::StdCall: return asCALL_STDCALL;
				case ScriptCallConv::ThisCallAsGlobal: return asCALL_THISCALL_ASGLOBAL;
				case ScriptCallConv::ThisCall: return asCALL_THISCALL;
				case ScriptCallConv::CDeclObjLast: return asCALL_CDECL_OBJLAST;
				case ScriptCallConv::CDeclObjFirst: return asCALL_CDECL_OBJFIRST;
				case ScriptCallConv::Generic: return asCALL_GENERIC;
				case ScriptCallConv::ThisCall_ObjLast: return asCALL_THISCALL_OBJLAST;
				case ScriptCallConv::ThisCall_ObjFirst: return asCALL_THISCALL_OBJFIRST;
				default: trinex_unreachable(); return asCALL_CDECL;
			}
		}

		static asEBehaviours create_behaviour(ScriptClassBehave behave)
		{
			switch (behave)
			{
				case ScriptClassBehave::Construct: return asBEHAVE_CONSTRUCT;
				case ScriptClassBehave::ListConstruct: return asBEHAVE_LIST_CONSTRUCT;
				case ScriptClassBehave::Destruct: return asBEHAVE_DESTRUCT;
				case ScriptClassBehave::Factory: return asBEHAVE_FACTORY;
				case ScriptClassBehave::ListFactory: return asBEHAVE_LIST_FACTORY;
				case ScriptClassBehave::AddRef: return asBEHAVE_ADDREF;
				case ScriptClassBehave::Release: return asBEHAVE_RELEASE;
				case ScriptClassBehave::GetWeakRefFlag: return asBEHAVE_GET_WEAKREF_FLAG;
				case ScriptClassBehave::TemplateCallback: return asBEHAVE_TEMPLATE_CALLBACK;
				case ScriptClassBehave::GetRefCount: return asBEHAVE_GETREFCOUNT;
				case ScriptClassBehave::GetGCFlag: return asBEHAVE_GETGCFLAG;
				case ScriptClassBehave::SetGCFlag: return asBEHAVE_SETGCFLAG;
				case ScriptClassBehave::EnumRefs: return asBEHAVE_ENUMREFS;
				case ScriptClassBehave::ReleaseRefs: return asBEHAVE_RELEASEREFS;
				default: trinex_unreachable(); return asBEHAVE_CONSTRUCT;
			}
		}

		static String normalize_type_name(String name, StringView suffix)
		{
			if (suffix.empty())
				return name;

			auto pos = name.find_first_of('<');
			if (pos == String::npos)
			{
				name += suffix;
				return name;
			}

			name = name.substr(0, pos);
			name += suffix;
			return name;
		}
	}// namespace

	static_assert(sizeof(FunctionPointer) >= sizeof(asSFuncPtr));
	static_assert(alignof(FunctionPointer) >= alignof(asSFuncPtr));

	FunctionPointer::FunctionPointer(FunctionPtr ptr, Private)
	{
		reference() = asFunctionPtr(ptr);
	}

	FunctionPointer::FunctionPointer(GenericPtr ptr)
	{
		reference() = asFunctionPtr(ptr);
	}

	FunctionPointer::FunctionPointer(MethodPtr ptr, Private)
	{
		reference() = asSMethodPtr<sizeof(ptr)>::Convert(ptr);
	}

	ObjectTypeOptions value_type_options(usize size, ScriptClassFlags flags, StringView name_suffix)
	{
		return ObjectTypeOptions{.size = size, .flags = flags | ScriptClassFlags::Value, .name_suffix = String(name_suffix)};
	}

	ObjectTypeOptions reference_type_options(usize size, ScriptClassFlags flags, StringView name_suffix)
	{
		return ObjectTypeOptions{.size = size, .flags = flags | ScriptClassFlags::Ref, .name_suffix = String(name_suffix)};
	}

	Class::Class(const StringView& name)
	    : m_class(name), m_class_base(Strings::class_name_sv_of(name)), m_namespace(Strings::namespace_sv_of(name))
	{}

	Class& Class::apply_name_suffix(StringView suffix)
	{
		m_class      = normalize_type_name(m_class, suffix);
		m_class_base = normalize_type_name(m_class_base, suffix);
		return *this;
	}

	Class Class::create(const StringView& name, const ObjectTypeOptions& options)
	{
		Class binding(name);
		binding.apply_name_suffix(options.name_suffix);
		ScopedNamespace ns(binding.m_namespace);

		auto engine = ScriptEngine::engine();
		trinex_verify(engine->RegisterObjectType(binding.m_class_base.c_str(), options.size, options.flags) >= 0);
		return binding;
	}

	Class Class::existing(const StringView& name)
	{
		return Class(name);
	}

	Class Class::existing(Refl::Class* class_instance)
	{
		return existing(class_instance->full_name());
	}

	Class Class::reflected(Refl::Class* class_instance, ScriptClassFlags flags)
	{
		auto binding = create(class_instance->full_name(), reference_type_options(class_instance->size(), flags));
		auto info    = binding.type_info();
		class_instance->script_type_info = info;
		ScriptEngine::register_class(info.type_id(), class_instance);
		return binding;
	}

	ScriptTypeInfo Class::type_info() const
	{
		ScopedNamespace ns(m_namespace);
		return ScriptEngine::type_info_by_name(m_class_base.c_str());
	}

	i32 Class::type_id() const
	{
		return type_info().type_id();
	}

	Class& Class::behaviour(ScriptClassBehave behaviour, const char* decl, const FunctionPointer& func, ScriptCallConv conv,
	                        void* auxiliary)
	{
		ScopedNamespace ns(m_namespace);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterObjectBehaviour(m_class_base.c_str(), create_behaviour(behaviour), decl, func,
		                                              create_call_conv(func, conv, true), auxiliary) >= 0);
		return *this;
	}

	Class& Class::method(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		ScopedNamespace ns(m_namespace);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterObjectMethod(m_class_base.c_str(), decl, func, create_call_conv(func, conv, true),
		                                           auxiliary) >= 0);
		return *this;
	}

	Class& Class::static_function(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		ScopedNamespace ns(m_class.c_str());
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterGlobalFunction(decl, func, create_call_conv(func, conv, false), auxiliary) >= 0);
		return *this;
	}

	Class& Class::property(const char* decl, usize offset)
	{
		ScopedNamespace ns(m_namespace);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterObjectProperty(m_class_base.c_str(), decl, offset) >= 0);
		return *this;
	}

	Class& Class::static_property(const char* decl, void* property)
	{
		ScopedNamespace ns(m_class);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterGlobalProperty(decl, property) >= 0);
		return *this;
	}

	Class& Class::constructor(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		return behaviour(ScriptClassBehave::Construct, decl, func, conv, auxiliary);
	}

	Class& Class::destructor(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		return behaviour(ScriptClassBehave::Destruct, decl, func, conv, auxiliary);
	}

	Class& Class::factory(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		return behaviour(ScriptClassBehave::Factory, decl, func, conv, auxiliary);
	}

	Class& Class::addref(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		return behaviour(ScriptClassBehave::AddRef, decl, func, conv, auxiliary);
	}

	Class& Class::release(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		return behaviour(ScriptClassBehave::Release, decl, func, conv, auxiliary);
	}

	Class& Class::template_callback(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		return behaviour(ScriptClassBehave::TemplateCallback, decl, func, conv, auxiliary);
	}

	Class& Class::funcdef(const String& decl)
	{
		ScopedNamespace ns(m_class);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterFuncdef(decl.c_str()) >= 0);
		return *this;
	}

	Enum::Enum(const StringView& namespace_name, const StringView& base_name, bool init)
	    : m_base(base_name), m_namespace(namespace_name)
	{
		if (init)
		{
			ScopedNamespace ns(m_namespace);
			auto engine = ScriptEngine::engine();

			trinex_verify(engine->RegisterEnum(m_base.c_str()) >= 0);
		}
	}

	Enum::Enum(const StringView& full_name, bool init)
	    : Enum(Strings::namespace_sv_of(full_name), Strings::class_name_sv_of(full_name), init)
	{}

	Enum& Enum::value(const char* name, i64 enum_value)
	{
		ScopedNamespace ns(m_namespace);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterEnumValue(m_base.c_str(), name, enum_value) >= 0);
		return *this;
	}

	ScriptTypeInfo Enum::type_info() const
	{
		ScopedNamespace ns(m_namespace);
		return ScriptEngine::type_info_by_name(m_base.c_str());
	}

	i32 Enum::type_id() const
	{
		return type_info().type_id();
	}

	Namespace::Namespace(const StringView& namespace_name) : m_namespace(namespace_name) {}

	const String& Namespace::name() const
	{
		return m_namespace;
	}

	Namespace Namespace::nested(const StringView& suffix) const
	{
		if (m_namespace.empty())
			return Namespace(suffix);

		return Namespace(Strings::concat_scoped_name(m_namespace, suffix));
	}

	Namespace& Namespace::property(const char* decl, void* property)
	{
		ScopedNamespace ns(m_namespace);
		ScriptEngine::register_property(decl, property);
		return *this;
	}

	Namespace& Namespace::func_def(const char* decl)
	{
		ScopedNamespace ns(m_namespace);
		ScriptEngine::register_funcdef(decl);
		return *this;
	}

	Namespace& Namespace::type_def(const char* new_type_name, const char* type)
	{
		ScopedNamespace ns(m_namespace);
		ScriptEngine::register_typedef(new_type_name, type);
		return *this;
	}

	Class Namespace::object_type(const StringView& name, const ObjectTypeOptions& options) const
	{
		auto full_name = m_namespace.empty() ? String(name) : Strings::concat_scoped_name(m_namespace, name);
		return Class::create(full_name, options);
	}

	Class Namespace::existing_class(const StringView& name) const
	{
		auto full_name = m_namespace.empty() ? String(name) : Strings::concat_scoped_name(m_namespace, name);
		return Class::existing(full_name);
	}

	Enum Namespace::enum_type(const StringView& name, bool init) const
	{
		return Enum(m_namespace, name, init);
	}

	Namespace& Namespace::function(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary)
	{
		ScopedNamespace ns(m_namespace);
		auto engine = ScriptEngine::engine();

		trinex_verify(engine->RegisterGlobalFunction(decl, func, create_call_conv(func, conv, false), auxiliary) >= 0);
		return *this;
	}

	Namespace global()
	{
		return Namespace("");
	}

	Namespace in(const StringView& namespace_name)
	{
		return Namespace(namespace_name);
	}
}// namespace Trinex::ScriptBinding
