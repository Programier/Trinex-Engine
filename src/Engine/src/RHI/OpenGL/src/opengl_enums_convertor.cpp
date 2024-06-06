#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <opengl_enums_convertor.hpp>

namespace Engine
{
    GLuint compare_mode(CompareMode mode)
    {
        return mode == CompareMode::RefToTexture ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE;
    }

    GLuint compare_func(CompareFunc func)
    {
        switch (func)
        {
            case CompareFunc::Always:
                return GL_ALWAYS;
            case CompareFunc::Lequal:
                return GL_LEQUAL;
            case CompareFunc::Gequal:
                return GL_GEQUAL;
            case CompareFunc::Less:
                return GL_LESS;
            case CompareFunc::Greater:
                return GL_GREATER;
            case CompareFunc::Equal:
                return GL_EQUAL;
            case CompareFunc::NotEqual:
                return GL_NOTEQUAL;
            case CompareFunc::Never:
                return GL_NEVER;
        }

        return GL_ALWAYS;
    }

    GLuint wrap_from(WrapValue value)
    {
        switch (value)
        {
            case WrapValue::Repeat:
                return GL_REPEAT;

            case WrapValue::MirroredRepeat:
                return GL_MIRRORED_REPEAT;

            case WrapValue::ClampToBorder:
                return GL_CLAMP_TO_BORDER;

            case WrapValue::ClampToEdge:
                return GL_CLAMP_TO_EDGE;

#if USING_OPENGL_CORE
            case WrapValue::MirrorClampToEdge:
                return GL_MIRROR_CLAMP_TO_EDGE;
#endif

            default:
                throw EngineException("Unsupported wrap mode");
        }
    }

    GLuint blend_func(BlendFunc func)
    {
        switch (func)
        {
            case BlendFunc::Zero:
                return GL_ZERO;

            case BlendFunc::One:
                return GL_ONE;

            case BlendFunc::SrcColor:
                return GL_SRC_COLOR;

            case BlendFunc::OneMinusSrcColor:
                return GL_ONE_MINUS_SRC_COLOR;

            case BlendFunc::DstColor:
                return GL_DST_COLOR;

            case BlendFunc::OneMinusDstColor:
                return GL_ONE_MINUS_DST_COLOR;

            case BlendFunc::SrcAlpha:
                return GL_SRC_ALPHA;

            case BlendFunc::OneMinusSrcAlpha:
                return GL_ONE_MINUS_SRC_ALPHA;

            case BlendFunc::DstAlpha:
                return GL_DST_ALPHA;

            case BlendFunc::OneMinusDstAlpha:
                return GL_ONE_MINUS_DST_ALPHA;

            case BlendFunc::ConstantColor:
                return GL_CONSTANT_COLOR;
            case BlendFunc::OneMinusConstantColor:
                return GL_ONE_MINUS_CONSTANT_COLOR;
            case BlendFunc::ConstantAlpha:
                return GL_CONSTANT_ALPHA;
            case BlendFunc::OneMinusConstantAlpha:
                return GL_ONE_MINUS_CONSTANT_ALPHA;

            default:
                throw EngineException("Undefined blend function!");
        }
    }

    GLuint blend_op(BlendOp op)
    {
        switch (op)
        {
            case BlendOp::Add:
                return GL_FUNC_ADD;
            case BlendOp::Subtract:
                return GL_FUNC_SUBTRACT;
            case BlendOp::ReverseSubtract:
                return GL_FUNC_REVERSE_SUBTRACT;
            case BlendOp::Min:
                return GL_MIN;
            case BlendOp::Max:
                return GL_MAX;
            default:
                return GL_FUNC_ADD;
        }
    }


    GLuint depth_func(DepthFunc func)
    {
        switch (func)
        {
            case DepthFunc::Always:
                return GL_ALWAYS;
            case DepthFunc::Lequal:
                return GL_LEQUAL;
            case DepthFunc::Gequal:
                return GL_GEQUAL;
            case CompareFunc::Less:
                return GL_LESS;
            case DepthFunc::Greater:
                return GL_GREATER;
            case DepthFunc::Equal:
                return GL_EQUAL;
            case DepthFunc::NotEqual:
                return GL_NOTEQUAL;
            case DepthFunc::Never:
                return GL_NEVER;
        }

        return GL_ALWAYS;
    }

