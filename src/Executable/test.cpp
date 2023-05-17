#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/pointer.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/renderer.hpp>
#include <ImGui/imgui.h>
#include <Window/window.hpp>


namespace Engine
{

    static void update(StaticMesh* mesh)
    {
        auto mesh_resource = mesh->lods[0].vertex_buffer->resources();
        auto out_size      = (mesh_resource->size() / 16) * 20;

        Buffer out_resource(out_size);

        size_t read_index  = 0;
        size_t write_index = 0;


        while (read_index < mesh_resource->size())
        {
            for (int i = 0; i < 8; i++)
            {
                out_resource[write_index++] = (*mesh_resource)[read_index++];
            }

            for (int i = 0; i < 4; i++) out_resource[write_index++] = 0.0f;

            for (int i = 0; i < 8; i++)
            {
                out_resource[write_index++] = (*mesh_resource)[read_index++];
            }
        }

        (*mesh_resource) = std::move(out_resource);
    }

    class ImGuiTest : public Engine::CommandLet
    {
    public:
        int execute(int argc, char** argv)
        {
            Package* package = Object::load_package("TestResources");

            for (auto& pair : package->objects())
            {
                if (pair.second->is_instance_of<StaticMesh>())
                {
                    update(pair.second->instance_cast<StaticMesh>());
                }
            }

            package->save();

            return 0;
        }
    };


    register_class(ImGuiTest, Engine::CommandLet);
}// namespace Engine
