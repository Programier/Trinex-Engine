#pragma once
#include <Graphics/camera.hpp>
#include <Graphics/drawable_object.hpp>
#include <Graphics/texture.hpp>
#include <TemplateFunctional/octree.hpp>


namespace Engine
{
    CLASS Scene : public DrawableObject
    {

    public:
        struct ActiveCamera {
            Camera* camera = nullptr;
            glm::mat4 projection;
            glm::mat4 view;
            glm::mat4 projview;

        public:
            void update_info();

        private:
        };

    private:
        // Cameras parameters
        std::unordered_set<Camera*> _M_cameras;
        ActiveCamera _M_active_camera;

        Octree<DrawableObject*> _M_octree;

    public:
        Scene();
        DrawableObject* copy() const override;
        static ENGINE_EXPORT Scene* get_active_scene();
        Scene& set_as_active_scene();
        bool is_active() const;

        const std::unordered_set<Camera*>& cameras() const;
        Scene& add_camera(Camera * camera);
        ActiveCamera& active_camera();
        Scene& active_camera(const std::wstring& name);
        Scene& active_camera(Camera * camera);

        ~Scene();
        friend class DrawableObject;
    };
}// namespace Engine
