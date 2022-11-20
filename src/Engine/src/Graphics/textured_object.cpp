#include <Graphics/scene.hpp>
#include <Graphics/shader_system.hpp>
#include <Graphics/textured_object.hpp>

namespace Engine
{
    TexturedObject::TexturedObject()
    {}

    DrawableObject* TexturedObject::copy() const
    {
        throw not_implemented;
    }

    void TexturedObject::render_layer(const glm::mat4& prev_model, on_render_layer_func on_render_layer) const
    {
        if (!_M_visible || is_empty_layer())
            return;

        Scene* scene = Scene::get_active_scene();
        if (!scene)
        {
            throw std::runtime_error("No active scene found!");
        }

        Scene::ActiveCamera& active_camera = scene->active_camera();

        if (!active_camera.camera)
        {
            throw std::runtime_error("No active camera found!");
        }

        namespace shd = ShaderSystem::Scene;

        const auto model = prev_model * get_cached_matrix();
        shd::shader.use().set(shd::model, model).set(shd::projview, active_camera.projview).set("lighting", 0);

        _M_diffuse.bind(0);
        BasicMesh::draw(Engine::Primitive::TRIANGLE);
        on_render_layer(this, prev_model);
    }

    const Texture2D& TexturedObject::diffuse_texture() const
    {
        return _M_diffuse;
    }

    bool TexturedObject::is_empty_layer() const
    {
        return data.empty();
    }

    TexturedObject& TexturedObject::diffuse_texture(const Texture2D& texture)
    {
        _M_diffuse = texture;
        return *this;
    }
}// namespace Engine
