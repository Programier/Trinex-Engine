#include <glm_bindings.hpp>

namespace Engine
{
    static void on_init()
    {
        initialize_matrix<Matrix4f>("Engine::Matrix4f");
        initialize_matrix<Matrix3f>("Engine::Matrix3f");
        initialize_matrix<Matrix2f>("Engine::Matrix2f");
    }

    static InitializeController initializer(on_init);
}// namespace Engine
