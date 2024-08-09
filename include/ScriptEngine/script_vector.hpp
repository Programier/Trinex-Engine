// #pragma once
// #include <Core/engine_types.hpp>

// class asITypeInfo;

// namespace Engine
// {
//     class ENGINE_EXPORT ScriptVectorBase : protected Vector<byte>
//     {
//     public:
//         using difference_type = ptrdiff_t;
//         using size_type       = size_t;

//     protected:
//         using Super = Vector<byte>;
//         mutable size_t m_refs;
//         mutable size_t m_element_size;
//         const int_t m_element_type_id;
//         const asITypeInfo* m_type;

//     protected:
//         ScriptVectorBase(const asITypeInfo* type);

//         void destroy_at(size_t index);
//         void assign_to(size_t index, byte* element);

//         void push_back(byte* element);

//     public:
//         size_t element_size() const;
//         bool is_primitive_element() const;
//         bool is_object_element() const;
//         bool is_handle_element() const;

//         void* address_of(size_t index);
//         const void* address_of(size_t index) const;

//         size_t size() const;
//         size_t capacity() const;
//         void reserve(size_t elements);

//         friend struct ScriptVectorBaseInitializer;
//     };

//     template<typename T>
//     class ScriptVector : public ScriptVectorBase
//     {

//     public:
//         ScriptVector() : ScriptVectorBase(nullptr)
//         {}
//     };
// }// namespace Engine
