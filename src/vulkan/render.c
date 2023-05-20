/**
 * @brief Set the size of the viewport (and scissor, which is set to the same)
 * 
 * @param command_buffer Vulkan command buffer
 * @param width Width of the viewport
 * @param height Height of the viewport
 */
void vk_render_set_viewport(
    VkCommandBuffer command_buffer,
    uint32_t width,
    uint32_t height
){
    VkViewport viewport = {0};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)width;
    viewport.height = (float)height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = width;
    scissor.extent.height = height;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

/**
 * @brief Begin the render pass. Should be called before any other render
 *     operations
 * 
 * @param state Vulkan state
 * @param buffer0 Uniform buffer 0, to be uploaded to the GPU prior to rendering
 * @param buffer1 Uniform buffer 1, to be uploaded to the GPU prior to rendering
 * @return int32_t Return code. -1 if failed and rendering should be aborted.
 *     0 otherwise
 */
int32_t vk_render_begin(
    struct graphics_state *state,
    struct vk_uniform_buffer0 *buffer0,
    struct vk_uniform_buffer1 *buffer1
){
    vkWaitForFences(state->device, 1, &state->render_ready_fence, 1,
        UINT64_MAX);

    // TODO: this causes a validation layer error when resizing screen, but
    //    I think this might just be an issue with the drivers?
    VkResult res = vkAcquireNextImageKHR(state->device, state->swapchain,
        UINT64_MAX, state->image_ready_semaphore, VK_NULL_HANDLE,
        &state->current_image_index);

    // TODO: these values are not always guaranteed. should not fully rely on
    if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
        graphics_resize(state);
        return -1;
    }

    vkResetFences(state->device, 1, &state->render_ready_fence);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkResetCommandBuffer(state->command_buffer, 0);
    vkBeginCommandBuffer(state->command_buffer, &begin_info);

    vkCmdBindDescriptorSets(state->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipeline_layout, 0,
        1, &state->descriptor_set, 0, 0);

    vk_update_uniform_buffer(state->uniform_buffer0, buffer0);
    vk_update_uniform_buffer(state->uniform_buffer1, buffer1);
    return 0;
}

/**
 * @brief Main colour rendering. Should be called after rendering to the
 *     shadow depth buffer is complete
 * 
 * @param state Vulkan state
 * @param clear_color Clear colour for the screen, in order R, G, B, A
 * @param viewport_width Viewport width
 * @param viewport_height Viewport height
 */
void vk_render_main(
    struct graphics_state *state,
    float clear_color[4],
    uint32_t viewport_width,
    uint32_t viewport_height
){
    vkCmdEndRenderPass(state->command_buffer);

    vk_render_set_viewport(state->command_buffer, viewport_width,
        viewport_height);

    VkClearValue clear_value[2] = {0};
    clear_value[0].color.float32[0] = clear_color[0];
    clear_value[0].color.float32[1] = clear_color[1];
    clear_value[0].color.float32[2] = clear_color[2];
    clear_value[0].color.float32[3] = clear_color[3];

    clear_value[1].depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = state->render_pass;
    render_pass_info.framebuffer =
        state->framebuffers[state->current_image_index].framebuffer;
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent = state->extent;
    render_pass_info.clearValueCount = SIZEOF_ARRAY(clear_value);
    render_pass_info.pClearValues = clear_value;

    vkCmdBindPipeline(state->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        state->shader.pipeline);

    vk_transition_image(state->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT, state->shadow_image.image);

    vkCmdBeginRenderPass(state->command_buffer, &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);
}

/**
 * @brief Prepare to render shadows
 * 
 * @param state Vulkan state
 */
void vk_render_shadow(
    struct graphics_state *state
){
    vk_render_set_viewport(state->command_buffer, VK_SHADOW_WIDTH,
        VK_SHADOW_HEIGHT);

    VkClearValue clear_value = {0};
    clear_value.depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = state->shadow_render_pass;
    render_pass_info.framebuffer = state->shadow_framebuffer;
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent.width = VK_SHADOW_WIDTH;
    render_pass_info.renderArea.extent.height = VK_SHADOW_HEIGHT;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    vkCmdBindPipeline(state->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        state->shadow_pipeline);

    vkCmdBeginRenderPass(state->command_buffer, &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);
}


