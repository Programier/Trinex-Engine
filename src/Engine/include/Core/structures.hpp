#pragma once
#include <Core/enums.hpp>

namespace Engine
{
    struct SwizzleRGBA {
        SwizzleValue R = SwizzleValue::Identity;
        SwizzleValue G = SwizzleValue::Identity;
        SwizzleValue B = SwizzleValue::Identity;
        SwizzleValue A = SwizzleValue::Identity;
    };

    struct ViewPort {
        Point2D pos = {0.0f, 0.0f};
        Size2D size;
        float min_depth = 0.0f;
        float max_depth = 1.0f;

        FORCE_INLINE bool operator==(const ViewPort& v) const
        {
            return pos == v.pos && size == v.size && min_depth == v.min_depth && max_depth == v.max_depth;
        }

        FORCE_INLINE bool operator!=(const ViewPort& v) const
        {
            return !((*this) == v);
        }
    };

    struct Scissor {
        Point2D pos = {0.0f, 0.0f};
        Size2D size;

        FORCE_INLINE bool operator==(const Scissor& s) const
        {
            return pos == s.pos && size == s.size;
        }

        FORCE_INLINE bool operator!=(const Scissor& s) const
        {
            return !((*this) == s);
        }
    };

    struct DepthStencilClearValue {
        float depth  = 1.0;
        byte stencil = 0.0;
    };


    struct ClassFieldInfo {
        const char* name;
        AccessType access;
        bool is_serializable;
    };

    struct EmptyStruct {
    };

    class EmptyClass
    {
    };

    template<typename Type>
    class SizeLimits
    {
    public:
        Type min;
        Type max;

        SizeLimits() = default;
        SizeLimits(const Type& _min, const Type& _max) : min(_min), max(_max)
        {}

        SizeLimits(Type&& _min, Type&& _max) : min(std::move(_min)), max(std::move(_max))
        {}

        SizeLimits(SizeLimits&&)      = default;
        SizeLimits(const SizeLimits&) = default;


        SizeLimits& operator=(const SizeLimits&) = default;
        SizeLimits& operator=(SizeLimits&&)      = default;
    };

    using SizeLimits1D = SizeLimits<Size1D>;
    using SizeLimits2D = SizeLimits<Size2D>;
    using SizeLimits3D = SizeLimits<Size3D>;


    using AABB_1D = SizeLimits1D;
    using AABB_2D = SizeLimits2D;
    using AABB_3D = SizeLimits3D;

    class ColorFormatInfo
    {
    private:
        EnumerateType _M_value;

    public:
        FORCE_INLINE byte components() const
        {
            return static_cast<byte>(_M_value & byte(7));
        }

        FORCE_INLINE byte component_size() const
        {
            return static_cast<byte>((_M_value >> 3) & byte(7));
        }

        FORCE_INLINE ColorFormatMetaData metadata() const
        {
            return static_cast<ColorFormatMetaData>((_M_value >> 6) & byte(15));
        }

        FORCE_INLINE ColorFormatAspect aspect() const
        {
            return static_cast<ColorFormatAspect>((_M_value >> 10) & byte(7));
        }

        static FORCE_INLINE ColorFormatInfo info_of(ColorFormat format)
        {
            ColorFormatInfo info;
            info._M_value = static_cast<EnumerateType>(format);
            return info;
        }
    };

}// namespace Engine