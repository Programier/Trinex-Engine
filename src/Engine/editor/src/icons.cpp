#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <icons.hpp>


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

    ImGuiRenderer::ImGuiTexture* icon(IconType type)
    {
        Texture2D* texture = m_icons[type];
        if (texture && texture->has_object())
            return ImGuiRenderer::Window::current()->create_texture(texture, m_default_sampler);
        return nullptr;
    }

    ImGuiRenderer::ImGuiTexture* find_imgui_icon(class Object* object)
    {
        ImGuiRenderer::Window* window = ImGuiRenderer::Window::current();
        if (!window)
            return nullptr;

        Sampler* sampler = default_sampler();
        if (!sampler)
            return nullptr;
        if (object)
        {
            {
                Texture2D* texture = object->instance_cast<Texture2D>();
                if (texture && texture->has_object())
                    return window->create_texture(texture, sampler);
            }
        }

        Texture2D* texture = default_texture();
        if (texture && texture->has_object())
            return window->create_texture(texture, sampler);

        return nullptr;
    }


    void on_editor_package_loaded()
    {
        auto editor = Package::find_package("Editor");
        trinex_always_check(editor, "Editor package can't be null!");

        m_default_sampler = editor->find_object_checked<Sampler>("DefaultSampler");
        trinex_always_check(m_default_sampler, "Editor default sampler can't be null!");
        m_icons[IconType::Default] = editor->find_object_checked<Texture2D>("DefaultIcon");
        m_icons[IconType::Add]     = editor->find_object_checked<Texture2D>("AddIcon");
        m_icons[IconType::Remove]  = editor->find_object_checked<Texture2D>("RemoveIcon");
        m_icons[IconType::Select]  = editor->find_object_checked<Texture2D>("SelectIcon");
        m_icons[IconType::Move]    = editor->find_object_checked<Texture2D>("MoveIcon");
        m_icons[IconType::Rotate]  = editor->find_object_checked<Texture2D>("RotateIcon");
        m_icons[IconType::Scale]   = editor->find_object_checked<Texture2D>("ScaleIcon");

        for (Texture2D* icon : m_icons)
        {
            trinex_always_check(icon, "Icon can't be null!");
        }
    }
}// namespace Engine::Icons
