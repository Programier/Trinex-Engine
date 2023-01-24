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

Shader sh_s::Anim::shader;
const std::string sh_s::Anim::projview = "projview";
const std::string sh_s::Anim::model = "model";
const std::string sh_s::Anim::transposed_inversed_model = "transposed_inversed_model";

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


static void copy_data(FileBuffer& buffer, const std::string& data)
{
    buffer.clear();
    buffer.resize(data.size() + 1);
    buffer.back() = 0;
    std::copy(data.begin(), data.end(), buffer.begin());
}

#define init_system(ns, type, params)                                                                                       \
    copy_data(params.vertex, type##_shader_vert);                                                                           \
    copy_data(params.fragment, type##_shader_frag);                                                                         \
    params.name = #ns;                                                                                                      \
    sh_s::ns::shader.load(params);

void sh_s::init()
{
    if (inited)
        return;

    ShaderParams params;
    params.source_type = ShaderSourceType::Text;

    init_system(SkyBox, skybox, params);
    init_system(Text, text, params);
    init_system(Scene, scene, params);
    init_system(Line, line, params);
    init_system(Depth, depth, params);
    init_system(DepthRenderer, DepthRender, params);
    init_system(Anim, anim, params);

    inited = true;
}
