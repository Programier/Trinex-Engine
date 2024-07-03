#include <no_api.hpp>


namespace Engine
{
    class D3D11 : public NoApi
    {
    public:
        static D3D11* m_instance;

        D3D11();
        D3D11& initialize(Window* window) override;
        ~D3D11();
    };
}// namespace Engine
