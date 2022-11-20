#include <Application.hpp>
#include <Core/logger.hpp>
#include <Graphics/octree.hpp>
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


void print_sizes(Octree<int>::OctreeNode* node)
{

    byte i = 0;
    bool has_obj = false;
    for (; i < 8; i++)
    {
        auto tmp = node->get(i);
        if (tmp)
        {
            print_sizes(tmp);
            has_obj = true;
        }
    }

    if (has_obj)
    {
        std::clog << node->box().half_size() << std::endl;
    }
}

#include <chrono>

void octree_test()
{
    BoxHB box({0, 0, 0}, {1, 1, 1});

    box = box.apply_model(glm::scale(Constants::identity_matrix, {10, 10, 10}));
    std::clog << box.center() << "\t" << box.half_size() << std::endl;
}


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
