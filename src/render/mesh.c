#include <gdfe/render/mesh.h>
#include <i_render/mesh.h>

static GDF_Mesh_T PRIM_MESH_HANDLES[gdfe_primitive_mesh_type_max];

FORCEINLINE static void init_mesh(GDF_Mesh mesh)
{

}

FORCEINLINE static void deinit_mesh(GDF_Mesh mesh)
{

}

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

        static const u32 indices[] = { 0, 1, 2, 2, 3, 0, 4, 7, 6, 6, 5, 4, 3, 2, 6, 6, 7, 3, 0, 4,
            5, 5, 1, 0, 0, 3, 7, 7, 4, 0, 1, 5, 6, 6, 2, 1 };

        GDF_Mesh cube = &PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE];

        cube->vertices     = vertices;
        cube->vertex_count = 8;
        cube->indices      = indices;
        cube->index_count  = 36;

        GDF_VkBufferCreateIndex(cube->indices, cube->index_count, &cube->index_buffer);
        GDF_VkBufferCreateVertex(
            cube->vertices, cube->vertex_count, sizeof(*cube->vertices), &cube->vertex_buffer);
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

        static const u32 indices[] = { 0, 3, 2, 2, 1, 0 };

        GDF_Mesh plane = &PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE];

        plane->vertices     = vertices;
        plane->vertex_count = 4;
        plane->indices      = indices;
        plane->index_count  = 6;

        GDF_VkBufferCreateIndex(plane->indices, plane->index_count, &plane->index_buffer);
        GDF_VkBufferCreateVertex(
            plane->vertices, plane->vertex_count, sizeof(*plane->vertices), &plane->vertex_buffer);
    }
}

void gdfe_destroy_primitive_meshes()
{
    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE].index_buffer);
    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE].vertex_buffer);

    // TODO! spheres dont exist yet real
    // GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_SPHERE].index_buffer);
    // GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_SPHERE].vertex_buffer);

    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE].index_buffer);
    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE].vertex_buffer);
}

GDF_Mesh GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE type) { return &PRIM_MESH_HANDLES[type]; }
