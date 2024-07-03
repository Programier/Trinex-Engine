#include <Core/struct.hpp>
#include <d3d11_api.hpp>

namespace Engine
{
    D3D11* D3D11::m_instance = nullptr;

    implement_struct(Engine::RHI, D3D11, ).push([]() {
        Struct::static_find("Engine::RHI::D3D11", true)->struct_constructor([]() -> void* {
            if (D3D11::m_instance == nullptr)
                D3D11::m_instance = new D3D11();
            return D3D11::m_instance;
        });
    });

    D3D11::D3D11()
    {}

    D3D11& D3D11::initialize(Window* window)
    {
        return *this;
    }

    D3D11::~D3D11()
    {}
}// namespace Engine
