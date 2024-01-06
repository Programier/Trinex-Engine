#include <Core/class.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <icons.hpp>


namespace Engine::Icons
{
    Sampler* default_sampler()
    {
        static Sampler* sampler = nullptr;
        if (sampler == nullptr)
        {
            Package* pkg = Package::find_package("Editor");
            if (pkg)
            {
                sampler = pkg->find_object_checked<Sampler>("DefaultSampler");
            }
        }

        return sampler;
    }

    Texture2D* default_texture()
    {
        static Texture2D* texture = nullptr;
        if (texture == nullptr)
        {
            Package* pkg = Package::find_package("Editor");
            if (pkg)
            {
                texture = pkg->find_object_checked<Texture2D>("DefaultIcon");
            }
        }
        return texture;
    }

    ImGuiRenderer::ImGuiTexture* find_imgui_icon(class Object* object)
    {
        if (!object)
            return nullptr;

        ImGuiRenderer::Window* window = ImGuiRenderer::Window::current();
        if (!window)
            return nullptr;

        Sampler* sampler = default_sampler();
        if (!sampler)
            return nullptr;

        {
            Texture2D* texture = object->instance_cast<Texture2D>();
            if (texture && texture->has_object())
                return window->create_texture(texture, sampler);
        }

        Texture2D* texture = default_texture();
        if (texture && texture->has_object())
            return window->create_texture(texture, sampler);

        return nullptr;
    }
}// namespace Engine::Icons
