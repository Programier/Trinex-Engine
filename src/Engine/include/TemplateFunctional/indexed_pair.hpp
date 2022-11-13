#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{
    template<typename Type1, typename Type2>
    struct Pair
    {
        Type1 first;
        Type2 second;

        Pair() =default;
        Pair(const Type1& value, const Type2& value2) : first(value), second(value2){}

        byte* operator[](bool get_second)
        {
            if(get_second)
                return static_cast<byte*>(&second);
            return static_cast<byte*>(&first);
        }
    };
}
