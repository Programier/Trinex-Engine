#pragma once
#include <BasicFunctional/dynamic_array.hpp>
#include <Image/image.hpp>
#include <list>
#include <vector>


namespace Engine
{
    namespace basic_texturearray
    {
        ObjectID gen_texture_array(const std::vector<Image*>& images, const Size2D& max_size);
        ObjectID gen_texture_array(const std::list<Image*>& images, const Size2D& max_size);
        ObjectID gen_texture_array(const DynamicArray<Image*>& images, const Size2D& max_size);
        ObjectID gen_texture_array(const std::vector<Image>& images, const Size2D& max_size);
        ObjectID gen_texture_array(const std::list<Image>& images, const Size2D& max_size);
        ObjectID gen_texture_array(const DynamicArray<Image>& images, const Size2D& max_size);
    }// namespace basic_texturearray
}// namespace Engine
