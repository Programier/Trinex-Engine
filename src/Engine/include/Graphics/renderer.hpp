#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{

    namespace RHI
    {
        struct ApiInterface;
    }

    class ENGINE_EXPORT Renderer
    {
    private:
        RHI::ApiInterface* _M_api = nullptr;

    protected:
        Renderer(RHI::ApiInterface* interface);
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
