#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_pointer.hpp>
#include <angelscript.h>

namespace Engine
{
	ScriptPointer::ScriptPointer(void* address) : m_address(address) {}

	ScriptPointer::ScriptPointer(const ScriptPointer& other) : m_address(other.m_address) {}

	ScriptPointer& ScriptPointer::operator=(const ScriptPointer& other)
	{
		if (this != &other)
			m_address = other.m_address;
		return *this;
	}

	void* ScriptPointer::address() const
	{
		return m_address;
	}

	bool ScriptPointer::is_null() const
	{
		return m_address == nullptr;
	}

	static void default_constructor(void* mem, asITypeInfo* ti)
	{
		new (mem) ScriptPointer();
	}

	static void nullptr_constructor(void* mem, asITypeInfo* ti, byte* null)
	{
		new (mem) ScriptPointer();
	}

	static void nullptr_constructor_void(void* mem, byte* null)
	{
		new (mem) ScriptPointer();
	}

	static void value_constructor(void* mem, asITypeInfo* ti, void* address)
	{
		new (mem) ScriptPointer(address);
	}

	static void copy_constructor(void* mem, asITypeInfo* ti, const ScriptPointer& ptr)
	{
		new (mem) ScriptPointer(ptr);
	}

	static ScriptPointer& assign_ptr(ScriptPointer* self, const ScriptPointer& other)
	{
		(*self) = other;
		return *self;
	}

	static ScriptPointer& assign_hndl(ScriptPointer* self, void* hndl)
	{
		new (self) ScriptPointer(hndl);
		return *self;
	}

	static ScriptPointer& assign_nullptr(ScriptPointer* self, void* null)
	{
		new (self) ScriptPointer();
		return *self;
	}

	static void* address(ScriptPointer* ptr)
	{
		void* result = ptr->address();
		if (result == nullptr)
			asGetActiveContext()->SetException("Null pointer access");
		return result;
	}

	static ScriptPointer self_return(ScriptPointer& self)
	{
		return self;
	}

	static void register_nullptr()
	{
		ScriptClassRegistrar::ValueInfo info = ScriptClassRegistrar::ValueInfo::from<byte>();
		info.pod                             = true;
		info.is_primitive                    = true;

		auto r           = ScriptClassRegistrar::value_class("NullPtr", sizeof(byte), info);
		static byte null = 0;
		ScriptEngine::instance().register_property("const NullPtr nullptr", &null);
	}

	static void on_init()
	{
		register_nullptr();

		ScriptClassRegistrar::ValueInfo info = ScriptClassRegistrar::ValueInfo::from<ScriptPointer>();
		info.align8                          = true;
		info.all_ints                        = true;
		info.template_type                   = "<T>";

		auto register_base_methods = [](ScriptClassRegistrar& r) {
			r.behave(ScriptClassBehave::Destruct, "void f()", r.destructor<ScriptPointer>);

			String assign = Strings::format("{}& opAssign(const NullPtr& other)", r.class_name());
			r.method(assign.c_str(), assign_nullptr);
			r.method("bool is_null() const", &ScriptPointer::is_null);
		};

		auto register_void_ptr = [&]() {
			info.template_type = "";
			auto r             = ScriptClassRegistrar::value_class("Ptr<void>", sizeof(ScriptPointer), info);
			r.behave(ScriptClassBehave::Construct, "void f()", r.constructor<ScriptPointer>);
			r.behave(ScriptClassBehave::Construct, "void f(const Ptr<void>& ptr)",
			         r.constructor<ScriptPointer, const ScriptPointer&>);
			r.behave(ScriptClassBehave::Construct, "void f(const NullPtr& nullptr)", nullptr_constructor_void);
			register_base_methods(r);
		};

		{
			auto r = ScriptClassRegistrar::value_class("Ptr<class T>", sizeof(ScriptPointer), info);
			register_void_ptr();
			register_base_methods(r);

			r.behave(ScriptClassBehave::Construct, "void f(int&in)", default_constructor);
			r.behave(ScriptClassBehave::Construct, "void f(int&in, const Ptr<T>& ptr)", copy_constructor);
			r.behave(ScriptClassBehave::Construct, "void f(int&in, const NullPtr& nullptr)", nullptr_constructor);
			r.behave(ScriptClassBehave::Construct, "void f(int&in, T&)", value_constructor);
			r.method("Ptr<T>& opAssign(const Ptr<T>& other)", assign_ptr);
			r.method("Ptr<T>& opAssign(T& other)", assign_hndl);
			r.method("T& get() const", address);

			r.method("Ptr<void> opConv() const", self_return);
			r.method("Ptr<void> opImplConv() const", self_return);
			r.method("Ptr<void> opCast() const", self_return);
			r.method("Ptr<void> opImplCast() const", self_return);
		}
	}

	static PreInitializeController init(on_init, "Engine::ScriptPointer");
}// namespace Engine
