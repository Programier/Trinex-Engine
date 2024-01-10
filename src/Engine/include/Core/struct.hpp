#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class ENGINE_EXPORT Struct
    {
    public:
        using GroupedPropertiesMap = TreeMap<Name, Vector<class Property*>, Name::Less>;

    private:
        void* (*_M_struct_constructor)() = nullptr;

    protected:
        String _M_base_name_splitted;

        Name _M_full_name;
        Name _M_namespace_name;
        Name _M_base_name;
        Name _M_parent;

        mutable Struct* _M_parent_struct = nullptr;

        Vector<class Property*> _M_properties;
        GroupedPropertiesMap _M_grouped_properties;

        Struct(const Name& name, const Name& namespace_name, const Name& parent = Name::none, void* (*constructor)() = nullptr);
        Struct(const Name& name, const Name& namespace_name, Struct* parent, void* (*constructor)() = nullptr);

    public:
        static ENGINE_EXPORT Struct* create(const Name& name, const Name& namespace_name, const Name& parent = Name::none,
                                            void* (*constructor)() = nullptr);
        static ENGINE_EXPORT Struct* static_find(const String& name, bool requred = false);

        const String& base_name_splitted() const;
        const Name& name() const;
        const Name& namespace_name() const;
        const Name& base_name() const;
        const Name& parent_name() const;
        Struct* parent() const;
        virtual void* create_struct() const;

        bool is_a(const Struct* other) const;
        virtual bool is_class() const;

        Struct& add_property(Property* prop);
        const Vector<class Property*>& properties() const;
        const GroupedPropertiesMap& grouped_properties() const;

        template<typename... Args>
        Struct& add_properties(Args&&... args)
        {
            (add_property(std::forward<Args>(args)), ...);
            return *this;
        }

        virtual ~Struct();
    };

#define implement_struct(struct_name, namespace_name, parent_struct_name)                                                        \
    Engine::InitializeController initialize_##struct_name = Engine::InitializeController(                                        \
            []() {                                                                                                               \
                Engine::Struct::create(#struct_name, #namespace_name, #parent_struct_name,                                       \
                                       []() -> void* { return new CONCAT_TYPE_AND_NAMESPACE(struct_name, namespace_name)(); });  \
            },                                                                                                                   \
            ENTITY_INITIALIZER_NAME(struct_name, namespace_name))
}// namespace Engine
