#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_handle.hpp>
#include <angelscript.h>

namespace Engine
{
	ScriptPointer::ScriptPointer()
	{}

	ScriptPointer* ScriptPointer::create(asITypeInfo* ti)
	{
		return create(ti, nullptr);
	}

	ScriptPointer* ScriptPointer::create(asITypeInfo* ti, void* address)
	{
		ScriptPointer* handle = new ScriptPointer();
		handle->m_address	  = address;
		handle->m_type_id	  = ti->GetSubTypeId();
		handle->m_refs		  = 1;
		handle->m_gc_flag	  = false;
		return handle;
	}

	void ScriptPointer::add_ref() const
	{
		++m_refs;
	}

	void ScriptPointer::release() const
	{
		--m_refs;
		if (m_refs == 0)
			delete this;
	}

	void* ScriptPointer::address() const
	{
		return m_address;
	}

	int_t ScriptPointer::type_id() const
	{
		return m_type_id;
	}

	bool ScriptPointer::is_null() const
	{
		return m_address == nullptr;
	}

	static void on_init()
	{
		ScriptClassRegistrar::RefInfo info;
		info.no_count		 = false;
		info.implicit_handle = false;
		info.template_type	 = "<T>";

		ScriptClassRegistrar registrar = ScriptClassRegistrar::reference_class("Ptr<class T>", info);
		registrar.behave(ScriptClassBehave::Factory, "Ptr<T>@ f(int& in)",
						 func_of<ScriptPointer*(asITypeInfo*)>(ScriptPointer::create), ScriptCallConv::CDecl);
		registrar.behave(ScriptClassBehave::Factory, "Ptr<T>@ f(int& in, T& inout)",
						 func_of<ScriptPointer*(asITypeInfo*, void* address)>(ScriptPointer::create),
						 ScriptCallConv::CDecl);

		registrar.behave(ScriptClassBehave::AddRef, "void f() const", &ScriptPointer::add_ref);
		registrar.behave(ScriptClassBehave::Release, "void f() const", &ScriptPointer::release);
		registrar.method("bool is_null() const", &ScriptPointer::is_null);
		registrar.method("T& get() const", &ScriptPointer::address);
	}

	static ReflectionInitializeController init(on_init, "Engine::ScriptPointer");
}// namespace Engine
