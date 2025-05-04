#pragma once
#include <gdfe/def.h>
#include <gdfe/math/math.h>

/// A mesh handle. All meshes should be defined with their vertices in counter-clockwise winding order.
typedef struct GDF_Mesh_T* GDF_Mesh;

typedef PACKED_STRUCT GDF_MeshVertex {
    vec3 pos;
    vec3 normal;
} GDF_MeshVertex END_PACKED_STRUCT;

#define GDF_MESH_VERTEX_SIZE (sizeof(MeshVertex))

typedef enum GDF_PRIMITIVE_MESH_TYPE {
    GDF_PRIMITIVE_MESH_TYPE_SPHERE,
    GDF_PRIMITIVE_MESH_TYPE_CUBE,
    GDF_PRIMITIVE_MESH_TYPE_PLANE,

    /// For internal use. Do not pass this into any API calls.
    gdfe_primitive_mesh_type_max
} GDF_PRIMITIVE_MESH_TYPE;

EXTERN_C_BEGIN

/// Get a mesh of a primitive type.
/// This does not allocate any additional memory, primitive meshes are initialized
/// on engine initialization.
/// @return A mesh handle.
/// @note Do not call \code GDF_MeshDestroy()\endcode on the returned handle.
GDF_Mesh GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE type);

/// @param vertices An array of vertices.
/// @param vertex_count The number of vertices.
/// @param indices An array of indices. (TODO! add NULL behavior)
/// @param index_count The number of indices.
/// @return A mesh handle. Must be destroyed with \code GDF_MeshDestroy()\endcode
/// @note This function copies the content of the arrays that are given as parameters.
/// Because of this, it is not recommended to frequently create new meshes.
GDF_Mesh GDF_MeshCreate(
    GDF_MeshVertex* vertices,
    u32 vertex_count,
    u32* indices,
    u32 index_count
);

/// Deallocates a mesh created by the caller.
/// @param mesh A mesh handle.
void GDF_MeshDestroy(GDF_Mesh mesh);

EXTERN_C_END