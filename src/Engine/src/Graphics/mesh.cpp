#include <Core/buffer_manager.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/mesh.hpp>

namespace Engine
{
    static const Array<size_t, 8> semantic_sizes = {
            sizeof(Point3D),    // Vertex
            sizeof(Point2D),    // TexCoord
            sizeof(Vector4D),   // Color
            sizeof(Vector3D),   // Normal
            sizeof(Vector3D),   // Tangent
            sizeof(Vector3D),   // Binormal,
            sizeof(Vector4D),   // BlendWeight
            sizeof(IntVector4D),// BlendIndices
    };

    size_t StaticMeshSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index)
    {
        return 0;
    }
}// namespace Engine
