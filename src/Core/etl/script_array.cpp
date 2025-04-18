#include <Core/etl/script_array.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>
#include <scriptarray.h>

namespace Engine
{
#define script_array_init_check(fail_ret)                                                                                        \
	if (m_as_array == nullptr)                                                                                                   \
		return fail_ret;

	static FORCE_INLINE asIScriptEngine* engine()
	{
		return ScriptEngine::engine();
	}

	int ScriptArrayBase::find_object_type_id() const
	{
		return engine()->GetTypeIdByDecl(full_declaration().c_str());
	}

	asITypeInfo* ScriptArrayBase::find_object_type() const
	{
		auto id = find_object_type_id();
		return engine()->GetTypeInfoById(id);
	}

	void ScriptArrayBase::insert_last(const void* ptr)
	{
		script_array_init_check();
		m_as_array->InsertLast(const_cast<void*>(ptr));
	}

	void* ScriptArrayBase::element_at(size_type pos) const
	{
		script_array_init_check(nullptr);
		void* result = m_as_array->At(static_cast<asUINT>(pos));
		if (result == nullptr)
		{
			throw EngineException("Position out of range");
		}
		return result;
	}

	void ScriptArrayBase::do_copy(const ScriptArrayBase* from)
	{
		if (from->m_as_array == nullptr)
		{
			release();
			return;
		}

		if (m_as_array == nullptr)
		{
			create();
		}

		if (m_as_array)
		{
			(*m_as_array) = (*(from->m_as_array));
		}
	}

	void ScriptArrayBase::do_move(ScriptArrayBase* from)
	{
		m_as_array       = from->m_as_array;
		from->m_as_array = nullptr;
	}

	ScriptArrayBase::ScriptArrayBase() : m_as_array(nullptr) {}

	bool ScriptArrayBase::create(size_type init_size)
	{
		asITypeInfo* info = find_object_type();
		if (info == nullptr)
			return false;
		CScriptArray* array = CScriptArray::Create(info, init_size);
		bool result         = attach(array, false);
		if (!result)
		{
			array->Release();
		}

		return result;
	}

	bool ScriptArrayBase::has_array() const
	{
		return m_as_array != nullptr;
	}

	bool ScriptArrayBase::attach(CScriptArray* array, bool add_reference)
	{
		if (array->GetArrayTypeId() != find_object_type_id())
			return false;

		release();

		if (add_reference)
		{
			array->AddRef();
		}

		m_as_array = array;
		return true;
	}

	const ScriptArrayBase& ScriptArrayBase::release() const
	{
		if (m_as_array && engine())
		{
			m_as_array->Release();
			m_as_array = nullptr;
		}
		return *this;
	}

	CScriptArray* ScriptArrayBase::array(bool inc_ref_count)
	{
		if (m_as_array == nullptr)
			return nullptr;

		if (inc_ref_count)
		{
			m_as_array->AddRef();
		}

		return m_as_array;
	}

	ScriptArrayBase::size_type ScriptArrayBase::size() const
	{
		script_array_init_check(0);
		return static_cast<size_type>(m_as_array->GetSize());
	}

	ScriptArrayBase& ScriptArrayBase::resize(size_type new_size)
	{
		script_array_init_check(*this);
		m_as_array->Resize(new_size);
		return *this;
	}

	bool ScriptArrayBase::empty() const
	{
		script_array_init_check(true);
		return m_as_array->IsEmpty();
	}

	ScriptArrayBase& ScriptArrayBase::reserve(size_type n)
	{
		script_array_init_check(*this);
		m_as_array->Reserve(n);
		return *this;
	}

	size_t ScriptArrayBase::add_reference() const
	{
		script_array_init_check(0);
		m_as_array->AddRef();
		return references();
	}

	size_t ScriptArrayBase::remove_reference() const
	{
		script_array_init_check(0);
		m_as_array->Release();
		return references();
	}

	size_t ScriptArrayBase::references() const
	{
		script_array_init_check(0);
		return m_as_array->GetRefCount();
	}

	ScriptArrayBase::~ScriptArrayBase()
	{
		release();
	}
}// namespace Engine
