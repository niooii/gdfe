#include <gdfe/render/mesh.h>
#include <i_render/mesh.h>
#include <i_render/renderer.h>

static GDF_Mesh PRIM_MESH_HANDLES[gdfe_primitive_mesh_type_max];

// helper to allocate and initialize meshes, does not allocate vertex data, index data, and
// the vulkan buffers.
FORCEINLINE static GDF_Mesh alloc_mesh()
{
    GDF_Mesh mesh = GDF_Malloc(sizeof(GDF_Mesh_T), GDF_MEMTAG_APPLICATION);
    mesh->instance_data = GDF_ListCreate(GDF_ObjInstanceData);

    GDF_ListPush(GDFE_RENDER_STATE.core_ctx.meshes, mesh);
    LOG_DEBUG("PUSH NEW LEN: %d", GDF_ListLen(GDFE_RENDER_STATE.core_ctx.meshes));

    return mesh;
}

FORCEINLINE static void dealloc_mesh(GDF_Mesh mesh)
{
    GDF_ListDestroy(mesh->instance_data);
    GDF_Free(mesh);
}

GDF_Mesh GDF_MeshCreateIcosphere(u8 subdivisions)
{
    const u32 four_to_n               = 1 << 2 * subdivisions;
    const u32 num_vertices            = 10 * four_to_n + 2;
    // TODO! dont use list just alloc
    GDF_LIST(GDF_MeshVertex) vertices = GDF_ListReserve(GDF_MeshVertex, num_vertices);
    const u32 num_indices             = 60 * four_to_n;
    GDF_LIST(GDF_MeshIndex) indices   = GDF_ListReserve(GDF_MeshIndex, num_indices);

    // setup initial faces
    const float one_norm = ginvsqrt(PHI * PHI + 1.0f) /
        1.66666666f; // div by 5/2 bc cube goes from -0.5 to 0.5 so same for this, im not really
                     // sure why this works but it inscribes the icosahedron in the cube.
    const float phi_norm = PHI * one_norm;
    vertices[0] = (GDF_MeshVertex){ .pos = { 0, one_norm, phi_norm, }, };
    vertices[1] = (GDF_MeshVertex){ .pos = { 0, one_norm, -phi_norm, }, };
    vertices[2] = (GDF_MeshVertex){ .pos = { 0, -one_norm, phi_norm, }, };
    vertices[3] = (GDF_MeshVertex){ .pos = { 0, -one_norm, -phi_norm, }, };

    vertices[4] = (GDF_MeshVertex){ .pos = { one_norm, phi_norm, 0, }, };
    vertices[5] = (GDF_MeshVertex){ .pos = { one_norm, -phi_norm, 0, }, };
    vertices[6] = (GDF_MeshVertex){ .pos = { -one_norm, phi_norm, 0, }, };
    vertices[7] = (GDF_MeshVertex){ .pos = { -one_norm, -phi_norm, 0, }, };

    vertices[8]  = (GDF_MeshVertex){ .pos = { phi_norm, 0, one_norm, }, };
    vertices[9]  = (GDF_MeshVertex){ .pos = { phi_norm, 0, -one_norm, }, };
    vertices[10] = (GDF_MeshVertex){ .pos = { -phi_norm, 0, one_norm, }, };
    vertices[11] = (GDF_MeshVertex){ .pos = { -phi_norm, 0, -one_norm, }, };

    GDF_ListSetLen(vertices, 12);

    const u32 initial_faces[][3] = {
        { 0, 2, 8 },
        { 0, 8, 4 },
        { 0, 4, 6 },
        { 0, 6, 10 },
        { 0, 10, 2 },

        { 1, 3, 11 },
        { 1, 11, 6 },
        { 1, 6, 4 },
        { 1, 4, 9 },
        { 1, 9, 3 },

        { 2, 10, 7 },
        { 2, 7, 5 },
        { 2, 5, 8 },
        { 8, 5, 9 },
        { 8, 9, 4 },

        { 3, 9, 5 },
        { 3, 5, 7 },
        { 3, 7, 11 },
        { 7, 10, 11 },
        { 11, 10, 6 },
    };

    GDF_Memcpy(indices, initial_faces, sizeof(initial_faces));

    GDF_ListSetLen(indices, 80);

    for (u32 i = 0; i < 60; i += 3)
    {
        const GDF_MeshIndex i0 = indices[i];
        const GDF_MeshIndex i1 = indices[i + 1];
        const GDF_MeshIndex i2 = indices[i + 2];

        const GDF_MeshVertex* v0 = vertices + i0;
        const GDF_MeshVertex* v1 = vertices + i1;
        const GDF_MeshVertex* v2 = vertices + i2;

        vec3 m0 = vec3_mul_scalar(vec3_add(v0->pos, v1->pos), 0.5f);
        vec3 m1 = vec3_mul_scalar(vec3_add(v1->pos, v2->pos), 0.5f);
        vec3 m2  = vec3_mul_scalar(vec3_add(v2->pos, v0->pos), 0.5f);
        vec3 mid = {
            .x = (m0.x + m1.x + m2.x) / 3.f,
            .y = (m0.y + m1.y + m2.y) / 3.f,
            .z = (m0.z + m1.z + m2.z) / 3.f,
        };
        LOG_DEBUG("%f, %f, %f", mid.x, mid.y, mid.z);
        // just push for now
        GDF_ListPush(vertices, mid);
        const u64 pushed_vertex_idx = GDF_ListLen(vertices) - 1;

        // GDF_ListPush(indices, i1);
        // GDF_ListPush(indices, pushed_vertex_idx);
        // GDF_ListPush(indices, i0);

        // GDF_ListPush(indices, i1);
        // GDF_ListPush(indices, pushed_vertex_idx);
        // GDF_ListPush(indices, i2);

        // GDF_ListPush(indices, i2);
        // GDF_ListPush(indices, pushed_vertex_idx);
        // GDF_ListPush(indices, i0);
    }

    // TODO! subdivisions
    GDF_Mesh mesh = alloc_mesh();
    mesh->index_count  = num_indices;
    mesh->vertex_count = num_vertices;
    // const u32 temp_vert_count = 12;
    // const u32 temp_idx_count  = 60;
    // mesh->index_count         = temp_idx_count;
    // mesh->vertex_count        = temp_vert_count;
    GDF_VkBufferCreateVertex(
        vertices, mesh->vertex_count, sizeof(GDF_MeshVertex), &mesh->vertex_buffer);
    GDF_VkBufferCreateIndex(indices, mesh->index_count, &mesh->index_buffer);

    return mesh;
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

        GDF_Mesh cube = alloc_mesh();

        cube->vertices     = vertices;
        cube->vertex_count = 8;
        cube->indices      = indices;
        cube->index_count  = 36;

        GDF_VkBufferCreateIndex(cube->indices, cube->index_count, &cube->index_buffer);
        GDF_VkBufferCreateVertex(
            cube->vertices, cube->vertex_count, sizeof(*cube->vertices), &cube->vertex_buffer);

        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE] = cube;
    }

    /* Create sphere mesh (ico sphere) */
    PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_SPHERE] = GDF_MeshCreateIcosphere(7);

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

        GDF_Mesh plane = alloc_mesh();

        plane->vertices     = vertices;
        plane->vertex_count = 4;
        plane->indices      = indices;
        plane->index_count  = 6;

        GDF_VkBufferCreateIndex(plane->indices, plane->index_count, &plane->index_buffer);
        GDF_VkBufferCreateVertex(
            plane->vertices, plane->vertex_count, sizeof(*plane->vertices), &plane->vertex_buffer);

        PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE] = plane;
    }
}

void gdfe_destroy_primitive_meshes()
{
    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE]->index_buffer);
    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_CUBE]->vertex_buffer);

    // TODO! spheres dont exist yet real
    // GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_SPHERE].index_buffer);
    // GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_SPHERE].vertex_buffer);

    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE]->index_buffer);
    GDF_VkBufferDestroy(&PRIM_MESH_HANDLES[GDF_PRIMITIVE_MESH_TYPE_PLANE]->vertex_buffer);
}

GDF_Mesh GDF_MeshGetPrimitive(GDF_PRIMITIVE_MESH_TYPE type) { return PRIM_MESH_HANDLES[type]; }
