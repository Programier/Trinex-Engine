#include <Application.hpp>
#include <Core/logger.hpp>
#include <cstdarg>
#include <fstream>
#include <iostream>

#include <Graphics/scene.hpp>

using namespace Engine;

#define PATH "resources/scene/simple_scene.gltf"

static void scene_test()
{
    Scene scene;
    scene.load(PATH);
    std::clog << scene.sub_objects().size() << std::endl;
}


std::unordered_map<std::string, void (*)()> _M_funcs = {
        {"scene_test", scene_test},
};

int main(int argc, char** argv)
{

    bool has_parameter = false;
    for (int i = 1; i < argc; i++)
    {
        try
        {
            _M_funcs.at(std::string(argv[i]))();
            has_parameter = true;
        }
        catch (...)
        {}
    }

    return has_parameter ? 0 : game_main(argc, argv);
}
