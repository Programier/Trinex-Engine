#include <Graphics/scene.hpp>
#include <engine.hpp>

using namespace Engine;

Scene::Scene() = default;

Scene& Scene::load(const std::string& filename)
{
    throw not_implemented;
}

Scene& Scene::render() const
{
    throw not_implemented;
}