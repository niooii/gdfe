// #include <render/vk/game_renderer.h>
// #include <geometry.h>
// #include <game/events.h>
// #include <render/renderer.h>
// #include <render/chunkmesh.h>
//
// bool vk_game_renderer_init(VkRenderContext* context, GDF_RendererState* backend)
// {
//     // if (
//     //     !GDF_EVENT_Register(GDF_EVENT_CHUNK_LOAD, NULL, __on_chunk_load) ||
//     //     !GDF_EVENT_Register(GDF_EVENT_CHUNK_UNLOAD, NULL, __on_chunk_unload) ||
//     //     !GDF_EVENT_Register(GDF_EVENT_CHUNK_UPDATE, NULL, __on_chunk_update)
//     // )
//     // {
//     //     return false;
//     // }
//
//     return true;
// }
//
// bool vk_game_renderer_draw(VkRenderContext* context, GDF_RendererState* renderer, u8 resource_idx, f32 dt)
// {
//     GDF_Game* game = renderer->game;
//     GDF_Camera* active_camera = game->main_camera;
//     VkCommandBuffer cmd_buffer = context->command_buffers[resource_idx];
//
//     VkDeviceSize offsets[] = {0};
//
//     // Render chunks
//
//     VkDescriptorSet sets[2] = {
//         context->global_vp_ubo_sets[resource_idx],
//         context->block_pipeline.descriptor_sets[resource_idx]
//     };
//
//     VkPipeline block_pipeline = VK_NULL_HANDLE;
//     VkPipelineLayout block_pipeline_layout = VK_NULL_HANDLE;
//     switch (renderer->render_mode)
//     {
//         case GDF_RENDER_MODE_FULL:
//         {
//             block_pipeline = context->block_pipeline.handle;
//             block_pipeline_layout = context->block_pipeline.layout;
//             break;
//         }
//         case GDF_RENDER_MODE_WIREFRAME:
//         {
//             block_pipeline = context->block_pipeline.wireframe_handle;
//             block_pipeline_layout = context->block_pipeline.wireframe_layout;
//             break;
//         }
//         default:
//         LOG_FATAL("wyd lil bro");
//         logging_flush_buffer();
//         exit(1);
//     }
//
//     vkCmdBindPipeline(
//         cmd_buffer,
//         VK_PIPELINE_BIND_POINT_GRAPHICS,
//         block_pipeline
//     );
//
//     vkCmdBindDescriptorSets(
//         cmd_buffer,
//         VK_PIPELINE_BIND_POINT_GRAPHICS,
//         block_pipeline_layout,
//         0,
//         sizeof(sets) / sizeof(VkDescriptorSet),
//         sets,
//         0,
//         NULL
//     );
//
//     struct PushConstantTemp {
//         ivec3 cc;
//     };
//     struct PushConstantTemp pct = {
//         .cc = (ivec3) {},
//     };
//
//     // LOG_WARN("RENDERING START");
//     // TODO! make an array that holds all modified chunks buffers to be updated
//     // or something.
//     // for now if the mesh buffer isnt up to date we update it right before
//     // drawing ew ew ew ew
//     for (
//         HashmapEntry* entry = GDF_HashmapIter(renderer->chunk_meshes);
//         entry != NULL;
//         GDF_HashmapIterAdvance(&entry)
//     )
//     {
//         ChunkMesh* mesh = *((ChunkMesh**)entry->val);
//         if (!mesh->buffers[resource_idx].up_to_date) {
//             if (!chunk_mesh_update_buffers(context, mesh, resource_idx)) {
//                 LOG_FATAL("gg go next");
//                 return false;
//             }
//         }
//
//         ivec3* cc = (ivec3*)entry->key;
//         vkCmdBindVertexBuffers(
//             cmd_buffer,
//             0,
//             1,
//             &mesh->buffers[resource_idx].vertex_buffer.handle,
//             offsets
//         );
//         vkCmdBindIndexBuffer(
//             cmd_buffer,
//             mesh->buffers[resource_idx].index_buffer.handle,
//             0,
//             VK_INDEX_TYPE_UINT16
//         );
//         // LOG_DEBUG("RENDERING CHUNK AT %d, %d, %d", cc->x, cc->y, cc->z);
//         pct.cc = *cc;
//         vkCmdPushConstants(
//             cmd_buffer,
//             block_pipeline_layout,
//             VK_SHADER_STAGE_VERTEX_BIT,
//             0,
//             sizeof(struct PushConstantTemp),
//             &pct
//         );
//         vkCmdDrawIndexed(cmd_buffer, mesh->num_indices, 1, 0, 0, 0);
//     }
//
//     // draw debug grid
//     vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->grid_pipeline.handle);
//
//     vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &context->up_facing_plane_vbo.handle, offsets);
//     vkCmdBindIndexBuffer(cmd_buffer, context->up_facing_plane_index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);
//
//     vkCmdBindDescriptorSets(
//         cmd_buffer,
//         VK_PIPELINE_BIND_POINT_GRAPHICS,
//         context->grid_pipeline.layout,
//         0,
//         1,
//         &context->global_vp_ubo_sets[resource_idx],
//         0,
//         NULL
//     );
//
//     vkCmdPushConstants(
//         cmd_buffer,
//         context->grid_pipeline.layout,
//         VK_SHADER_STAGE_VERTEX_BIT,
//         0,
//         sizeof(vec3),
//         &active_camera->pos
//     );
//
//     vkCmdDrawIndexed(cmd_buffer, 6, 1, 0, 0, 0);
//     return true;
// }