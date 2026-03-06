#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/templates.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>
#include <cstring>

namespace Engine
{
	class ScriptVector
	{
	public:
		struct Instance : public Vector<u8> {
			using Vector::Vector;

			using Vector::m_end;
			using Vector::m_finish;
			using Vector::m_start;
		};

		static Instance* m_self;
		static asITypeInfo* m_type;
		static i32 m_type_id;
		static usize m_type_size;
		static usize m_refs;


		struct TypeInitializer {
			void init()
			{
				if (ScriptEngine::is_primitive_type(m_type_id))
				{
					m_type_size = ScriptEngine::sizeof_primitive_type(m_type_id);
				}
				else if (ScriptEngine::is_handle_type(m_type_id))
				{
					m_type_size = sizeof(void*);
				}
				else
				{
					m_type      = ScriptEngine::engine()->GetTypeInfoById(m_type_id);
					m_type_size = m_type->GetSize();
				}
			}

			TypeInitializer(Instance* self, asITypeInfo* ot)
			{
				if (m_refs == 0)
				{
					m_self    = self;
					m_type_id = ot->GetSubTypeId(0);
					init();
				}

				++m_refs;
			}

			TypeInitializer(asIScriptGeneric* g)
			{
				if (m_refs == 0)
				{
					m_self    = reinterpret_cast<Instance*>(g->GetObject());
					m_type_id = ScriptEngine::engine()->GetTypeInfoById(g->GetObjectTypeId())->GetSubTypeId(0);
					init();
				}

				++m_refs;
			}

			static void destruct()
			{
				--m_refs;

				if (m_refs == 0)
				{
					m_self      = nullptr;
					m_type      = nullptr;
					m_type_id   = 0;
					m_type_size = 0;
				}
			}

			~TypeInitializer() { destruct(); }
		};


		struct TempInstance : public Instance {
			using Instance::Instance;

			u8* init_temp(u8* src)
			{
				++m_refs;
				reserve(m_type_size);
				m_finish = m_end;

				if (m_type)
					call_copy_constructor(m_start, src);
				else
					fill_primitives(m_start, m_finish, src);

				return m_start;
			}

			~TempInstance()
			{
				if (!empty() && m_type)
				{
					call_destructor(m_start);
					TypeInitializer::destruct();
				}
			}
		};

		static inline asIScriptFunction* primitive_behaviour()
		{
			static auto beh = reinterpret_cast<asIScriptFunction*>(static_cast<u8*>(nullptr) + 1);
			return beh;
		}

		static asIScriptFunction* find_default_constructor()
		{
			if (m_type == nullptr || m_type->GetFlags() & asOBJ_POD)
				return primitive_behaviour();

			auto beh_count = m_type->GetBehaviourCount();

			for (asUINT bi = 0; bi < beh_count; ++bi)
			{
				asEBehaviours beh_type;
				auto beh = m_type->GetBehaviourByIndex(bi, &beh_type);

				if (beh_type == asBEHAVE_CONSTRUCT && beh->GetParamCount() == 0)
				{
					return beh;
				}
			}

			return nullptr;
		}

		static asIScriptFunction* find_copy_constructor()
		{
			if (m_type == nullptr || m_type->GetFlags() & asOBJ_POD)
				return primitive_behaviour();

			auto beh_count = m_type->GetBehaviourCount();

			for (asUINT bi = 0; bi < beh_count; ++bi)
			{
				asEBehaviours beh_type;
				auto beh = m_type->GetBehaviourByIndex(bi, &beh_type);

				if (beh_type == asBEHAVE_CONSTRUCT && beh->GetParamCount() == 1)
				{
					i32 type_id;
					asDWORD flags;

					if (beh->GetParam(0, &type_id, &flags) >= 0)
					{
						if ((flags & asTM_CONST) && (flags & asTM_INREF))
						{
							return beh;
						}
					}
				}
			}

			return nullptr;
		}

