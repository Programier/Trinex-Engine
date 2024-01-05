#include <Core/entry_point.hpp>
#include <Core/class.hpp>
#include <Core/engine_config.hpp>

#include <Core/logger.hpp>
#include <Engine/ray.hpp>
#include <Engine/aabb.hpp>


namespace Engine
{
    class Main : public EntryPoint
    {
        declare_class(Main, EntryPoint);

    public:

        EntryPoint& load_configs() override
        {
            new (&engine_config) EngineConfig();
            return *this;
        }

        int_t execute(int_t argc, char** argv) override
        {
            AABB_3Df aabb1 = {{-1, -1, -1}, {1, 1, 1}};

            Vector2D intersection = aabb1.intersect(Ray{{-10, 0, 0}, {0.1, 1, 0}});
            info_log("AABB", "%f -> %f", intersection.x, intersection.y);
            return 0;
        }
    };

    implement_engine_class_default_init(Main);
}
