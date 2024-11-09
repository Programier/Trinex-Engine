#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_pointer.hpp>
#include <angelscript.h>

namespace Engine
{
	ScriptPointer::ScriptPointer(void* address) : m_address(address)
	{}

	ScriptPointer::ScriptPointer(const ScriptPointer& other) : m_address(other.m_address)
	{}

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

		auto r = ScriptClassRegistrar::value_class("Ptr<class T>", sizeof(ScriptPointer), info);
		r.behave(ScriptClassBehave::Construct, "void f(int&in)", default_constructor);
		r.behave(ScriptClassBehave::Construct, "void f(int&in, const NullPtr& nullptr)", nullptr_constructor);
		r.behave(ScriptClassBehave::Construct, "void f(int&in, T& inout)", value_constructor);
		r.behave(ScriptClassBehave::Construct, "void f(int&in, const Ptr<T>& in ptr)", copy_constructor);
		r.behave(ScriptClassBehave::Destruct, "void f()", r.destructor<ScriptPointer>);

		r.method("Ptr<T>& opAssign(const Ptr<T>& other)", assign_ptr);
		r.method("Ptr<T>& opAssign(T& other)", assign_hndl);
		r.method("Ptr<T>& opAssign(const NullPtr& other)", assign_nullptr);
		r.method("bool is_null() const", &ScriptPointer::is_null);
		r.method("T& get() const", address);
	}

	static ScriptAddonsInitializeController init(on_init, "Engine::ScriptPointer");
}// namespace Engine
