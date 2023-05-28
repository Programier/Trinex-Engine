#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/file_manager.hpp>
#include <Core/package.hpp>
#include <Graphics/gltf_loader.hpp>
#include <Graphics/material.hpp>


namespace Engine
{
    static Material* create_framebuffer_material(Package* package)
    {
        try
        {
            package->objects().at("Framebuffer Material")->mark_for_delete();
        }
        catch (...)
        {}

        String t_path1 = "/home/programier/Projects/Shaders/new/framebuffer.frag";
        String t_path2 = "/home/programier/Projects/Shaders/new/framebuffer.vert";

        String b_path1 = "/home/programier/Projects/Shaders/new/framebuffer.fm";
        String b_path2 = "/home/programier/Projects/Shaders/new/framebuffer.vm";

        Material* material       = Object::new_instance_named<Material>("Framebuffer Material", package);
        PipelineCreateInfo* info = material->resources(true);

        auto read_text = [](String& path, FileBuffer& to, size_t is_text = 1) {
            to.clear();
            to.shrink_to_fit();

            FileReader reader(path);
            size_t size = reader.size();
            to.resize(size + is_text, 0);
            reader.read((byte*) to.data(), size);
        };

        read_text(t_path1, info->text.fragment, 1);
        read_text(t_path2, info->text.vertex, 1);
        read_text(b_path1, info->binaries.fragment, 0);
        read_text(b_path2, info->binaries.vertex, 0);


        ShaderUniformBuffer buffer;
        buffer.binding = 0;
        buffer.name    = "camera_ubo";
        buffer.size    = 64;

        info->uniform_buffers.push_back(buffer);

        buffer.binding = 1;
        buffer.name    = "model_ubo";
        buffer.size    = 64;

        info->uniform_buffers.push_back(buffer);

        {
            ShaderTextureSampler sampler;
            sampler.binding = 2;
            info->texture_samplers.push_back(sampler);
        }

        info->vertex_info.attributes.emplace_back("in_position_0");
        info->vertex_info.attributes.emplace_back("in_tex_coord_0");

        info->name              = "Framebuffer Shader";
        info->framebuffer_usage = 1;

        info->state.color_blending.blend_attachment.resize(4);
        return material;
    }

    static Material* create_output_material(Package* package)
    {
        try
        {
            package->objects().at("Output Material")->mark_for_delete();
        }
        catch (...)
        {}

        String t_path1 = "/home/programier/Projects/Shaders/new/output.frag";
        String t_path2 = "/home/programier/Projects/Shaders/new/output.vert";

        String b_path1 = "/home/programier/Projects/Shaders/new/output.fm";
        String b_path2 = "/home/programier/Projects/Shaders/new/output.vm";

        Material* material       = Object::new_instance_named<Material>("Output Material", package);
        PipelineCreateInfo* info = material->resources(true);

        auto read_text = [](String& path, FileBuffer& to, size_t is_text = 1) {
            to.clear();
            to.shrink_to_fit();

            FileReader reader(path);
            size_t size = reader.size();
            to.resize(size + is_text, 0);
            reader.read((byte*) to.data(), size);
        };

        read_text(t_path1, info->text.fragment, 1);
        read_text(t_path2, info->text.vertex, 1);
        read_text(b_path1, info->binaries.fragment, 0);
        read_text(b_path2, info->binaries.vertex, 0);

        {
            ShaderTextureSampler sampler;
            sampler.binding = 0;
            info->texture_samplers.push_back(sampler);

            sampler.binding = 1;
            info->texture_samplers.push_back(sampler);
        }

        info->vertex_info.attributes.emplace_back("in_position_0");
        info->vertex_info.attributes.emplace_back("in_tex_coord_0");

        info->name              = "Output Shader";
        info->framebuffer_usage = 0;

        info->state.color_blending.blend_attachment.resize(1);
        info->state.rasterizer.cull_mode = CullMode::None;
        info->state.depth_test.enable    = 0;
        return material;
    }

    static void create_materials(Package* package)
    {
        auto a = create_output_material(package);
        auto b = create_framebuffer_material(package);
        package->find_object_checked<StaticMeshComponent>("Mesh 1")->lods[0].material_reference(b);
        package->find_object_checked<StaticMeshComponent>("Mesh 2")->lods[0].material_reference(a);
        for (auto& pair : package->objects())
        {
            logger->log("OBJECT: %s", pair.first.c_str());
        };
    }


    class TestGLTF : public CommandLet
    {
    public:
        int execute(int argc, char** argv)
        {

            return 0;
        }
    };

    register_class(TestGLTF, Engine::CommandLet);
}// namespace Engine
