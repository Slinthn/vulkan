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
 * @param clear_color Clear colour for the screen, in order R, G, B, A
 * @param buffer0 Uniform buffer 0, to be uploaded to the GPU prior to rendering
 * @param viewport_width Viewport width
 * @param viewport_height Viewport height
 */
void vk_render_begin(
    struct vk_state *state,
    struct vk_uniform_buffer0 *buffer0,
    struct vk_uniform_buffer1 *buffer1
){
    vkWaitForFences(state->device, 1, &state->render_ready_fence, 1,
        UINT64_MAX);
    vkResetFences(state->device, 1, &state->render_ready_fence);

    vkAcquireNextImageKHR(state->device, state->swapchain, UINT64_MAX,
        state->image_ready_semaphore, VK_NULL_HANDLE,
        &state->current_image_index);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkResetCommandBuffer(state->command_buffer, 0);
    vkBeginCommandBuffer(state->command_buffer, &begin_info);

    vkCmdBindDescriptorSets(state->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipeline_layout, 0,
        1, &state->descriptor_set, 0, 0);

    vk_update_uniform_buffer(state->uniform_buffer0, buffer0);
    vk_update_uniform_buffer(state->uniform_buffer1, buffer1);
}

void vk_render_main(
    struct vk_state *state,
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

    _vk_transition_image(state->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT, state->depth_image.image);

    vkCmdBeginRenderPass(state->command_buffer, &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);
}

void vk_render_shadow(
    struct vk_state *state
){
    vk_render_set_viewport(state->command_buffer, 100, 100);  // TODO: random numbers

    VkClearValue clear_value = {0};
    clear_value.depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = state->shadow_render_pass;
    render_pass_info.framebuffer = state->shadow_framebuffer;
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent.width = 100;  // TODO: random numbers
    render_pass_info.renderArea.extent.height = 100;
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
    struct vk_state state
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
 * @brief Draw a model TODO:
 * 
 * @param model Model to draw
 * @param constant Push constant to apply to this draw call
 */
void sln_draw_model(
    struct vk_state *state,
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