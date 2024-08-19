#pragma once
#include <Core/engine_types.hpp>

#define declare_primitive(name, type)                                                                                            \
	class name final                                                                                                             \
	{                                                                                                                            \
		mutable size_t m_refs = 0;                                                                                               \
                                                                                                                                 \
	public:                                                                                                                      \
		type value;                                                                                                              \
		name(type initial);                                                                                                      \
		name(const name&);                                                                                                       \
		name& operator=(const name&);                                                                                            \
		void add_ref() const;                                                                                                    \
		void release() const;                                                                                                    \
		type op_conv() const;                                                                                                    \
	}

namespace Engine
{
	declare_primitive(Boolean, bool);
	declare_primitive(Integer8, int8_t);
	declare_primitive(Integer16, int16_t);
	declare_primitive(Integer32, int32_t);
	declare_primitive(Integer64, int64_t);
	declare_primitive(UnsignedInteger8, uint8_t);
	declare_primitive(UnsignedInteger16, uint16_t);
	declare_primitive(UnsignedInteger32, uint32_t);
	declare_primitive(UnsignedInteger64, uint64_t);
	declare_primitive(Float, float);
	declare_primitive(Double, double);
}// namespace Engine

#undef declare_primitive
