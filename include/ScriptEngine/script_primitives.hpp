#pragma once
#include <Core/engine_types.hpp>

#define declare_primitive(name, type)                                                                                            \
	class name final                                                                                                             \
	{                                                                                                                            \
		mutable size_t m_refs = 0;                                                                                               \
                                                                                                                                 \
	public:                                                                                                                      \
		type value;                                                                                                              \
		name(type initial = type());                                                                                             \
		name(const name&);                                                                                                       \
		name& operator=(const name&);                                                                                            \
		void add_ref() const;                                                                                                    \
		void release() const;                                                                                                    \
		type op_conv() const;                                                                                                    \
	}

namespace Trinex
{
	declare_primitive(Boolean, bool);
	declare_primitive(Integer8, i8);
	declare_primitive(Integer16, i16);
	declare_primitive(Integer32, i32);
	declare_primitive(Integer64, i64);
	declare_primitive(UnsignedInteger8, u8);
	declare_primitive(UnsignedInteger16, u16);
	declare_primitive(UnsignedInteger32, u32);
	declare_primitive(UnsignedInteger64, u64);
	declare_primitive(Float, float);
	declare_primitive(Double, double);
}// namespace Trinex

#undef declare_primitive
