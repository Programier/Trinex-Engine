#include <Graphics/shader_system.hpp>
#include <shader_code.hpp>

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
    if (inited)
        return;

    sh_s::SkyBox::shader.load(skybox_shader_vert, skybox_shader_frag, false);
    sh_s::Text::shader.load(text_shader_vert, text_shader_frag, false);
    sh_s::Scene::shader.load(scene_shader_vert, scene_shader_frag, false);
    sh_s::Line::shader.load(line_shader_vert, line_shader_frag, false);
    sh_s::Depth::shader.load(depth_shader_vert, depth_shader_frag, false);
    sh_s::DepthRenderer::shader.load(DepthRender_shader_vert, DepthRender_shader_frag, false);

    inited = true;
}
