#include <Core/init.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/string_convert.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/material.hpp>
#include <Graphics/octree.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/textured_object.hpp>
#include <cassert>
#include <chrono>
#include <iostream>


int test()
{
    using namespace Engine;

    Object::new_instance<Package>(L"Test Package")
            ->add_object(&Object::new_instance<Texture>()->name(L"Texture Example"));

    std::clog << Object::find_object(L"Test Package::Texture Example") << std::endl;

    std::clog << "\n\n\n#################################################################" << std::endl;
    return 0;
}
