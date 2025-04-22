#pragma once

#include <gdfe/collections/list.h>
#include <gdfe/render/mesh.h>

typedef struct GDF_Mesh_T {
    GDF_MeshVertex* vertices;
    u32 vertex_count;
    u32* indices;
    u32 index_count;
} GDF_Mesh_T;

void gdfe_init_primitive_meshes();