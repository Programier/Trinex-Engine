#include <Core/class.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <icons.hpp>


namespace Engine
{
    static Sampler* default_sampler()
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

    void* find_imgui_icon(class Object* object)
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
                return window->create_texture(texture, sampler)->handle();
        }

        return nullptr;
    }
}// namespace Engine
