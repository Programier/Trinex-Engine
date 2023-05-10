#include <Graphics/renderer.hpp>
#include <api.hpp>


namespace Engine
{
    Renderer::Renderer(GraphicApiInterface::ApiInterface* interface) : _M_api(interface)
    {}

    Renderer& Renderer::begin()
    {
        _M_api->begin_render();
        return *this;
    }

    Renderer& Renderer::end()
    {
        _M_api->end_render();
        return *this;
    }

    String Renderer::name() const
    {
        return _M_api->renderer();
    }

    Renderer& Renderer::wait_idle()
    {
        _M_api->wait_idle();
        return *this;
    }

    Renderer& Renderer::draw_indexed(size_t count, size_t offset)
    {
        _M_api->draw_indexed(count, offset);
        return *this;
    }

    Renderer::~Renderer()
    {}
}// namespace Engine