    GLuint stencil_op(StencilOp op)
    {
        switch (op)
        {
            case StencilOp::Keep:
                return GL_KEEP;

            case StencilOp::Zero:
                return GL_ZERO;

            case StencilOp::Replace:
                return GL_REPLACE;

            case StencilOp::Incr:
                return GL_INCR;

            case StencilOp::IncrWrap:
                return GL_INCR_WRAP;

            case StencilOp::Decr:
                return GL_DECR;

            case StencilOp::DecrWrap:
                return GL_DECR_WRAP;

            case StencilOp::Invert:
                return GL_INVERT;

            default:
                return GL_DECR;
        }
    }

    GLuint convert_topology(PrimitiveTopology topology)
    {
        switch (topology)
        {
            case PrimitiveTopology::TriangleList:
                return GL_TRIANGLES;

            case PrimitiveTopology::PointList:
                return GL_POINTS;

            case PrimitiveTopology::LineList:
                return GL_LINES;

            case PrimitiveTopology::LineStrip:
                return GL_LINE_STRIP;

            case PrimitiveTopology::TriangleStrip:
                return GL_TRIANGLE_STRIP;

            case PrimitiveTopology::TriangleFan:
                return GL_TRIANGLE_FAN;

            case PrimitiveTopology::LineListWithAdjacency:
                return GL_LINES_ADJACENCY;

            case PrimitiveTopology::LineStripWithAdjacency:
                return GL_LINE_STRIP_ADJACENCY;

            case PrimitiveTopology::TriangleListWithAdjacency:
                return GL_TRIANGLES_ADJACENCY;

            case PrimitiveTopology::TriangleStripWithAdjacency:
                return GL_TRIANGLE_STRIP_ADJACENCY;

            case PrimitiveTopology::PatchList:
                return GL_PATCHES;

            default:
                break;
        }

        throw EngineException("Unsupported topology");
    }

#if USING_OPENGL_CORE
    GLuint polygon_mode(PolygonMode mode)
    {
        switch (mode)
        {
            case PolygonMode::Fill:
                return GL_FILL;
            case PolygonMode::Line:
                return GL_LINE;
            case PolygonMode::Point:
                return GL_POINT;
            default:
            {
                return GL_FILL;
            }
        }
    }

    GLuint logic_op(LogicOp op)
    {
        switch (op)
        {
            case LogicOp::Clear:
                return GL_CLEAR;
            case LogicOp::And:
                return GL_AND;
            case LogicOp::AndReverse:
                return GL_AND_REVERSE;
            case LogicOp::Copy:
                return GL_COPY;
            case LogicOp::AndInverted:
                return GL_AND_INVERTED;
            case LogicOp::NoOp:
                return GL_NOOP;
            case LogicOp::Xor:
                return GL_XOR;
            case LogicOp::Or:
                return GL_OR;
            case LogicOp::Nor:
                return GL_NOR;
            case LogicOp::Equivalent:
                return GL_EQUAL;
            case LogicOp::Invert:
                return GL_INVERT;
            case LogicOp::OrReverse:
                return GL_OR_REVERSE;
            case LogicOp::CopyInverted:
                return GL_COPY_INVERTED;
            case LogicOp::OrInverted:
                return GL_OR_INVERTED;
            case LogicOp::Nand:
                return GL_NAND;
            case LogicOp::Set:
                return GL_SET;
            default:
                return GL_CLEAR;
        }
    }
#endif

    GLuint front_face(FrontFace face)
    {
        return face == FrontFace::CounterClockWise ? GL_CCW : GL_CW;
    }

    GLuint cull_mode(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::Back:
                return GL_BACK;

            case CullMode::Front:
                return GL_FRONT;

            case CullMode::FrontAndBack:
                return GL_FRONT_AND_BACK;

            case CullMode::None:
                return GL_NONE;

            default:
                return GL_NONE;
        }
    }
}// namespace Engine
