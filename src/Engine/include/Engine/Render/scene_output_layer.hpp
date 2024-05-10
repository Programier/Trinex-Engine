#include <Engine/Render/batched_primitives.hpp>
#include <Engine/Render/command_buffer.hpp>


namespace Engine
{
    class ENGINE_EXPORT SceneOutputLayer : public CommandBufferLayer
    {

    private:
        SceneOutputLayer& render_light_octree(class Scene* scene);
        SceneOutputLayer& render_primitive_octree(class Scene* scene);

    public:
        BatchedLines lines;
        BatchedTriangles triangles;

        SceneOutputLayer& clear() override;
        SceneOutputLayer& begin_render(SceneRenderer* renderer, RenderTargetBase* render_target) override;
        SceneOutputLayer& render(SceneRenderer*, RenderTargetBase*) override;
        SceneOutputLayer& end_render(SceneRenderer* renderer, RenderTargetBase* render_target) override;
    };
}// namespace Engine
