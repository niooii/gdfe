#include <gdfe/render/mesh.h>
#include <i_render/mesh.h>

static GDF_Mesh_T PRIM_MESH_HANDLES[gdfe_primitive_mesh_type_max];

void gdfe_init_primitive_meshes()
{
    /* Create cube mesh */
    {
        static const GDF_MeshVertex vertices[] = {
            {
                .pos    = { -0.5f, -0.5f, -0.5f },
                .normal = { 0, -1, 0 },
            },
            {
                .pos    = { 0.5f, -0.5f, -0.5f },
                .normal = { 0, -1, 0 },
            },
            {
                .pos    = { 0.5f, -0.5f, 0.5f },
                .normal = { 0, -1, 0 },
            },
            {
                .pos    = { -0.5f, -0.5f, 0.5f },
                .normal = { 0, -1, 0 },
            },

            // Top face vertices (y = 0.5)
            {
                .pos    = { -0.5f, 0.5f, -0.5f },
                .normal = { 0, 1, 0 },
            },
            {
                .pos    = { 0.5f, 0.5f, -0.5f },
                .normal = { 0, 1, 0 },
            },
            {
                .pos    = { 0.5f, 0.5f, 0.5f },
                .normal = { 0, 1, 0 },
            },
            {
                .pos    = { -0.5f, 0.5f, 0.5f },
                .normal = { 0, 1, 0 },
            },
        };

        static const u16 indices[] = { 0, 1, 2, 2, 3, 0, 4, 7, 6, 6, 5, 4, 3, 2, 6, 6, 7, 3, 0, 4,
            5, 5, 1, 0, 0, 3, 7, 7, 4, 0, 1, 5, 6, 6, 2, 1 };

        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE].vertices     = vertices;
        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE].vertex_count = 8;
        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE].indices      = indices;
        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE].index_count  = 36;
    }

    /* Create sphere mesh (ico sphere) */
    // TODO("Unimplemented")

    /* Create plane mesh */
    {
        static const GDF_MeshVertex vertices[] = {
            {
                .pos    = { -0.5f, 0.5f, -0.5f },
                .normal = { 0, 1, 0 },
            },
            {
                .pos    = { 0.5f, 0.5f, -0.5f },
                .normal = { 0, 1, 0 },
            },
            {
                .pos    = { 0.5f, 0.5f, 0.5f },
                .normal = { 0, 1, 0 },
            },
            {
                .pos    = { -0.5f, 0.5f, 0.5f },
                .normal = { 0, 1, 0 },
            },
        };

        static const u16 indices[] = { 0, 3, 2, 2, 1, 0 };

        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE].vertices     = vertices;
        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE].vertex_count = 4;
        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE].indices      = indices;
        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE].index_count  = 6;
    }
}

GDF_Mesh GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE type) { return &PRIM_MESH_HANDLES[type]; }
