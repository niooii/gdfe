#pragma once

#include <gdfe/collections/list.h>
#include <gdfe/render/mesh.h>
#include <gdfe/render/vk/buffers.h>

typedef PACKED_STRUCT GDF_ObjInstanceData
{
    mat4 model;
}
GDF_ObjInstanceData END_PACKED_STRUCT;

typedef struct GDF_Mesh_T {
    GDF_MeshVertex* vertices;
    u32             vertex_count;
    u32*            indices;
    u32             index_count;

    /// All instance data pertaining to this mesh. Individual GDF_Objects own an
    /// instance_index that they can use to index into this list and retrieve their
    /// instance data.
    GDF_LIST(GDF_ObjInstanceData) instance_data;

    GDF_VkBuffer vertex_buffer;
    GDF_VkBuffer index_buffer;
} GDF_Mesh_T;

void gdfe_init_primitive_meshes();
void gdfe_destroy_primitive_meshes();
