#include <glm_bindings.hpp>

namespace Engine
{
    static void on_init()
    {
        initialize_vector<Vector1D>("Engine::Vector1D");
        initialize_vector<Vector2D>("Engine::Vector2D");
        initialize_vector<Vector3D>("Engine::Vector3D");
        initialize_vector<Vector4D>("Engine::Vector4D");
    }

    static InitializeController initializer(on_init);

}// namespace Engine
