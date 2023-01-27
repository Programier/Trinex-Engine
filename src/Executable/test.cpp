#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <iostream>

int test()
{
    std::clog << Engine::Strings::to_std_string(Engine::Object::decode_name(STR("_ZTVN6Engine9VulkanAPIE")))
              << std::endl;
    return 0;
}
