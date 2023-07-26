#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/shader_compiler.hpp>
#include <Core/shader_types.hpp>


Engine::String fragment_code = R"(#version 310 es
precision highp float;

layout(location = 0) in vec3 in_position_0;
layout(location = 1) in vec2 in_coord_0;
layout(location = 2) in mat4 in_coord_3;
layout(location = 0) out vec2 out_coord;
layout(location = 1) out vec4 out_position;

layout(binding = 0) uniform CameraUBO
{
    mat4 projview;
} camera_ubo;

layout(binding = 1) uniform ModelUBO
{
    mat4 model;
} model_ubo;


void main()
{
    out_position = vec4(in_position_0.xyz, 1.0) + (in_coord_3 * vec4(1.0));
    gl_Position = camera_ubo.projview * model_ubo.model * vec4(in_position_0.xyz, 1.0);
    out_coord = in_coord_0;
}
)";

namespace Engine
{
    class TestCompiler : public CommandLet
    {
    public:
        using Super = CommandLet;
        int_t execute(int_t argc, char** argv) override
        {
            ShaderCompiler* compiler = ShaderCompiler::load_compiler();

            PipelineCreateInfo info;

            if (compiler->compile(fragment_code, ShaderStage::Vertex, info.binaries.vertex))
            {
                info_log("TestCompiler", "OK!");
            }

            compiler->update_reflection(&info);
            return 0;
        }
    };

    static void on_init()
    {
        register_class(TestCompiler);
    }

    static InitializeController initializer(on_init);
}// namespace Engine