		static asIScriptFunction* find_destructor()
		{
			if (m_type == nullptr)
				return nullptr;

			auto beh_count = m_type->GetBehaviourCount();

			for (asUINT bi = 0; bi < beh_count; ++bi)
			{
				asEBehaviours beh_type;
				auto beh = m_type->GetBehaviourByIndex(bi, &beh_type);

				if (beh_type == asBEHAVE_DESTRUCT)
				{
					return beh;
				}
			}

			return nullptr;
		}

		static Instance* instance(asIScriptGeneric* g) { return reinterpret_cast<Instance*>(g->GetObject()); }

		static Instance* instance(asIScriptGeneric* g, asUINT arg) { return reinterpret_cast<Instance*>(g->GetArgAddress(arg)); }

		static void fill_primitives(u8* begin, u8* end, u8* default_value)
		{
			while (begin < end)
			{
				std::memcpy(begin, default_value, m_type_size);
				begin += m_type_size;
			}
		}

		static void fill_primitives(Instance& array, u8* default_value)
		{
			fill_primitives(array.m_start, array.m_finish, default_value);
		}

		static void fill_primitives_list(u8* begin, u8* end, u8* default_value)
		{
			while (begin < end)
			{
				std::memcpy(begin, default_value, m_type_size);
				begin += m_type_size;
				default_value += m_type_size;
			}
		}

		static void fill_primitives_list(Instance& array, u8* default_value)
		{
			fill_primitives_list(array.m_start, array.m_finish, default_value);
		}

		static void call_default_constructor(u8* mem, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_default_constructor();

			if (f && f != primitive_behaviour())
			{
				auto context = asGetActiveContext();

				context->PushState();
				context->Prepare(f);
				context->SetObject(mem);
				context->Execute();
				context->PopState();
				return;
			}

			std::memset(mem, 0, m_type_size);
		}

		static void call_default_constructor(u8* begin, u8* end, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_default_constructor();

			if (f == nullptr)
				return;

			while (begin < end)
			{
				call_default_constructor(begin, f);
				begin += m_type_size;
			}
		}

		static void call_default_constructor(Instance& array, asIScriptFunction* f = nullptr)
		{
			call_default_constructor(array.m_start, array.m_finish, f);
		}

		static void call_copy_constructor(u8* mem, u8* src, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_copy_constructor();

			if (f && f != primitive_behaviour())
			{
				auto context = asGetActiveContext();

				context->PushState();
				context->Prepare(f);
				context->SetObject(mem);
				context->SetArgAddress(0, src);
				context->Execute();
				context->PopState();
				return;
			}

			std::memcpy(mem, src, m_type_size);
		}

		static void call_copy_constructor(u8* begin, u8* end, u8* src)
		{
			auto f = find_copy_constructor();

			if (f == nullptr)
				return;

			while (begin < end)
			{
				call_copy_constructor(begin, src, f);
				begin += m_type_size;
			}
		}

		static void call_copy_constructor_list(u8* begin, u8* end, u8* src)
		{
			auto f = find_copy_constructor();

			if (f == nullptr)
				return;

			while (begin < end)
			{
				call_copy_constructor(begin, src, f);
				begin += m_type_size;
				src += m_type_size;
			}
		}

		static void call_copy_constructor(Instance& array, u8* src) { call_copy_constructor(array.m_start, array.m_finish, src); }

		static void call_copy_constructor_list(Instance& array, u8* src)
		{
			call_copy_constructor_list(array.m_start, array.m_finish, src);
		}

		static void call_destructor(u8* mem, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_destructor();

			if (f)
			{
				auto context = asGetActiveContext();

				context->PushState();
				context->Prepare(f);
				context->SetObject(mem);
				context->Execute();
				context->PopState();
			}
		}

		static void call_destructor(u8* begin, u8* end, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_destructor();

			if (f == nullptr)
				return;

			while (begin < end)
			{
				call_destructor(begin, f);
				begin += m_type_size;
			}
		}

