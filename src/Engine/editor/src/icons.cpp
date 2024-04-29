#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <icons.hpp>
#include <editor_resources.hpp>


namespace Engine::Icons
{
    static Sampler* m_default_sampler              = nullptr;
    static Texture2D* m_icons[IconType::__COUNT__] = {0};

    Sampler* default_sampler()
    {
        return m_default_sampler;
    }

    Texture2D* default_texture()
    {
        return m_icons[IconType::Default];
    }

    Texture2D* icon(IconType type)
    {
        return m_icons[type];
    }

    Texture2D* find_imgui_icon(class Object* object)
    {
        if (object)
        {
            if (Texture2D* texture = object->instance_cast<Texture2D>())
                return texture;
        }

        return default_texture();
    }


    void on_editor_package_loaded()
    {
        m_default_sampler = EditorResources::default_sampler;
        trinex_always_check(m_default_sampler, "Editor default sampler can't be null!");
        m_icons[IconType::Default] = EditorResources::default_icon;
        m_icons[IconType::Add]     = EditorResources::add_icon;
        m_icons[IconType::Remove]  = EditorResources::remove_icon;
        m_icons[IconType::Select]  = EditorResources::select_icon;
        m_icons[IconType::Move]    = EditorResources::move_icon;
        m_icons[IconType::Rotate]  = EditorResources::rotate_icon;
        m_icons[IconType::Scale]   = EditorResources::scale_icon;
        m_icons[IconType::More]    = EditorResources::more_icon;

        for (Texture2D* icon : m_icons)
        {
            trinex_always_check(icon, "Icon can't be null!");
        }
    }
}// namespace Engine::Icons
