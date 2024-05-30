#pragma once
#include <Core/enums.hpp>
#include <Graphics/material_parameter_types.hpp>

namespace Engine
{
    struct ViewPort {
        Point2D pos = {0.0f, 0.0f};
        Size2D size;
        float min_depth = 0.0f;
        float max_depth = 1.0f;


        FORCE_INLINE float aspect() const
        {
            return size.x / size.y;
        }

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


    struct ENGINE_EXPORT BindLocation {
        union
        {
            struct {
                BindingIndex binding;
                BindingIndex set;
            };

            uint16_t id = 0;
        };

        static const BindLocation undefined;

        BindLocation();
        BindLocation(BindingIndex in_binding, BindingIndex in_set = 0);
        bool operator==(const BindLocation& location) const;
        bool operator!=(const BindLocation& location) const;
        bool operator<(const BindLocation& location) const;
        bool operator<=(const BindLocation& location) const;
        bool operator>(const BindLocation& location) const;
        bool operator>=(const BindLocation& location) const;
        bool is_valid() const;
    };

    struct ENGINE_EXPORT ShaderDefinition {
        String key;
        String value;
    };

    ENGINE_EXPORT bool operator&(class Archive& ar, ShaderDefinition& definition);

    class ENGINE_EXPORT MaterialScalarParametersInfo final
    {
        BindingIndex m_binding_index = 255;

    public:
        FORCE_INLINE bool has_parameters() const
        {
            return m_binding_index < 255;
        }

        FORCE_INLINE BindingIndex bind_index() const
        {
            return m_binding_index;
        }

        FORCE_INLINE MaterialScalarParametersInfo& bind_index(BindingIndex index)
        {
            m_binding_index = index;
            return *this;
        }

        FORCE_INLINE MaterialScalarParametersInfo& remove_parameters()
        {
            m_binding_index = 255;
            return *this;
        }

        friend bool operator&(Archive& ar, MaterialScalarParametersInfo& info);
    };

    ENGINE_EXPORT bool operator&(Archive& ar, MaterialScalarParametersInfo& info);

    struct ENGINE_EXPORT MaterialParameterInfo {
        MaterialParameterType type;
        String name;
        size_t size;
        size_t offset;
        BindLocation location;

        MaterialParameterInfo();
    };

    ENGINE_EXPORT bool operator&(Archive& ar, MaterialParameterInfo& info);

}// namespace Engine