		static void call_destructor(Instance& array, asIScriptFunction* f = nullptr)
		{
			call_destructor(array.m_start, array.m_finish, f);
		}

		static bool is_current_array_element(u8* obj) { return obj >= m_self->m_start && obj <= m_self->m_end; }

		///////////////// IMPLEMENTATION /////////////////

		static void constructor(Instance* self, asITypeInfo* ot) { new (self) Instance(); }

		static void constructor_sz(Instance* self, asITypeInfo* ot, usize size)
		{
			ScriptVector::TypeInitializer initializer(self, ot);

			if (m_type == nullptr)
			{
				new (self) Instance(size * m_type_size);
				return;
			}

			constructor(self, ot);
			self->reserve(size * m_type_size);
			self->m_finish = self->m_end;

			call_default_constructor(*self);
		}

		static void constructor_sz_v(Instance* self, asITypeInfo* ot, usize size, u8* default_value)
		{
			ScriptVector::TypeInitializer initializer(self, ot);
			constructor(self, ot);

			self->reserve(m_type_size * size);
			self->m_finish = self->m_end;

			if (m_type)
				call_copy_constructor(*self, default_value);
			else
				fill_primitives(*self, default_value);
		}

		static void constructor_lst(Instance* self, asITypeInfo* ot, asUINT* lst)
		{
			ScriptVector::TypeInitializer initializer(self, ot);

			usize size = static_cast<usize>(*lst) * m_type_size;
			u8* src    = reinterpret_cast<u8*>(lst + 1);

			if (m_type)
			{
				constructor(self, ot);
				self->reserve(size);
				self->m_finish = self->m_end;

				call_copy_constructor_list(*self, src);
				return;
			}

			new (self) Instance(src, src + size);
		}

		static void copy_constructor(Instance* self, asITypeInfo* ot, Instance* other)
		{
			ScriptVector::TypeInitializer initializer(self, ot);

			if (m_type)
			{
				constructor(self, ot);
				self->reserve(other->size());
				self->m_finish = self->m_end;

				call_copy_constructor_list(*self, other->data());
				return;
			}

			new (self) Instance(*other);
		}

		static void destructor(asIScriptGeneric* g)
		{
			clear(g);
			std::destroy_at(instance(g));
		}

		static void opAssign(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			auto other = instance(g, 0);

			if (m_self == other)
				return;

			if (m_type)
			{
				clear(g);
				m_self->reserve(other->size());
				m_self->m_finish = m_self->m_start + other->size();
				call_copy_constructor_list(*m_self, other->data());
			}
			else
				*m_self = *other;
		}

		static void at(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			g->SetReturnAddress(&(m_self->at(g->GetArgQWord(0) * m_type_size)));
		}

		static u8* front(Instance* self)
		{
			if (self->empty())
			{
				asGetActiveContext()->SetException("Index out of range!");
				return nullptr;
			}

			return &(self->front());
		}

		static void back(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			if (m_self->empty())
			{
				asGetActiveContext()->SetException("Index out of range!");
				return;
			}

			g->SetReturnAddress(m_self->m_finish - m_type_size);
		}

		static bool empty(Instance* self) { return self->empty(); }

		static void size(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			g->SetReturnQWord(m_self->size() / m_type_size);
		}

		static void max_size(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			g->SetReturnQWord(m_self->max_size() / m_type_size);
		}

		static void capacity(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			g->SetReturnQWord(m_self->capacity() / m_type_size);
		}

		static void clear(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			if (ScriptEngine::is_object_type(m_type_id))
			{
				call_destructor(*m_self);
			}

			m_self->m_finish = m_self->m_start;
		}

		static void reserve_impl(usize result_cap)
		{
			if (m_self->capacity() >= result_cap)
				return;

			if (m_type)
			{
				Instance new_buffer;
				new_buffer.reserve(result_cap);

				if (!m_self->empty())
				{
					new_buffer.m_finish = new_buffer.m_start + m_self->size();
					call_copy_constructor_list(new_buffer.data(), new_buffer.data() + m_self->size(), m_self->data());
				}
				m_self->swap(new_buffer);
			}
			else
				m_self->reserve(result_cap);
		}

