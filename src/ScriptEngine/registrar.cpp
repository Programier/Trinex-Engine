#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/reflection/class.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{
#define SCRIPT_CHECK(call) trinex_always_check(call, #call)

	static FORCE_INLINE asIScriptEngine* script_engine()
	{
		return ScriptEngine::engine();
	}

	static BitMask create_flags(const ScriptClassRegistrar::BaseInfo& info)
	{
		BitMask result = info.extra_flags;

		if (!info.template_type.empty())
			result |= asOBJ_TEMPLATE;

		return result;
	}

	static BitMask create_flags(const ScriptClassRegistrar::ValueInfo& info)
	{
		BitMask result = asOBJ_VALUE | create_flags(static_cast<const ScriptClassRegistrar::BaseInfo&>(info));

		if (info.as_handle)
			result |= asOBJ_ASHANDLE;

		if (info.all_ints)
			result |= asOBJ_APP_CLASS_ALLINTS;

		if (info.all_floats)
			result |= asOBJ_APP_CLASS_ALLFLOATS;

		if (info.pod)
			result |= asOBJ_POD;

		if (!info.template_type.empty())
			result |= asOBJ_TEMPLATE;

		if (info.is_array)
			result |= asOBJ_APP_ARRAY;

		if (info.is_class)
			result |= asOBJ_APP_CLASS;

		if (info.is_array)
			result |= asOBJ_APP_ARRAY;

		if (info.is_float)
			result |= asOBJ_APP_FLOAT;

		if (info.is_primitive)
			result |= asOBJ_APP_PRIMITIVE;

		if (info.has_constructor)
			result |= asOBJ_APP_CLASS_CONSTRUCTOR;

		if (info.more_constructors)
			result |= asOBJ_APP_CLASS_MORE_CONSTRUCTORS;

		if (info.has_assignment_operator)
			result |= asOBJ_APP_CLASS_ASSIGNMENT;

		if (info.has_copy_constructor)
			result |= asOBJ_APP_CLASS_COPY_CONSTRUCTOR;

		if (info.has_destructor)
			result |= asOBJ_APP_CLASS_DESTRUCTOR;

		if (info.align8)
			result |= asOBJ_APP_CLASS_ALIGN8;

		return result;
	}

	static BitMask create_flags(const ScriptClassRegistrar::RefInfo& info)
	{
		BitMask result = asOBJ_REF | create_flags(static_cast<const ScriptClassRegistrar::BaseInfo&>(info));

		if (info.no_count)
			result |= asOBJ_NOCOUNT;

		if (info.implicit_handle && script_engine()->GetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES) != 0)
			result |= asOBJ_IMPLICIT_HANDLE;

		return result;
	}

	static asDWORD create_call_conv(ScriptCallConv conv)
	{
		switch (conv)
		{
			case ScriptCallConv::CDecl:
				return asCALL_CDECL;
			case ScriptCallConv::StdCall:
				return asCALL_STDCALL;
			case ScriptCallConv::ThisCallAsGlobal:
				return asCALL_THISCALL_ASGLOBAL;
			case ScriptCallConv::ThisCall:
				return asCALL_THISCALL;
			case ScriptCallConv::CDeclObjLast:
				return asCALL_CDECL_OBJLAST;
			case ScriptCallConv::CDeclObjFirst:
				return asCALL_CDECL_OBJFIRST;
			case ScriptCallConv::Generic:
				return asCALL_GENERIC;
			case ScriptCallConv::ThisCall_ObjLast:
				return asCALL_THISCALL_OBJLAST;
			case ScriptCallConv::ThisCall_ObjFirst:
				return asCALL_THISCALL_OBJFIRST;
			default:
				throw EngineException("Undefined call convension!");
		}
	}

	static asEBehaviours create_behaviour(ScriptClassBehave behave)
	{
		switch (behave)
		{
			case ScriptClassBehave::Construct:
				return asBEHAVE_CONSTRUCT;
			case ScriptClassBehave::ListConstruct:
				return asBEHAVE_LIST_CONSTRUCT;
			case ScriptClassBehave::Destruct:
				return asBEHAVE_DESTRUCT;
			case ScriptClassBehave::Factory:
				return asBEHAVE_FACTORY;
			case ScriptClassBehave::ListFactory:
				return asBEHAVE_LIST_FACTORY;
			case ScriptClassBehave::AddRef:
				return asBEHAVE_ADDREF;
			case ScriptClassBehave::Release:
				return asBEHAVE_RELEASE;
			case ScriptClassBehave::GetWeakRefFlag:
				return asBEHAVE_GET_WEAKREF_FLAG;
			case ScriptClassBehave::TemplateCallback:
				return asBEHAVE_TEMPLATE_CALLBACK;
			case ScriptClassBehave::GetRefCount:
				return asBEHAVE_GETREFCOUNT;
			case ScriptClassBehave::GetGCFlag:
				return asBEHAVE_GETGCFLAG;
			case ScriptClassBehave::SetGCFlag:
				return asBEHAVE_SETGCFLAG;
			case ScriptClassBehave::EnumRefs:
				return asBEHAVE_ENUMREFS;
			case ScriptClassBehave::ReleaseRefs:
				return asBEHAVE_RELEASEREFS;
			case ScriptClassBehave::GetTypeInfo:
				return asBEHAVE_GET_TYPE_INFO;
			default:
				throw EngineException("Undefined behave!");
		}
	}


	ScriptClassRegistrar::BaseInfo::BaseInfo() : template_type(""), extra_flags(0)
	{}

	ScriptClassRegistrar::ValueInfo::ValueInfo()
	    : as_handle(false), pod(false), all_ints(false), all_floats(false), align8(false), more_constructors(false),
	      is_class(true), is_array(false), is_float(false), is_primitive(false), has_constructor(false), has_destructor(false),
	      has_assignment_operator(false), has_copy_constructor(false)
	{}

	ScriptClassRegistrar::RefInfo::RefInfo() : no_count(true), implicit_handle(true)
	{}

	ScriptClassRegistrar::ScriptClassRegistrar(const StringView& name)
	    : m_class_name(name), m_class_base_name(Strings::class_name_sv_of(name)), m_namespace_name(Strings::namespace_sv_of(name))
	{}

	ScriptClassRegistrar::ScriptClassRegistrar(const StringView& name, size_t size, BitMask flags) : ScriptClassRegistrar(name)
	{
		ScriptNamespaceScopedChanger changer(m_namespace_name);
		script_engine()->RegisterObjectType(m_class_base_name.c_str(), size, flags);
	}

	ScriptClassRegistrar& ScriptClassRegistrar::modify_name_if_template(const BaseInfo& info)
	{
		if (!info.template_type.empty())
		{
			m_class_name      = m_class_name.substr(0, m_class_name.find_first_of('<'));
			m_class_base_name = m_class_base_name.substr(0, m_class_base_name.find_first_of('<'));
			m_class_name += info.template_type;
			m_class_base_name += info.template_type;
		}
		return *this;
	}

	ScriptClassRegistrar ScriptClassRegistrar::value_class(const StringView& name, size_t size, const ValueInfo& info)
	{
		return ScriptClassRegistrar(name, size, create_flags(info)).modify_name_if_template(info);
	}

	ScriptClassRegistrar ScriptClassRegistrar::reference_class(const StringView& name, const RefInfo& info, size_t size)
	{
		return ScriptClassRegistrar(name, size, create_flags(info)).modify_name_if_template(info);
	}

	ScriptClassRegistrar ScriptClassRegistrar::reference_class(Refl::Class* class_instance)
	{
		ScriptClassRegistrar::RefInfo info;
		info.no_count        = true;
		info.implicit_handle = true;
		info.extra_flags     = asOBJ_APP_NATIVE_INHERITANCE;

		auto res = ScriptClassRegistrar(class_instance->full_name(), class_instance->size(), create_flags(info));
		auto ti  = res.type_info();
		class_instance->script_type_info = ti;
		ti.info()->SetNativeClassUserData(class_instance);
		return res;
	}

	ScriptClassRegistrar ScriptClassRegistrar::existing_class(const String& name)
	{
		return ScriptClassRegistrar(name);
	}

	ScriptClassRegistrar ScriptClassRegistrar::existing_class(Refl::Class* class_instance)
	{
		return existing_class(class_instance->full_name());
	}

	const String& ScriptClassRegistrar::class_name() const
	{
		return m_class_name;
	}

	const String& ScriptClassRegistrar::class_base_name() const
	{
		return m_class_base_name;
	}

	const String& ScriptClassRegistrar::namespace_name() const
	{
		return m_namespace_name;
	}

	ScriptTypeInfo ScriptClassRegistrar::type_info() const
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		return ScriptEngine::type_info_by_name(m_class_base_name);
	}

	ScriptFunction ScriptClassRegistrar::method(const char* declaration, ScriptMethodPtr* method, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		int r = script_engine()->RegisterObjectMethod(m_class_base_name.c_str(), declaration,
		                                              *reinterpret_cast<asSFuncPtr*>(method), create_call_conv(conv));
		SCRIPT_CHECK(r >= 0);
		return script_engine()->GetFunctionById(r);
	}

	ScriptFunction ScriptClassRegistrar::method(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		int r = script_engine()->RegisterObjectMethod(m_class_base_name.c_str(), declaration,
		                                              *reinterpret_cast<asSFuncPtr*>(function), create_call_conv(conv));
		SCRIPT_CHECK(r >= 0);
		return script_engine()->GetFunctionById(r);
	}

	ScriptFunction ScriptClassRegistrar::static_function(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_class_name);
		return ScriptEngine::register_function(declaration, function, conv);
	}

	ScriptClassRegistrar& ScriptClassRegistrar::property(const char* declaration, size_t offset)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		SCRIPT_CHECK(script_engine()->RegisterObjectProperty(m_class_base_name.c_str(), declaration, offset) >= 0);
		return *this;
	}

	ScriptClassRegistrar& ScriptClassRegistrar::static_property(const char* declaration, void* prop)
	{
		ScriptNamespaceScopedChanger ns(m_class_name);
		SCRIPT_CHECK(script_engine()->RegisterGlobalProperty(declaration, prop) >= 0);
		return *this;
	}

	ScriptClassRegistrar& ScriptClassRegistrar::behave(ScriptClassBehave behaviour, const char* declaration,
	                                                   ScriptFuncPtr* function, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		SCRIPT_CHECK(script_engine()->RegisterObjectBehaviour(m_class_base_name.c_str(), create_behaviour(behaviour), declaration,
		                                                      *reinterpret_cast<asSFuncPtr*>(function),
		                                                      create_call_conv(conv)) >= 0);
		return *this;
	}

	ScriptClassRegistrar& ScriptClassRegistrar::behave(ScriptClassBehave behaviour, const char* declaration,
	                                                   ScriptMethodPtr* method, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		SCRIPT_CHECK(script_engine()->RegisterObjectBehaviour(m_class_base_name.c_str(), create_behaviour(behaviour), declaration,
		                                                      *reinterpret_cast<asSFuncPtr*>(method),
		                                                      create_call_conv(conv)) >= 0);
		return *this;
	}


	ScriptEnumRegistrar::ScriptEnumRegistrar(const StringView& namespace_name, const StringView& base_name, bool init)
	    : m_enum_base_name(base_name), m_enum_namespace_name(namespace_name)
	{
		if (init)
		{
			prepare_namespace();

			SCRIPT_CHECK(script_engine()->RegisterEnum(m_enum_base_name.c_str()) >= 0);
			release_namespace();
		}
	}

	ScriptClassRegistrar& ScriptClassRegistrar::opfunc(const char* declaration, ScriptMethodPtr* method, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		SCRIPT_CHECK(script_engine()->RegisterObjectMethod(m_class_base_name.c_str(), declaration,
		                                                   *reinterpret_cast<asSFuncPtr*>(method), create_call_conv(conv)) >= 0);
		return *this;
	}

	ScriptClassRegistrar& ScriptClassRegistrar::opfunc(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv)
	{
		ScriptNamespaceScopedChanger ns(m_namespace_name);
		SCRIPT_CHECK(script_engine()->RegisterObjectMethod(m_class_base_name.c_str(), declaration,
		                                                   *reinterpret_cast<asSFuncPtr*>(function),
		                                                   create_call_conv(conv)) >= 0);
		return *this;
	}

	ScriptEnumRegistrar::ScriptEnumRegistrar(const StringView& full_name, bool init)
	    : ScriptEnumRegistrar(Strings::namespace_of(full_name), Strings::class_name_of(full_name), init)
	{}

	ScriptEnumRegistrar& ScriptEnumRegistrar::prepare_namespace()
	{
		m_current_namespace = script_engine()->GetDefaultNamespace();
		SCRIPT_CHECK(script_engine()->SetDefaultNamespace(m_enum_namespace_name.c_str()) >= 0);
		return *this;
	}

	ScriptEnumRegistrar& ScriptEnumRegistrar::release_namespace()
	{
		SCRIPT_CHECK(script_engine()->SetDefaultNamespace(m_current_namespace.c_str()) >= 0);
		return *this;
	}

	ScriptEnumRegistrar& ScriptEnumRegistrar::set(const char* name, int_t value)
	{
		prepare_namespace();
		SCRIPT_CHECK(script_engine()->RegisterEnumValue(m_enum_base_name.c_str(), name, value) >= 0);
		release_namespace();
		return *this;
	}
}// namespace Engine