/**
 * @brief Completes render pass. Renders image to user
 * 
 * @param state Vulkan state
 */
void vk_render_end(
    struct graphics_state state
){
    vkCmdEndRenderPass(state.command_buffer);
    vkEndCommandBuffer(state.command_buffer);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &state.image_ready_semaphore;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &state.command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &state.render_ready_semaphore;

    vkQueueSubmit(state.queue.type.graphics, 1, &submit_info,
        state.render_ready_fence);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &state.render_ready_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &state.swapchain;
    present_info.pImageIndices = &state.current_image_index;

    vkQueuePresentKHR(state.queue.type.present, &present_info);
}

/**
 * @brief Draw a model
 * 
 * @param state Vulkan state
 * @param model Model to draw
 * @param texture Texture to apply
 * @param constant Push constant to apply to this draw call
 */
void sln_draw_model(
    struct graphics_state *state,
    struct vk_model model,
    struct vk_texture texture,
    struct vk_push_constant0 *constant
){
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(state->command_buffer, 0, 1,
        &model.vertex_buffer.buffer, offsets);

    vkCmdBindIndexBuffer(state->command_buffer,
        model.index_buffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(state->command_buffer, state->pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct vk_push_constant0),
        constant);

    vkCmdBindDescriptorSets(state->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipeline_layout, 1,
        1, &texture.set, 0, 0);

    vkCmdBindDescriptorSets(state->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipeline_layout, 2,
        1, &state->shadow_set, 0, 0);

    vkCmdDrawIndexed(state->command_buffer, model.index_buffer.index_count, 1,
        0, 0, 0);
}

void graphics_render_all_objects(
    struct graphics_state state,
    struct vk_object objects[VK_MAX_OBJECTS]
){
    for (uint32_t i = 0; i < VK_MAX_OBJECTS; i++) {
        if (!(objects[i].flags & VK_FLAG_EXISTS))
            continue;

        struct vk_model model = *objects[i].model;
        struct vk_texture texture = *objects[i].texture;
        state.push_constant_list.constants[i].index = i;
        sln_draw_model(&state, model, texture,
            &state.push_constant_list.constants[i]);
    }
}

/**
 * @brief Render all objects TODO:
 * 
 */
void graphics_render(
    struct graphics_state *state,
    struct sln_app app,
    struct transform view,
    struct graphics_world world
){
    // Constant buffer 0
    struct vk_uniform_buffer0 buf0 = {0};
    mat4_perspective(&buf0.projection,
        app.height / (float)app.width, DEG_TO_RAD(120),
        0.1f, 100.0f);
    mat4_transform(&buf0.view, view);

    mat4_orthographic(&buf0.camera_projection,
        -20, 20, -20, 20, 1, 30.0f);

    struct transform camera_view = {0};
    camera_view.position = (union vector3){0, -20, 0};
    camera_view.rotation = (union vector3){-DEG_TO_RAD(90), -DEG_TO_RAD(20), 0};
    camera_view.scale = (union vector3){1, 1, 1};

    mat4_transform(&buf0.camera_view, camera_view);

    // Constant buffer 1
    struct vk_uniform_buffer1 buf1 = {0};
    for (uint32_t i = 0; i < VK_MAX_OBJECTS; i++)
        if (world.objects[i].flags & VK_FLAG_EXISTS)
            mat4_transform(&buf1.model[i], world.objects[i].transform);

    // Render
    if (vk_render_begin(state, &buf0, &buf1))
        return;

    vk_render_shadow(state);
    graphics_render_all_objects(*state, world.objects);
    vk_render_main(state, (float[4]){1, 0, 1, 1}, state->extent.width,
        state->extent.height);
    graphics_render_all_objects(*state, world.objects);

    vk_render_end(*state);
}