		static void reserve(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			reserve_impl(g->GetArgQWord(0) * m_type_size);
		}

		static void resize(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			usize n            = static_cast<usize>(g->GetArgQWord(0));
			usize bytes        = n * m_type_size;
			usize current_size = m_self->size();

			if (bytes > current_size)
			{
				reserve_impl(bytes);

				if (m_type)
				{
					m_self->m_finish = m_self->m_start + bytes;
					call_default_constructor(m_self->data() + current_size, m_self->m_finish);
				}
				else
				{
					m_self->m_finish = m_self->m_start + bytes;
				}
			}
			else
			{
				if (m_type)
					call_destructor(m_self->data() + bytes, m_self->m_finish);

				m_self->m_finish = m_self->m_start + bytes;
			}
		}

		static void resize_v(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			usize n            = static_cast<usize>(g->GetArgQWord(0));
			usize bytes        = n * m_type_size;
			usize current_size = m_self->size();

			if (bytes > current_size)
			{
				u8* src = static_cast<u8*>(g->GetArgAddress(1));
				TempInstance tmp;

				if (bytes > m_self->capacity() && is_current_array_element(src))
					src = tmp.init_temp(src);

				reserve_impl(bytes);

				m_self->m_finish = m_self->m_start + bytes;

				if (m_type)
				{
					call_copy_constructor(m_self->data() + current_size, m_self->m_finish, src);
				}
				else
				{
					fill_primitives(m_self->data() + current_size, m_self->m_finish, src);
				}
			}
			else
			{
				if (m_type)
					call_destructor(m_self->data() + bytes, m_self->data() + current_size);
				m_self->m_finish = m_self->m_start + bytes;
			}
		}

		static void assign_nv(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			usize n = static_cast<usize>(g->GetArgQWord(0)) * m_type_size;
			u8* v   = static_cast<u8*>(g->GetArgAddress(1));

			TempInstance tmp;
			if (n > m_self->capacity() && is_current_array_element(v))
				v = tmp.init_temp(v);

			clear(g);
			reserve_impl(n);
			m_self->m_finish = m_self->m_start + n;

			if (m_type)
			{
				call_copy_constructor(m_self->m_start, m_self->m_finish, v);
				return;
			}

			fill_primitives(m_self->m_start, m_self->m_finish, v);
		}

		static void grow(usize bytes)
		{
			usize c = m_self->capacity();

			if (c == 0)
				c = 1;

			while (c < m_self->size() + bytes) c *= 2;
			reserve_impl(c);
		}

		static void prepare_insert(usize start_bytes, usize num_bytes)
		{
			grow(num_bytes);

			auto to    = m_self->m_finish + num_bytes;
			auto end   = m_self->m_finish;
			auto start = m_self->m_start + start_bytes;

			m_self->m_finish += num_bytes;

			if (m_type == nullptr)
			{
				std::move_backward(start, end, to);
				return;
			}

			auto copy_f     = find_copy_constructor();
			auto destruct_f = find_destructor();

			while (start < end)
			{
				end -= m_type_size;
				to -= m_type_size;

				call_copy_constructor(to, end, copy_f);
				call_destructor(end, destruct_f);
			}
		}

		static void insert_pnv_impl(usize p, usize n, u8* v)
		{
			p *= m_type_size;
			n *= m_type_size;

			TempInstance tmp;
			if ((m_self->size() + n > m_self->capacity()) && is_current_array_element(v))
				v = tmp.init_temp(v);

			if (m_type)
			{
				prepare_insert(p, n);
				call_copy_constructor(m_self->m_start + p, m_self->m_start + p + n, v);
			}
			else
				fill_primitives(m_self->m_start + p, m_self->m_start + p + n, v);
		}

