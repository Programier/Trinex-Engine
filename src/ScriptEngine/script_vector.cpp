#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
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
		struct Instance : public Vector<byte> {
			using Vector::Vector;

			using Vector::m_end;
			using Vector::m_finish;
			using Vector::m_start;
		};

		static asIScriptFunction* find_default_constructor(asITypeInfo* ot)
		{
			auto beh_count = ot->GetBehaviourCount();

			for (asUINT bi = 0; bi < beh_count; ++bi)
			{
				asEBehaviours beh_type;
				auto beh = ot->GetBehaviourByIndex(bi, &beh_type);

				if (beh_type == asBEHAVE_CONSTRUCT && beh->GetParamCount() == 0)
				{
					return beh;
				}
			}

			return nullptr;
		}

		static asIScriptFunction* find_copy_constructor(asITypeInfo* ot)
		{
			auto beh_count = ot->GetBehaviourCount();

			for (asUINT bi = 0; bi < beh_count; ++bi)
			{
				asEBehaviours beh_type;
				auto beh = ot->GetBehaviourByIndex(bi, &beh_type);

				if (beh_type == asBEHAVE_CONSTRUCT && beh->GetParamCount() == 1)
				{
					int_t type_id;
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

		static asIScriptFunction* find_destructor(asITypeInfo* ot)
		{
			auto beh_count = ot->GetBehaviourCount();

			for (asUINT bi = 0; bi < beh_count; ++bi)
			{
				asEBehaviours beh_type;
				auto beh = ot->GetBehaviourByIndex(bi, &beh_type);

				if (beh_type == asBEHAVE_DESTRUCT)
				{
					return beh;
				}
			}

			return nullptr;
		}

		static Instance* instance(asIScriptGeneric* g)
		{
			return reinterpret_cast<Instance*>(g->GetObject());
		}

		static Instance* instance(asIScriptGeneric* g, asUINT arg)
		{
			return reinterpret_cast<Instance*>(g->GetArgAddress(arg));
		}

		static int_t element_type_id(asIScriptGeneric* g)
		{
			return ScriptEngine::engine()->GetTypeInfoById(g->GetObjectTypeId())->GetSubTypeId(0);
		}

		static asITypeInfo* element_type(int_t type_id)
		{
			if (ScriptEngine::is_object_type(type_id))
				return ScriptEngine::engine()->GetTypeInfoById(type_id);
			return nullptr;
		}

		static asITypeInfo* element_type(asIScriptGeneric* g)
		{
			return element_type(element_type_id(g));
		}

		static size_t element_size(int_t type_id)
		{
			if (ScriptEngine::is_primitive_type(type_id))
			{
				return ScriptEngine::sizeof_primitive_type(type_id);
			}

			if (ScriptEngine::is_handle_type(type_id))
			{
				return sizeof(void*);
			}

			return ScriptEngine::engine()->GetTypeInfoById(type_id)->GetSize();
		}

		static size_t element_size(asITypeInfo* info)
		{
			return info->GetSize();
		}

		static size_t element_size(asIScriptGeneric* g)
		{
			return element_size(element_type_id(g));
		}

		static void fill_primitives(byte* begin, byte* end, byte* default_value, size_t size_of_v)
		{
			while (begin < end)
			{
				std::memcpy(begin, default_value, size_of_v);
				begin += size_of_v;
			}
		}

		static void fill_primitives(Instance& array, byte* default_value, size_t size_of_v)
		{
			fill_primitives(array.m_start, array.m_finish, default_value, size_of_v);
		}

		static void call_default_constructor(byte* mem, asITypeInfo* type, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_default_constructor(type);

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

		static void call_default_constructor(byte* begin, byte* end, asITypeInfo* type)
		{
			auto f = find_default_constructor(type);

			if (f == nullptr)
				return;

			auto size = type->GetSize();

			while (begin < end)
			{
				call_default_constructor(begin, type, f);
				begin += size;
			}
		}

		static void call_default_constructor(Instance& array, asITypeInfo* type)
		{
			call_default_constructor(array.m_start, array.m_finish, type);
		}

		static void call_copy_constructor(byte* mem, byte* src, asITypeInfo* type, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_copy_constructor(type);

			if (f)
			{
				auto context = asGetActiveContext();

				context->PushState();
				context->Prepare(f);
				context->SetObject(mem);
				context->SetArgAddress(0, src);
				context->Execute();
				context->PopState();
			}
		}

		static void call_copy_constructor(byte* begin, byte* end, byte* src, asITypeInfo* type)
		{
			auto f = find_copy_constructor(type);

			if (f == nullptr)
				return;

			auto size = type->GetSize();

			while (begin < end)
			{
				call_copy_constructor(begin, src, type, f);
				begin += size;
			}
		}

		static void call_copy_constructor_list(byte* begin, byte* end, byte* src, asITypeInfo* type)
		{
			auto f = find_copy_constructor(type);

			if (f == nullptr)
				return;

			auto size = type->GetSize();

			while (begin < end)
			{
				call_copy_constructor(begin, src, type, f);
				begin += size;
				src += size;
			}
		}

		static void call_copy_constructor(Instance& array, byte* src, asITypeInfo* type)
		{
			call_copy_constructor(array.m_start, array.m_finish, src, type);
		}

		static void call_copy_constructor_list(Instance& array, byte* src, asITypeInfo* type)
		{
			call_copy_constructor_list(array.m_start, array.m_finish, src, type);
		}

		static void call_destructor(byte* mem, asITypeInfo* type, asIScriptFunction* f = nullptr)
		{
			if (f == nullptr)
				f = find_destructor(type);

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

		static void call_destructor(byte* begin, byte* end, asITypeInfo* type)
		{
			auto f = find_destructor(type);

			if (f == nullptr)
				return;

			auto size = type->GetSize();

			while (begin < end)
			{
				call_destructor(begin, type, f);
				begin += size;
			}
		}

		static void call_destructor(Instance& array, asITypeInfo* type)
		{
			call_destructor(array.m_start, array.m_finish, type);
		}

		///////////////// IMPLEMENTATION /////////////////

		static void constructor(Instance* self, asITypeInfo* ot)
		{
			new (self) Instance();
		}

		static void constructor_sz(Instance* self, asITypeInfo* ot, size_t size)
		{
			int_t type_id = ot->GetSubTypeId(0);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				auto component_size = element_size(type_id);
				new (self) Instance(size * component_size);
				return;
			}

			constructor(self, ot);

			auto type           = ot->GetSubType(0);
			auto component_size = static_cast<size_t>(type->GetSize());
			self->reserve(size * component_size);
			self->m_finish = self->m_end;

			call_default_constructor(*self, type);
		}

		static void constructor_sz_v(Instance* self, asITypeInfo* ot, size_t size, byte* default_value)
		{
			int_t type_id = ot->GetSubTypeId(0);

			constructor(self, ot);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				auto component_size = element_size(type_id);
				self->reserve(component_size * size);
				self->m_finish = self->m_end;

				fill_primitives(*self, default_value, component_size);
				return;
			}

			auto type           = ot->GetSubType(0);
			auto component_size = static_cast<size_t>(type->GetSize());
			self->reserve(component_size * size);
			call_copy_constructor(*self, default_value, type);
		}

		static void constructor_lst(Instance* self, asITypeInfo* ot, asUINT* lst)
		{
			int_t type_id = ot->GetSubTypeId(0);
			size_t size   = static_cast<size_t>(*lst) * element_size(type_id);
			byte* src     = reinterpret_cast<byte*>(lst + 1);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				new (self) Instance(src, src + size);
				return;
			}

			constructor(self, ot);
			self->reserve(size);
			self->m_finish = self->m_end;

			call_copy_constructor_list(*self, src, ot->GetSubType(0));
		}

		static void copy_constructor(Instance* self, asITypeInfo* ot, Instance* other)
		{
			int_t type_id = ot->GetSubTypeId(0);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				new (self) Instance(*other);
				return;
			}

			constructor(self, ot);
			self->reserve(other->size());
			self->m_finish = self->m_end;

			call_copy_constructor_list(*self, other->data(), ot->GetSubType(0));
		}

		static void destructor(asIScriptGeneric* g)
		{
			clear(g);
			auto self = instance(g);
			std::destroy_at(self);
		}

		static void opAssign(asIScriptGeneric* g)
		{
			auto self  = instance(g);
			auto other = instance(g, 0);

			if (self == other)
				return;

			int_t type_id = element_type_id(g);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				*self = *other;
				return;
			}

			clear(g);
			self->reserve(other->size());
			self->m_finish = self->m_start + other->size();
			call_copy_constructor_list(*self, other->data(), ScriptEngine::engine()->GetTypeInfoById(type_id));
		}

		static void at(asIScriptGeneric* g)
		{
			auto self = instance(g);
			try
			{
				g->SetReturnAddress(&(self->at(g->GetArgQWord(0) * element_size(g))));
			}
			catch (const std::exception& e)
			{
				asGetActiveContext()->SetException(e.what());
			}
		}

		static byte* front(Instance* self)
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
			auto self = instance(g);
			if (self->empty())
			{
				asGetActiveContext()->SetException("Index out of range!");
				return;
			}
			g->SetReturnAddress(self->m_finish - element_size(g));
		}

		static bool empty(Instance* self)
		{
			return self->empty();
		}

		static void size(asIScriptGeneric* g)
		{
			auto self = instance(g);
			g->SetReturnQWord(self->size() / element_size(g));
		}

		static void max_size(asIScriptGeneric* g)
		{
			auto self = instance(g);
			g->SetReturnQWord(self->max_size() / element_size(g));
		}

		static void capacity(asIScriptGeneric* g)
		{
			auto self = instance(g);
			g->SetReturnQWord(self->capacity() / element_size(g));
		}

		static void clear(asIScriptGeneric* g)
		{
			auto self     = instance(g);
			int_t type_id = element_type_id(g);

			if (ScriptEngine::is_object_type(type_id))
			{
				auto type = ScriptEngine::engine()->GetTypeInfoById(type_id);
				call_destructor(*self, type);
			}

			self->clear();
		}

		static void reserve_impl(Instance* self, size_t result_cap, int_t type_id)
		{
			if (self->capacity() >= result_cap)
				return;

			try
			{
				if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
				{
					self->reserve(result_cap);
					return;
				}

				Instance new_buffer;
				new_buffer.reserve(result_cap);

				if (!self->empty())
				{
					new_buffer.m_finish = new_buffer.m_start + self->size();
					call_copy_constructor_list(new_buffer.data(), new_buffer.data() + self->size(), self->data(),
					                           element_type(type_id));
				}
				self->swap(new_buffer);
			}
			catch (const std::exception& e)
			{
				asGetActiveContext()->SetException(e.what());
			}
		}

		static void reserve(asIScriptGeneric* g)
		{
			auto type_id = element_type_id(g);
			reserve_impl(instance(g), g->GetArgQWord(0) * element_size(type_id), type_id);
		}

		static void resize(asIScriptGeneric* g)
		{
			auto self           = instance(g);
			size_t n            = static_cast<size_t>(g->GetArgQWord(0));
			size_t bytes        = n * element_size(g);
			size_t current_size = self->size();

			if (bytes > current_size)
			{
				auto type_id = element_type_id(g);
				reserve_impl(self, bytes, type_id);

				if (ScriptEngine::is_object_type(type_id))
				{
					self->m_finish = self->m_start + bytes;
					call_default_constructor(self->data() + current_size, self->m_finish, element_type(type_id));
				}
				else
				{
					self->resize(bytes);
				}
			}
			else
			{
				auto type_id = element_type_id(g);
				if (ScriptEngine::is_object_type(type_id))
					call_destructor(self->data() + bytes, self->m_finish, element_type(type_id));

				self->m_finish = self->m_start + bytes;
			}
		}

		static void resize_v(asIScriptGeneric* g)
		{
			auto self           = instance(g);
			size_t n            = static_cast<size_t>(g->GetArgQWord(0));
			size_t ell_size     = element_size(g);
			size_t bytes        = n * ell_size;
			size_t current_size = self->size();

			if (bytes > current_size)
			{
				auto type_id = element_type_id(g);
				reserve_impl(self, bytes, type_id);

				byte* src      = static_cast<byte*>(g->GetArgAddress(1));
				self->m_finish = self->m_start + bytes;

				if (ScriptEngine::is_object_type(type_id))
				{
					call_copy_constructor(self->data() + current_size, self->m_finish, src, element_type(type_id));
				}
				else
				{
					fill_primitives(self->data() + current_size, self->m_finish, src, ell_size);
				}
			}
			else
			{
				auto type_id = element_type_id(g);
				if (ScriptEngine::is_object_type(type_id))
					call_destructor(self->data() + bytes, self->data() + current_size, element_type(type_id));
				self->m_finish = self->m_start + bytes;
			}
		}

		static void assign_nv(asIScriptGeneric* g)
		{
			auto self       = instance(g);
			int_t type_id   = element_type_id(g);
			size_t ell_size = element_size(type_id);
			size_t n        = static_cast<size_t>(g->GetArgQWord(0));
			byte* v         = static_cast<byte*>(g->GetArgAddress(1));

			size_t bytes = n * ell_size;

			clear(g);
			reserve_impl(self, bytes, type_id);
			self->resize(bytes);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				fill_primitives(self->data(), self->data() + bytes, v, ell_size);
				return;
			}

			call_copy_constructor(self->data(), self->data() + bytes, v, element_type(type_id));
		}

		static void grow(Instance* self, size_t bytes, int_t type_id)
		{
			size_t c = self->capacity();

			if (c == 0)
				c = 1;

			while (c < self->size() + bytes) c *= 2;
			reserve_impl(self, c, type_id);
		}

		static void prepare_insert(Instance* self, size_t start_bytes, size_t num_bytes, int_t type_id, size_t ell_size)
		{
			grow(self, num_bytes, type_id);

			auto to    = self->m_finish + num_bytes;
			auto end   = self->m_finish;
			auto start = self->m_start + start_bytes;

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				std::move_backward(start, end, to);
				self->m_finish += num_bytes;
				return;
			}

			auto type       = element_type(type_id);
			auto copy_f     = find_copy_constructor(type);
			auto destruct_f = find_destructor(type);

			while (start < end)
			{
				end -= ell_size;
				to -= ell_size;

				call_copy_constructor(to, end, type, copy_f);
				call_destructor(end, type, destruct_f);
			}

			self->m_finish += num_bytes;
		}

		static void insert_pnv_impl(Instance* self, size_t p, size_t n, byte* v, int_t type_id)
		{
			auto ell_size = element_size(type_id);
			prepare_insert(self, p * ell_size, n * ell_size, type_id, ell_size);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				fill_primitives(self->m_start + p * ell_size, self->m_start + ((p + n) * ell_size), v, ell_size);
				return;
			}

			call_copy_constructor(self->m_start + p * ell_size, self->m_start + ((p + n) * ell_size), v, element_type(type_id));
		}

		static void insert_pv(asIScriptGeneric* g)
		{
			size_t p  = g->GetArgQWord(0);
			byte* v   = static_cast<byte*>(g->GetArgAddress(1));
			auto self = instance(g);
			insert_pnv_impl(self, p, 1, v, element_type_id(g));
		}

		static void insert_pnv(asIScriptGeneric* g)
		{
			size_t p  = g->GetArgQWord(0);
			size_t n  = g->GetArgQWord(1);
			byte* v   = static_cast<byte*>(g->GetArgAddress(2));
			auto self = instance(g);
			insert_pnv_impl(self, p, n, v, element_type_id(g));
		}

		static void push_back(asIScriptGeneric* g)
		{
			auto self = instance(g);
			byte* v   = static_cast<byte*>(g->GetArgAddress(0));

			int_t type_id   = element_type_id(g);
			size_t ell_size = element_size(type_id);

			grow(self, ell_size, type_id);

			if (ScriptEngine::is_primitive_type(type_id) || ScriptEngine::is_handle_type(type_id))
			{
				fill_primitives(self->m_finish, self->m_finish + ell_size, v, ell_size);
				self->m_finish += ell_size;
				return;
			}

			auto type = element_type(type_id);
			call_copy_constructor(self->m_finish, v, type);
			self->m_finish += ell_size;
		}

		static void pop_back(asIScriptGeneric* g)
		{
			auto self = instance(g);

			if (self->empty())
				return;

			int_t type_id   = element_type_id(g);
			size_t ell_size = element_size(type_id);

			self->m_finish -= ell_size;

			if (ScriptEngine::is_object_type(type_id))
			{
				call_destructor(self->m_finish, element_type(type_id));
			}
		}

		static void erase(asIScriptGeneric* g)
		{
			auto self     = instance(g);
			int_t type_id = element_type_id(g);

			size_t p = static_cast<size_t>(g->GetArgQWord(0));
			size_t n = static_cast<size_t>(g->GetArgQWord(1));

			asITypeInfo* type = nullptr;
			size_t ell_size   = 0;

			if (ScriptEngine::is_object_type(type_id))
			{
				type     = element_type(type_id);
				ell_size = element_size(type);

				p *= ell_size;
				n *= ell_size;

				byte* start = self->m_start + p;
				byte* end   = start + n;

				call_destructor(start, end, type);

				auto copy_f    = find_copy_constructor(type);
				auto destroy_f = find_destructor(type);

				while (end < self->m_finish)
				{
					call_copy_constructor(start, end, type, copy_f);
					call_destructor(end, type, destroy_f);

					start += ell_size;
					end += ell_size;
				}

				self->m_finish -= n;
			}
			else
			{
				ell_size = element_size(type_id);
				p *= ell_size;
				n *= ell_size;
				self->erase(self->begin() + p, self->begin() + p + n);
			}
		}
	};

	static bool template_callback(asITypeInfo* ot, bool& dont_garbage_collect)
	{
		using T              = ScriptVector;
		dont_garbage_collect = true;

		int_t type_id = ot->GetSubTypeId();

		if (type_id & asTYPEID_MASK_OBJECT)
		{
			if ((type_id & asTYPEID_APPOBJECT) == 0)
			{
				error_log("ScriptVector", "ScriptVector doesn't support script objects!");
				return false;
			}

			if ((type_id & asTYPEID_OBJHANDLE) == 0)
			{
				auto type = ot->GetSubType(0);

				if (T::find_default_constructor(type) == nullptr)
				{
					error_log("ScriptVector", "ScriptVector requires default constructor!");
					return false;
				}

				if (T::find_copy_constructor(type) == nullptr)
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
		r.behave(ScriptClassBehave::Construct, "void f(int&, uint64, const T& in v)", T::constructor_sz_v);
		r.behave(ScriptClassBehave::ListConstruct, "void f(int&, int&in) { repeat T }", T::constructor_lst);
		r.behave(ScriptClassBehave::Construct, "void f(int&, const Vector<T>& in)", T::copy_constructor);
		r.behave(ScriptClassBehave::Destruct, "void f()", T::destructor, ScriptCallConv::Generic);
		r.method("Vector<T>& opAssign(const Vector<T>& in)", T::opAssign, ScriptCallConv::Generic);
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
		r.method("void resize(uint64 new_size, const T& in v)", T::resize_v, ScriptCallConv::Generic);
		r.method("void assign(uint64 new_size, const T& in v)", T::assign_nv, ScriptCallConv::Generic);
		r.method("void insert(uint64 pos, const T& in v)", T::insert_pv, ScriptCallConv::Generic);
		r.method("void insert(uint64 pos, uint64 n, const T& in v)", T::insert_pnv, ScriptCallConv::Generic);
		r.method("void push_back(const T& in v)", T::push_back, ScriptCallConv::Generic);
		r.method("void pop_back()", T::pop_back, ScriptCallConv::Generic);
		r.method("void erase(uint64 p, uint64 n = 1)", T::erase, ScriptCallConv::Generic);
	}

	static ReflectionInitializeController initializer(initialize, "Engine::ScriptVector");
}// namespace Engine
