#pragma once
#include <Graphics/imgui.hpp>

namespace Engine
{
    class ImGuiObjectProperties : public ImGuiRenderer::ImGuiAdditionalWindow
    {
    public:
        using PropertiesMap = TreeMap<Name, Vector<class Property*>>;

    private:
        Object* m_object;
        Identifier m_destroy_id;

        TreeMap<class Struct*, PropertiesMap> m_properties;
        PropertiesMap& build_props_map(Struct* self);

    public:
        CallBacks<void(Object*)> on_begin_render;

        ImGuiObjectProperties();
        ~ImGuiObjectProperties();

        bool render(RenderViewport* viewport) override;
        Struct* struct_instance() const;
        Object* object() const;

        ImGuiObjectProperties& update(Object* object);
        const PropertiesMap& properties_map(Struct* self);
        void render_struct_properties(void* object, class Struct* struct_class, bool editable = true);
        static bool collapsing_header(const void* id, const char* format, ...);
        static const char* name();
    };

}// namespace Engine