		static void insert_pvec(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			Instance* other = instance(g, 1);
			usize p         = g->GetArgQWord(0) * m_type_size;
			usize n         = other->size();

			prepare_insert(p, n);
			auto* callback = m_type ? overload_of<void(u8*, u8*, u8*)>(call_copy_constructor_list)
			                        : overload_of<void(u8*, u8*, u8*)>(fill_primitives_list);

			if (m_self == other)
			{
				u8* uninitialized_start = m_self->m_start;
				u8* uninitialized_end   = uninitialized_start + n;

				callback(m_self->m_start, uninitialized_start, uninitialized_start);
				uninitialized_start += uninitialized_start - m_self->m_start;
				callback(uninitialized_end, m_self->m_finish, uninitialized_start);
			}
			else
			{
				callback(m_self->m_start + p, m_self->m_start + p + n, other->m_start);
			}
		}

		static void insert_pv(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			usize p = g->GetArgQWord(0);
			u8* v   = static_cast<u8*>(g->GetArgAddress(1));
			insert_pnv_impl(p, 1, v);
		}

		static void insert_pnv(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			usize p = g->GetArgQWord(0);
			usize n = g->GetArgQWord(1);
			u8* v   = static_cast<u8*>(g->GetArgAddress(2));
			insert_pnv_impl(p, n, v);
		}

		static void push_back(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);
			u8* v = static_cast<u8*>(g->GetArgAddress(0));

			TempInstance tmp;
			if ((m_self->size() + m_type_size > m_self->capacity()) && is_current_array_element(v))
				v = tmp.init_temp(v);

			grow(m_type_size);

			if (m_type)
				call_copy_constructor(m_self->m_finish, v);
			else
				fill_primitives(m_self->m_finish, m_self->m_finish + m_type_size, v);

			m_self->m_finish += m_type_size;
		}

		static void pop_back(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			if (m_self->empty())
				return;

			m_self->m_finish -= m_type_size;

			if (m_type)
			{
				call_destructor(m_self->m_finish);
			}
		}

