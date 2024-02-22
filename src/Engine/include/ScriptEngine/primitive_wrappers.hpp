#pragma once
#include <Core/engine_types.hpp>


#define declare_primitive(name, type)                                                                                  \
    class name final                                                                                                   \
    {                                                                                                                  \
        mutable size_t m_refs = 0;                                                                                    \
                                                                                                                       \
    public:                                                                                                            \
        type value;                                                                                                    \
        name(type initial);                                                                                            \
        name(const name&);                                                                                             \
        name& operator=(const name&);                                                                                  \
        void add_ref() const;                                                                                          \
        void release() const;                                                                                          \
        type op_conv() const;                                                                                          \
    }

declare_primitive(Boolean, bool);
declare_primitive(Integer8, Engine::int8_t);
declare_primitive(Integer16, Engine::int16_t);
declare_primitive(Integer, Engine::int32_t);
declare_primitive(Integer64, Engine::int64_t);
declare_primitive(UnsignedInteger8, Engine::uint8_t);
declare_primitive(UnsignedInteger16, Engine::uint16_t);
declare_primitive(UnsignedInteger, Engine::uint32_t);
declare_primitive(UnsignedInteger64, Engine::uint64_t);
declare_primitive(Float, float);
declare_primitive(Double, double);

#undef declare_primitive
