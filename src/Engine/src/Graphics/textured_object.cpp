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

    void TexturedObject::render_layer(const glm::mat4& prev_model)
    {
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

        shd::shader.use()
                .set(shd::model, prev_model * this->_M_model.get())
                .set(shd::projview, active_camera.projview)
                .set("lighting", 0);

        _M_diffuse.bind(0);
        BasicMesh::draw(Engine::Primitive::TRIANGLE);
    }

    const Texture2D& TexturedObject::diffuse_texture() const
    {
        return _M_diffuse;
    }

    TexturedObject& TexturedObject::diffuse_texture(const Texture2D& texture)
    {
        _M_diffuse = texture;
        return *this;
    }
}// namespace Engine
