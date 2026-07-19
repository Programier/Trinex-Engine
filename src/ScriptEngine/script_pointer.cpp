#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_pointer.hpp>
#include <angelscript.h>

namespace Trinex
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

	static void nullptr_constructor(void* mem, asITypeInfo* ti, u8* null)
	{
		new (mem) ScriptPointer();
	}

	static void nullptr_constructor_void(void* mem, u8* null)
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
		auto r         = ScriptBinding::Class::create("NullPtr", ScriptBinding::value_type<u8>(ScriptClassFlags::Pod));
		static u8 null = 0;
		ScriptEngine::instance().register_property("const NullPtr nullptr", &null);
	}

	trinex_on_pre_init({.name = "Trinex::ScriptPointer"})
	{
		register_nullptr();

		auto flags = ScriptClassFlags::AppClassAllInts | ScriptClassFlags::AppClassAlign8;

		auto register_base_methods = [](ScriptBinding::Class& r) {
			r.destructor<ScriptPointer>();

			String assign = Strings::format("{}& opAssign(const NullPtr& other)", r.name());
			r.method(assign.c_str(), assign_nullptr);
			r.method("bool is_null() const", &ScriptPointer::is_null);
		};

		auto register_void_ptr = [&]() {
			auto r = ScriptBinding::Class::create("Ptr<void>", ScriptBinding::value_type<ScriptPointer>(flags));
			r.constructor<ScriptPointer>();
			r.constructor<ScriptPointer, const ScriptPointer&>("void f(const Ptr<void>& ptr)");
			r.behaviour(ScriptClassBehave::Construct, "void f(const NullPtr& nullptr)", nullptr_constructor_void);
			register_base_methods(r);
		};

		{
			auto r = ScriptBinding::Class::create("Ptr<T>",
			                                      ScriptBinding::value_type<ScriptPointer>(flags | ScriptClassFlags::Template));
			register_void_ptr();
			register_base_methods(r);

			r.behaviour(ScriptClassBehave::Construct, "void f(int&in)", default_constructor);
			r.behaviour(ScriptClassBehave::Construct, "void f(int&in, const Ptr<T>& ptr)", copy_constructor);
			r.behaviour(ScriptClassBehave::Construct, "void f(int&in, const NullPtr& nullptr)", nullptr_constructor);
			r.behaviour(ScriptClassBehave::Construct, "void f(int&in, T&)", value_constructor);
			r.method("Ptr<T>& opAssign(const Ptr<T>& other)", assign_ptr);
			r.method("Ptr<T>& opAssign(T& other)", assign_hndl);
			r.method("T& get() const", address);

			r.method("Ptr<void> opConv() const", self_return);
			r.method("Ptr<void> opImplConv() const", self_return);
			r.method("Ptr<void> opCast() const", self_return);
			r.method("Ptr<void> opImplCast() const", self_return);
		}
	}
}// namespace Trinex
