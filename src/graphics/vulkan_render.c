/**
 * @brief TODO:
 * 
 * @param state 
 * @param width 
 * @param height 
 */
void vk_render_set_viewport(VkCommandBuffer command_buffer, uint32_t width,
        uint32_t height)
{
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
 * @brief Begin the render pass TODO:
 * 
 * @param state Vulkan state
 * @param clear_color Clear colour for the screen, in format RGBA
 */
void vk_render_begin(struct vk_state *state, float clear_color[4],
        struct vk_uniform_buffer0 *buffer0, uint32_t viewport_width,
        uint32_t viewport_height)
{
    vkWaitForFences(state->device, 1, &state->render_ready_fence, 1, UINT64_MAX);
    vkResetFences(state->device, 1, &state->render_ready_fence);

    vkAcquireNextImageKHR(state->device, state->swapchain, UINT64_MAX,
            state->image_ready_semaphore, VK_NULL_HANDLE,
            &state->current_image_index);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkResetCommandBuffer(state->command_buffer, 0);
    vkBeginCommandBuffer(state->command_buffer, &begin_info);

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

    vkCmdBeginRenderPass(state->command_buffer, &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(state->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        state->shader.pipeline);

    vkCmdBindDescriptorSets(state->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, state->shader.pipeline_layout, 0,
        1, &state->shader.descriptor_set, 0, 0);  // TODO: double-buffering

    vk_update_uniform_buffer(state->shader.uniform_buffer, buffer0);

    vk_render_set_viewport(state->command_buffer, viewport_width,
            viewport_height);
}

/**
 * @brief Call after rendering is complete. Finishes and presents frame
 * 
 * @param command_buffer Graphics command buffer
 * @param swapchain Swapchain
 * @param graphics_queue Graphics queue family
 * @param present_queue 
 */
void vk_render_end(struct vk_state state)
{
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

    vkQueueSubmit(state.queue.type.graphics, 1,
            &submit_info, state.render_ready_fence);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &state.render_ready_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &state.swapchain;
    present_info.pImageIndices = &state.current_image_index;

    vkQueuePresentKHR(state.queue.type.present, &present_info);
}