		static void erase(asIScriptGeneric* g)
		{
			ScriptVector::TypeInitializer initializer(g);

			usize p = static_cast<usize>(g->GetArgQWord(0)) * m_type_size;
			usize n = static_cast<usize>(g->GetArgQWord(1)) * m_type_size;

			if (m_type)
			{
				u8* start = m_self->m_start + p;
				u8* end   = start + n;

				auto copy_f    = find_copy_constructor();
				auto destroy_f = find_destructor();

				call_destructor(start, end, destroy_f);

				while (end < m_self->m_finish)
				{
					call_copy_constructor(start, end, copy_f);
					call_destructor(end, destroy_f);

					start += m_type_size;
					end += m_type_size;
				}

				m_self->m_finish -= n;
			}
			else
			{
				m_self->erase(m_self->begin() + p, m_self->begin() + p + n);
			}
		}
	};

	ScriptVector::Instance* ScriptVector::m_self = nullptr;
	asITypeInfo* ScriptVector::m_type            = nullptr;
	i32 ScriptVector::m_type_id                  = 0;
	usize ScriptVector::m_type_size              = 0;
	usize ScriptVector::m_refs                   = 0;

	static bool template_callback(asITypeInfo* ot, bool& dont_garbage_collect)
	{
		ScriptVector::TypeInitializer initializer(nullptr, ot);

		using T              = ScriptVector;
		dont_garbage_collect = true;

		if (ScriptVector::m_type_id & asTYPEID_MASK_OBJECT)
		{
			if ((ScriptVector::m_type_id & asTYPEID_APPOBJECT) == 0)
			{
				error_log("ScriptVector", "ScriptVector doesn't support script objects!");
				return false;
			}

			if ((ScriptVector::m_type_id & asTYPEID_OBJHANDLE) == 0)
			{
				if (T::find_default_constructor() == nullptr)
				{
					error_log("ScriptVector", "ScriptVector requires default constructor!");
					return false;
				}

				if (T::find_copy_constructor() == nullptr)
				{
					error_log("ScriptVector", "ScriptVector requires copy constructor!");
					return false;
				}
			}
		}

		return true;
	}

	static void initialize()
	{
		using T                      = ScriptVector;
		auto info                    = ScriptClassRegistrar::ValueInfo();
		info.is_class                = true;
		info.all_ints                = true;
		info.align8                  = alignof(void*) == 8;
		info.has_constructor         = true;
		info.has_copy_constructor    = true;
		info.has_destructor          = true;
		info.has_assignment_operator = true;
		info.template_type           = "<T>";

		auto r = ScriptClassRegistrar::value_class("Engine::Vector<class T>", sizeof(ScriptVector::Instance), info);

		r.behave(ScriptClassBehave::TemplateCallback, "bool f(int &in, bool&out)", template_callback, ScriptCallConv::CDecl);

		r.behave(ScriptClassBehave::Construct, "void f(int&)", T::constructor);
		r.behave(ScriptClassBehave::Construct, "void f(int&, uint64) explicit", T::constructor_sz);
		r.behave(ScriptClassBehave::Construct, "void f(int&, uint64, const T& v)", T::constructor_sz_v);
		r.behave(ScriptClassBehave::ListConstruct, "void f(int&, int&in) { repeat T }", T::constructor_lst);
		r.behave(ScriptClassBehave::Construct, "void f(int&, const Vector<T>&)", T::copy_constructor);
		r.behave(ScriptClassBehave::Destruct, "void f()", T::destructor, ScriptCallConv::Generic);
		r.method("Vector<T>& opAssign(const Vector<T>&)", T::opAssign, ScriptCallConv::Generic);
		r.method("T& opIndex(uint64 i)", T::at, ScriptCallConv::Generic);
		r.method("const T& opIndex(uint64 i) const", T::at, ScriptCallConv::Generic);
		r.method("T& at(uint64 i)", T::at, ScriptCallConv::Generic);
		r.method("const T& at(uint64 i) const", T::at, ScriptCallConv::Generic);
		r.method("T& front()", T::front);
		r.method("const T& front() const", T::front);
		r.method("T& back()", T::back, ScriptCallConv::Generic);
		r.method("const T& back() const", T::back, ScriptCallConv::Generic);
		r.method("bool empty() const", T::empty);
		r.method("uint64 size() const", T::size, ScriptCallConv::Generic);
		r.method("uint64 max_size() const", T::max_size, ScriptCallConv::Generic);
		r.method("uint64 capacity() const", T::capacity, ScriptCallConv::Generic);
		r.method("void clear()", T::clear, ScriptCallConv::Generic);
		r.method("void reserve(uint64 n)", T::reserve, ScriptCallConv::Generic);
		r.method("void resize(uint64 new_size)", T::resize, ScriptCallConv::Generic);
		r.method("void resize(uint64 new_size, const T& v)", T::resize_v, ScriptCallConv::Generic);
		r.method("void assign(uint64 new_size, const T& v)", T::assign_nv, ScriptCallConv::Generic);
		r.method("void insert(uint64 pos, const Vector<T>& v)", T::insert_pvec, ScriptCallConv::Generic);
		r.method("void insert(uint64 pos, const T& v)", T::insert_pv, ScriptCallConv::Generic);
		r.method("void insert(uint64 pos, uint64 n, const T& v)", T::insert_pnv, ScriptCallConv::Generic);
		r.method("void push_back(const T& v)", T::push_back, ScriptCallConv::Generic);
		r.method("void pop_back()", T::pop_back, ScriptCallConv::Generic);
		r.method("void erase(uint64 p, uint64 n = 1)", T::erase, ScriptCallConv::Generic);
	}

	static PreInitializeController initializer(initialize, "Engine::ScriptVector");
}// namespace Engine
