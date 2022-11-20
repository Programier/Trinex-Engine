#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Graphics/scene.hpp>


using namespace Engine;

static Scene* _M_active_scene = nullptr;

Scene::Scene()
{
    name(L"Scene");
}

DrawableObject* Scene::copy() const
{
    throw not_implemented;
}

ENGINE_EXPORT Scene* Scene::get_active_scene()
{
    return _M_active_scene;
}

Scene& Scene::set_as_active_scene()
{
    _M_active_scene = this;
    return *this;
}

bool Scene::is_active() const
{
    return this == _M_active_scene;
}


const std::unordered_set<Camera*>& Scene::cameras() const
{
    return _M_cameras;
}

Scene& Scene::add_camera(Camera* camera)
{
    _M_cameras.insert(camera);
    return *this;
}

Scene::ActiveCamera& Scene::active_camera()
{
    return _M_active_camera;
}

void Scene::ActiveCamera::update_info()
{
    projection = camera->projection();
    view = camera->view();
    projview = projection * view;
}


Scene& Scene::active_camera(const std::wstring& name)
{
    for (Camera* camera : _M_cameras)
    {
        if (camera->name == name)
        {
            _M_active_camera.camera = camera;
            _M_active_camera.update_info();
            return *this;
        }
    }

    logger->log("Can't find camera with name '%s'\n", Strings::to_string(name).c_str());
    return *this;
}

Scene& Scene::active_camera(Camera* find_camera)
{
    for (Camera* camera : _M_cameras)
    {
        if (camera == find_camera)
        {
            _M_active_camera.camera = camera;
            _M_active_camera.update_info();
            return *this;
        }
    }

    logger->log("Can't find camera with name '%s'\n", Strings::to_string(find_camera->name).c_str());
    return *this;
}

bool Scene::is_empty_layer() const
{
    return true;
}


#ifdef ENABLE_RENDER
void Scene::render() const
{}
#endif

Scene::~Scene()
{
    for (Camera* camera : _M_cameras)
    {
        delete camera;
    }
}
