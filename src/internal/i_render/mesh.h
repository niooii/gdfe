#pragma once

#include <gdfe/collections/list.h>
#include <gdfe/render/mesh.h>
#include <gdfe/render/vk/buffers.h>

typedef struct GDF_Mesh_T {
    GDF_MeshVertex* vertices;
    u32             vertex_count;
    u32*            indices;
    u32             index_count;

    GDF_VkBuffer vertex_buffer;
    GDF_VkBuffer index_buffer;
} GDF_Mesh_T;

void gdfe_init_primitive_meshes();
void gdfe_destroy_primitive_meshes();
