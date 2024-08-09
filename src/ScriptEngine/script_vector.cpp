// #include <Core/engine_loading_controllers.hpp>
// #include <ScriptEngine/registrar.hpp>
// #include <ScriptEngine/script_engine.hpp>
//#include <ScriptEngine/script_vector.hpp>
// #include <angelscript.h>

// namespace Engine
// {
//     ScriptVectorBase::ScriptVectorBase(const asITypeInfo* type)
//         : m_refs(0), m_element_size(0), m_element_type_id(type->GetSubTypeId()), m_type(type)
//     {}

//     size_t ScriptVectorBase::element_size() const
//     {
//         if (m_element_size == 0)
//         {
//             if (is_handle_element())
//                 m_element_size = sizeof(void*);

//             else if (is_object_element())
//                 m_element_size = m_type->GetSubType()->GetSize();
//             else
//                 m_element_size = ScriptEngine::sizeof_primitive_type(m_type->GetSubTypeId());
//         }
//         return m_element_size;
//     }

//     bool ScriptVectorBase::is_primitive_element() const
//     {
//         return m_element_type_id <= asTYPEID_DOUBLE;
//     }

//     bool ScriptVectorBase::is_object_element() const
//     {
//         return (m_element_type_id & asTYPEID_MASK_OBJECT) && !is_handle_element();
//     }

//     bool ScriptVectorBase::is_handle_element() const
//     {
//         return (m_element_type_id & asTYPEID_OBJHANDLE) != 0;
//     }

//     void* ScriptVectorBase::address_of(size_t index)
//     {
//         return Super::data() + (index * element_size());
//     }

//     const void* ScriptVectorBase::address_of(size_t index) const
//     {
//         return Super::data() + (index * element_size());
//     }

//     size_t ScriptVectorBase::size() const
//     {
//         return Super::size() / element_size();
//     }

//     size_t ScriptVectorBase::capacity() const
//     {
//         return Super::capacity() / element_size();
//     }

//     void ScriptVectorBase::reserve(size_t elements)
//     {
//         Super::reserve(elements * element_size());
//     }

//     void ScriptVectorBase::destroy_at(size_t index)
//     {
//         if (is_object_element())
//         {
//             void* object            = address_of(index);
//             asIScriptEngine* engine = ScriptEngine::engine();
//             engine->ReleaseScriptObject(object, m_type->GetSubType());
//         }
//         else if (is_handle_element())
//         {
//             void* object            = *reinterpret_cast<void**>(address_of(index));
//             asIScriptEngine* engine = ScriptEngine::engine();
//             engine->ReleaseScriptObject(object, m_type->GetSubType());
//         }
//     }

//     void ScriptVectorBase::assign_to(size_t index, byte* element)
//     {}

//     void ScriptVectorBase::push_back(byte* element)
//     {}

//     struct ScriptVectorBaseInitializer {
//         static ScriptVectorBase* factory_default(asITypeInfo* ti)
//         {
//             ScriptVectorBase* vector = new ScriptVectorBase(ti);
//             vector->m_refs           = 1;
//             return vector;
//         }

//         static void add_ref(const ScriptVectorBase* vector)
//         {
//             ++vector->m_refs;
//         }

//         static void release(const ScriptVectorBase* vector)
//         {
//             --vector->m_refs;

//             if (vector->m_refs == 0)
//             {
//                 delete vector;
//             }
//         }


//         static void initialize()
//         {
//             using T = ScriptVectorBase;
//             ScriptClassRegistrar::ValueInfo info;
//             info.template_type = "<T>";

//             ScriptNamespaceScopedChanger changer("Engine");

//             ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("Vector<class T>", sizeof(T), info);
//             registrar.behave(ScriptClassBehave::Factory, "Vector<T>@ f(const int& in)", factory_default, ScriptCallConv::CDecl);

//             registrar.behave(ScriptClassBehave::AddRef, "void f() const", add_ref);
//             registrar.behave(ScriptClassBehave::Release, "void f() const", release);

//             registrar.method("uint64 element_size() const", &ScriptVectorBase::element_size);

//             registrar.method("bool is_primitive_element() const", &T::is_primitive_element);
//             registrar.method("bool is_object_element() const", &T::is_object_element);
//             registrar.method("bool is_handle_element() const", &T::is_handle_element);
//             registrar.method("uint64 size() const", &T::size);
//             registrar.method("uint64 capacity() const", &T::capacity);
//             registrar.method("void reserve(uint64 elements)", &T::reserve);
//         }
//     };

//     static ReflectionInitializeController on_init(ScriptVectorBaseInitializer::initialize, "Engine::ScriptVector");
// }// namespace Engine
