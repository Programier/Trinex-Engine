#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/any.hpp>
#include <Core/exception.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>
#include <cstring>

namespace Engine
{
	const char* Any::bad_any_cast::what() const noexcept
	{
		return "bad any cast";
	}

	Any::Storage::Storage()
	{
		memset(&stack, 0, sizeof(stack));
	}

	Any::Manager::Manager()
	{
		reset();
	}

	void Any::Manager::reset()
	{
		destroy = nullptr;
		copy    = nullptr;
		move    = nullptr;
		swap    = nullptr;
	}

	bool Any::Manager::is_valid() const
	{
		return destroy && copy && move && swap;
	}

	Any::Any() = default;

	Any::Any(const Any& any) : m_manager(any.m_manager)
	{
		if (any.has_value())
		{
			any.m_manager->copy(any.m_storage, m_storage);
		}
	}

	Any::Any(Any&& any) : m_manager(any.m_manager)
	{
		if (any.has_value())
		{
			any.m_manager->move(any.m_storage, m_storage);
			any.m_manager = nullptr;
		}
	}

	Any& Any::operator=(const Any& any)
	{
		Any(any).swap(*this);
		return *this;
	}

	Any& Any::operator=(Any&& any)
	{
		Any(std::move(any)).swap(*this);
		return *this;
	}

	bool Any::has_value() const
	{
		return m_manager && m_manager->is_valid();
	}

	Any& Any::swap(Any& any)
	{
		if (this->m_manager != any.m_manager)
		{
			Any tmp(std::move(any));
			any.m_manager = m_manager;
			if (m_manager != nullptr)
				m_manager->move(m_storage, any.m_storage);

			m_manager = tmp.m_manager;
			if (tmp.m_manager != nullptr)
			{
				tmp.m_manager->move(tmp.m_storage, m_storage);
				tmp.m_manager = nullptr;
			}
		}
		else
		{
			if (this->m_manager != nullptr)
				this->m_manager->swap(m_storage, any.m_storage);
		}

		return *this;
	}

	Any& Any::reset()
	{
		if (has_value())
		{
			this->m_manager->destroy(m_storage);
			this->m_manager = nullptr;
		}

		return *this;
	}

	Any::~Any()
	{
		reset();
	}


	struct ScriptAny {
		template<bool is_handle>
		struct ObjectValue {
			void* data[2];

			ObjectValue(void* address, asITypeInfo* info)
			{
				data[1] = info;

				if constexpr (is_handle)
				{
					data[0] = address;
					ScriptEngine::engine()->AddRefScriptObject(address, info);
				}
				else
				{
					data[0] = ScriptEngine::engine()->CreateScriptObjectCopy(address, info);
				}
			}

			void* object()
			{
				return data[0];
			}

			asITypeInfo* info()
			{
				return reinterpret_cast<asITypeInfo*>(data[1]);
			}

			ObjectValue(const ObjectValue& value)
			{
				data[1] = value.data[1];

				if constexpr (is_handle)
				{
					data[0] = value.data[0];
					ScriptEngine::engine()->AddRefScriptObject(object(), info());
				}
				else
				{
					data[0] = ScriptEngine::engine()->CreateScriptObjectCopy(value.data[0], info());
				}
			}

			ObjectValue(ObjectValue&& value) noexcept
			{
				data[0] = value.data[0];
				data[1] = value.data[1];

				value.data[0] = nullptr;
				value.data[1] = nullptr;
			}

			~ObjectValue()
			{
				if (data[0] && data[1])
					ScriptEngine::engine()->ReleaseScriptObject(object(), info());
			}
		};

		static_assert(Any::is_stack_type<ObjectValue<true>>, "Object Value is not stack type!");

		static Any& opAssign(Any* self, const Any& any)
		{
			return (*self) = any;
		}

		static void opAssignValue(Any& any, void* value, int_t type_id)
		{
			if (ScriptEngine::is_primitive_type(type_id))
			{
				ScriptVariable var(value, type_id);

				switch (type_id)
				{
					case asTYPEID_BOOL:
						any = var.bool_value();
						break;
					case asTYPEID_INT8:
						any = var.int8_value();
						break;
					case asTYPEID_INT16:
						any = var.int16_value();
						break;
					case asTYPEID_INT32:
						any = var.int32_value();
						break;
					case asTYPEID_INT64:
						any = var.int64_value();
						break;
					case asTYPEID_UINT8:
						any = var.uint8_value();
						break;
					case asTYPEID_UINT16:
						any = var.uint16_value();
						break;
					case asTYPEID_UINT32:
						any = var.uint32_value();
						break;
					case asTYPEID_UINT64:
						any = var.uint64_value();
						break;
					case asTYPEID_FLOAT:
						any = var.float_value();
						break;
					case asTYPEID_DOUBLE:
						any = var.double_value();
						break;
					default:
						break;
				}
			}
			else if (ScriptEngine::is_handle_type(type_id))
			{
				auto engine = ScriptEngine::engine();
				auto info   = engine->GetTypeInfoById(type_id);
				any         = ObjectValue<true>(*reinterpret_cast<void**>(value), info);
			}
			else if (ScriptEngine::is_object_type(type_id))
			{
				auto engine = ScriptEngine::engine();
				auto info   = engine->GetTypeInfoById(type_id);
				any         = ObjectValue<false>(value, info);
			}
		}

