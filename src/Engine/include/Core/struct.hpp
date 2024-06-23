#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/templates.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/name.hpp>

namespace Engine
{
    using StructMap = Map<HashIndex, class Struct*>;

    class ENGINE_EXPORT Struct
    {
    public:
        using GroupedPropertiesMap = TreeMap<Name, Vector<class Property*>, Name::Less>;

    private:
        void* (*m_struct_constructor)() = nullptr;

    protected:
        String m_base_name_splitted;

        Name m_full_name;
        Name m_namespace_name;
        Name m_base_name;
        Name m_parent;
        class Group* m_group = nullptr;

        mutable Struct* m_parent_struct = nullptr;
        Vector<Struct*> m_childs;
        Vector<class Property*> m_properties;
        GroupedPropertiesMap m_grouped_properties;

        Struct(const Name& name, const Name& parent = Name::none);
        Struct(const Name& name, Struct* parent);

    public:
        static ENGINE_EXPORT Struct* create(const Name& name, const Name& parent = Name::none);
        static ENGINE_EXPORT Struct* static_find(const StringView& name, bool requred = false);

        const String& base_name_splitted() const;
        const Name& name() const;
        const Name& namespace_name() const;
        const Name& base_name() const;
        const Name& parent_name() const;
        Struct* parent() const;
        virtual void* create_struct() const;
        Struct& struct_constructor(void* (*constructor)());
        Struct& group(class Group*);
        class Group* group() const;
        size_t abstraction_level() const;
        Vector<Name> hierarchy(size_t offset = 0) const;
        const Vector<Struct*>& childs() const;

        bool is_a(const Struct* other) const;
        virtual bool is_class() const;

        Struct& add_property(Property* prop);
        const Vector<class Property*>& properties() const;
        class Property* find_property(const Name& name, bool recursive = false);
        const GroupedPropertiesMap& grouped_properties() const;
        static const StructMap& struct_map();


        template<typename... Args>
        Struct& add_properties(Args&&... args)
        {
            (add_property(std::forward<Args>(args)), ...);
            return *this;
        }


        template<typename Type>
        Struct& setup_struct_constuctor()
        {
            if (m_struct_constructor == nullptr && !is_class())
            {
                m_struct_constructor = static_void_constructor_of<Type>;
            }

            return *this;
        }

        virtual ~Struct();
    };

#define implement_struct(namespace_name, struct_name, parent_struct_name)                                                        \
    Engine::ReflectionInitializeController initialize_##struct_name = Engine::ReflectionInitializeController(                    \
            []() { Engine::Struct::create(ENTITY_INITIALIZER_NAME(struct_name, namespace_name), #parent_struct_name); },         \
            ENTITY_INITIALIZER_NAME(struct_name, namespace_name))
}// namespace Engine
