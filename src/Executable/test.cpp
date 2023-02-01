#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <iostream>

int test()
{
    std::clog << Engine::Strings::to_std_string(Engine::Object::decode_name(STR("_ZTVN6Engine17VulkanFramebufferE")))
              << std::endl;
    return 0;
}