		static bool get(Any& any, void* result, int_t type_id)
		{
			if (!any.has_value())
				return false;

			if (ScriptEngine::is_primitive_type(type_id))
			{
				switch (type_id)
				{
					case asTYPEID_BOOL:
						*reinterpret_cast<bool*>(result) = any.cast<bool>();
						break;
					case asTYPEID_INT8:
						*reinterpret_cast<int8_t*>(result) = any.cast<int8_t>();
						break;
					case asTYPEID_INT16:
						*reinterpret_cast<int16_t*>(result) = any.cast<int16_t>();
						break;
					case asTYPEID_INT32:
						*reinterpret_cast<int32_t*>(result) = any.cast<int32_t>();
						break;
					case asTYPEID_INT64:
						*reinterpret_cast<int64_t*>(result) = any.cast<int64_t>();
						break;
					case asTYPEID_UINT8:
						*reinterpret_cast<uint8_t*>(result) = any.cast<uint8_t>();
						break;
					case asTYPEID_UINT16:
						*reinterpret_cast<uint16_t*>(result) = any.cast<uint16_t>();
						break;
					case asTYPEID_UINT32:
						*reinterpret_cast<uint32_t*>(result) = any.cast<uint32_t>();
						break;
					case asTYPEID_UINT64:
						*reinterpret_cast<uint64_t*>(result) = any.cast<uint64_t>();
						break;
					case asTYPEID_FLOAT:
						*reinterpret_cast<float*>(result) = any.cast<float>();
						break;
					case asTYPEID_DOUBLE:
						*reinterpret_cast<double*>(result) = any.cast<double>();
						break;
					default:
						break;
				}

				return true;
			}
			else if (ScriptEngine::is_handle_type(type_id))
			{
				auto obj                            = any.cast<ObjectValue<true>>();
				(*reinterpret_cast<void**>(result)) = obj.object();
				obj.data[0] = obj.data[1] = nullptr;
				return true;
			}
			else if (ScriptEngine::is_object_type(type_id))
			{
				void* src   = any.cast<void*>();
				auto engine = ScriptEngine::engine();
				engine->AssignScriptObject(result, src, engine->GetTypeInfoById(type_id));
				return true;
			}

			return false;
		}

		static void opCast(Any& any, void* result, int_t type_id)
		{
			if (!get(any, result, type_id))
			{
				asGetActiveContext()->SetException("Failed to cast value!");
			}
		}

		static void constructor(void* self, void* value, int_t type_id)
		{
			opAssignValue(*new (self) Any(), value, type_id);
		}
	};

	static void initialize()
	{
		auto info                    = ScriptClassRegistrar::ValueInfo();
		info.is_class                = true;
		info.has_constructor         = true;
		info.has_destructor          = true;
		info.has_copy_constructor    = true;
		info.has_assignment_operator = true;
		info.more_constructors       = true;

		auto reg = ScriptClassRegistrar::value_class("Engine::Any", sizeof(Any), info);
		reg.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Any>);
		reg.behave(ScriptClassBehave::Construct, "void f(const Any& in any)", ScriptClassRegistrar::constructor<Any, const Any&>);
		reg.behave(ScriptClassBehave::Construct, "void f(const ?& in value)", ScriptAny::constructor);
		reg.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Any>);
		reg.method("Any& opAssign(const Any& in)", ScriptAny::opAssign);
		reg.method("Any& opAssign(const ?& in)", ScriptAny::opAssignValue);
		reg.method("Any& reset()", &Any::reset);
		reg.method("bool has_value() const", &Any::has_value);
		reg.method("Any& swap(const Any& in any)", &Any::swap);
		reg.method("bool get(?& out value)", &ScriptAny::get);
		reg.method("void opConv(?& out)", &ScriptAny::opCast);
	}

	static ReflectionInitializeController on_init(initialize);
}// namespace Engine
