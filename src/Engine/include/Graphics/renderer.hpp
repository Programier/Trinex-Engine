#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{

    namespace GraphicApiInterface
    {
        class ApiInterface;
    }

    class ENGINE_EXPORT Renderer
    {
    private:
        GraphicApiInterface::ApiInterface* _M_api = nullptr;

    protected:
        Renderer(GraphicApiInterface::ApiInterface* interface);
        virtual ~Renderer();

    public:
        String name() const;
        Renderer& begin();
        Renderer& end();
        Renderer& wait_idle();
        Renderer& draw_indexed(size_t count, size_t offset);

        friend class EngineInstance;
    };
}// namespace Engine