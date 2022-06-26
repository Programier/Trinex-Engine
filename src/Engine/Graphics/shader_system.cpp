#include <Graphics/shader_system.hpp>

namespace sh_s = Engine::ShaderSystem;
using namespace Engine;

static bool inited = false;

// Skybox
Shader sh_s::SkyBox::shader;
const std::string sh_s::SkyBox::projview = "projview";

// Text

Shader sh_s::Text::shader;
const std::string sh_s::Text::projview = "projview";
const std::string sh_s::Text::color = "color";

// Scene

Shader sh_s::Scene::shader;
const std::string sh_s::Scene::projview = "projview";
const std::string sh_s::Scene::model = "model";
const std::string sh_s::Scene::light_projview = "light_projview";
const std::string sh_s::Scene::camera_pos = "camera";
const std::string sh_s::Scene::lighting = "lighting";
const std::string sh_s::Scene::transposed_inversed_model = "transposed_inversed_model";

// Line
Shader sh_s::Line::shader;
const std::string sh_s::Line::projview = "projview";
const std::string sh_s::Line::model = "model";
const std::string sh_s::Line::color = "color";

// Depth
Shader sh_s::Depth::shader;
const std::string sh_s::Depth::projview = "projview";
const std::string sh_s::Depth::model = "model";

// Depth Renderer

Shader sh_s::DepthRenderer::shader;
const std::string sh_s::DepthRenderer::power = "power";

void sh_s::init()
{
    //    if (inited)
    //        return;

    sh_s::SkyBox::shader.load("Shaders/skybox/shader.vert", "Shaders/skybox/shader.frag");
    sh_s::Text::shader.load("Shaders/text/shader.vert", "Shaders/text/shader.frag");
    sh_s::Scene::shader.load("Shaders/scene/shader.vert", "Shaders/scene/shader.frag");
    sh_s::Line::shader.load("Shaders/line/shader.vert", "Shaders/line/shader.frag");
    sh_s::Depth::shader.load("Shaders/depth/shader.vert", "Shaders/depth/shader.frag");
    sh_s::DepthRenderer::shader.load("Shaders/DepthRender/shader.vert", "Shaders/DepthRender/shader.frag");
    inited = true;
}
