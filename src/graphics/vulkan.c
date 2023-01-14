/**
 * @brief Vulkan code
 * 
 */

#pragma warning(disable:4100)
/**
 * @brief Vulkan debug message handler
 * 
 * @param severity Severity of the message
 * @param type Type of message
 * @param callback_data Data of message
 * @param user_data User-defined data (when initialising Vulkan)
 * @return VKAPI_ATTR Return code
 */
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
  VkDebugUtilsMessageTypeFlagsEXT type,
  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
  void *user_data) {
#pragma warning(default:4100)

  if (severity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) {
    OutputDebugString(callback_data->pMessage);
    OutputDebugString("\n");
  }

  return VK_FALSE;
}

/**
 * @brief Creates and populates a VkDebugUtilsMessengerCreateInfoEXT structure
 * 
 * @return A populated VkDebugUtilsMessengerCreateInfoEXT structure
 */
VkDebugUtilsMessengerCreateInfoEXT vk_populate_debug_struct(void) {

  VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  create_info.messageType =
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

  create_info.pfnUserCallback = vk_debug_callback;

  return create_info;
}

/**
 * @brief Creates a Vulkan instance
 * 
 * @param app_version Version of Vulkan to use
 * @param instance Returns the created instance
 * @return VkResult Vulkan errors
 */
VkResult vk_create_instance(uint32_t app_version, VkInstance *instance) {

  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = app_version;

  // TODO only debug for validation layers
  char *layers[] = {
    "VK_LAYER_KHRONOS_validation"
  };

  char *extensions[] = {
    "VK_EXT_debug_utils",
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
  };

  VkDebugUtilsMessengerCreateInfoEXT messenger_create_info =
    vk_populate_debug_struct();

  VkInstanceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = &messenger_create_info;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
  create_info.ppEnabledLayerNames = layers;
  create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
  create_info.ppEnabledExtensionNames = extensions;

  return vkCreateInstance(&create_info, 0, instance);
}

/**
 * @brief Initialises the debug messaging function
 * 
 * @param instance A valid Vulkan instance
 * @param debug_messenger Returns the created debug messenger
 * @return VkResult Vulkan errors
 */
VkResult vk_create_debug_messenger(VkInstance instance,
  VkDebugUtilsMessengerEXT *debug_messenger) {

  VkDebugUtilsMessengerCreateInfoEXT create_info = vk_populate_debug_struct();

  // Extension function not loaded automatically
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
    (PFN_vkCreateDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(instance,
    "vkCreateDebugUtilsMessengerEXT");

  return vkCreateDebugUtilsMessengerEXT(instance, &create_info,
    0, debug_messenger);
}

/**
 * @brief Enumerates and selects a physical device
 * 
 * @param instance A valid Vulkan instance
 * @param selected_device Returns the selected physical device
 * @return VkResult Vulkan errors
 */
void vk_select_device(VkInstance instance,
  VkPhysicalDevice *selected_device) {

  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, 0);

  VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));

  vkEnumeratePhysicalDevices(instance, &device_count, devices);

  for (uint32_t i = 0; i < device_count; i++) {
    VkPhysicalDevice physical_device = devices[i];

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      continue;

#if 0
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);
#endif

    *selected_device = physical_device;
    goto complete;
  }

  // Fallback if nothing else works
  *selected_device = devices[0];

complete:
  free(devices);
}

struct vk_queue_family {
  uint32_t graphics;
  uint32_t present;
};

/**
 * @brief Get indicies for acceptable family queues for device
 * 
 * @param physical_device Physical device to enumerate
 * @param surface Vulkan surface for present support
 * @return Enumerated device family queue indices
 */
struct vk_queue_family vk_get_queue_family(
  VkPhysicalDevice physical_device, VkSurfaceKHR surface) {

  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, 0);

  VkQueueFamilyProperties *properties =
    malloc(count * sizeof(VkQueueFamilyProperties));
  
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, properties);

  struct vk_queue_family family = {0};

  for (uint32_t i = 0; i < count; i++) {
    if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      family.graphics = i;

    VkBool32 present_support = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i,
      surface, &present_support);

    if (present_support)
      family.present = i;
  }

  free(properties);

  return family;
}

/**
 * @brief Create a Vulkan device and queue
 * 
 * @param physical_device The physical device to use to create the device
 * @param queue_families The queue families to use
 * @param device Returns the created device
 * @return VkResult Vulkan errors
 */
VkResult vk_create_device_and_queue(VkPhysicalDevice physical_device,
  struct vk_queue_family queue_families, VkDevice *device) {

  float queue_priority = 1;

  VkDeviceQueueCreateInfo queue_info[2] = {0};
  queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info[0].queueFamilyIndex = queue_families.graphics;
  queue_info[0].queueCount = 1;
  queue_info[0].pQueuePriorities = &queue_priority;

  queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info[1].queueFamilyIndex = queue_families.present;
  queue_info[1].queueCount = 1;
  queue_info[1].pQueuePriorities = &queue_priority;

  char *extensions[] = {
    "VK_KHR_swapchain"
  };

  VkDeviceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = SIZEOF_ARRAY(queue_info);
  create_info.pQueueCreateInfos = queue_info;
  create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
  create_info.ppEnabledExtensionNames = extensions;
  create_info.pEnabledFeatures = 0;  // TODO: change?

  return vkCreateDevice(physical_device, &create_info, 0, device);
}

/**
 * @brief Link Vulkan to a HWND
 * 
 * @param instance Vulkan instance
 * @param hinstance Windows HINSTANCE
 * @param hwnd Windows HWND
 * @param surface Returns a Vulkan surface
 * @return VkResult Vulkan errors
 */
VkResult vk_win64(VkInstance instance, HINSTANCE hinstance,
  HWND hwnd, VkSurfaceKHR *surface) {

  VkWin32SurfaceCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hinstance = hinstance;
  create_info.hwnd = hwnd;

  // Extension function not loaded automatically
  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
    (PFN_vkCreateWin32SurfaceKHR)(void *)vkGetInstanceProcAddr(instance,
    "vkCreateWin32SurfaceKHR");

  return vkCreateWin32SurfaceKHR(instance, &create_info, 0, surface);
}

/**
 * @brief Create a swapchain
 * 
 * @param device Vulkan device
 * @param surface Vulkan surface
 * @param queue_family Queue families to use
 * @param format Swapchain colour format
 * @param swapchain Returns the created swapchain
 * @return VkResult Vulkan errors
 */
VkResult vk_create_swapchain(VkDevice device, VkSurfaceKHR surface,
  struct vk_queue_family queue_family, VkFormat format,
  VkSwapchainKHR *swapchain) {

  // TODO: I am lazy to enumerate available formats
  VkSurfaceFormatKHR surface_format = {
    .format = format,
    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
  };

  // TODO: Mess around with other values. pg 2364
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

  // TODO: Again, error checking
  VkExtent2D extent = {
    .width = 1424,
    .height = 720
  };

  VkSwapchainCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;
  create_info.minImageCount = 2;  // TODO: Check swapchain support for this?
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (queue_family.graphics == queue_family.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } else {
    uint32_t families[2] = {queue_family.graphics, queue_family.present};

    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = families;
  }

  // TODO: preTransform probably wrong
  create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = 1;

  return vkCreateSwapchainKHR(device, &create_info, 0, swapchain);
}

/**
 * @brief Returns images of the swapchain
 * 
 * @param device Vulkan device
 * @param swapchain Vulkan swapchain
 * @param images Return malloc() allocated memory for array of images
 * @param count Return number of images
 * @return VkResult Vulkan errors
 */
VkResult vk_get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain,
  VkImage **images, uint32_t *count) {

  vkGetSwapchainImagesKHR(device, swapchain, count, 0);

  *images = malloc(*count * sizeof(VkImage));

  return vkGetSwapchainImagesKHR(device, swapchain,
    count, *images);
}

/**
 * @brief Creates an image view for the passed image
 * 
 * @param device Vulkan device
 * @param image Image to use
 * @param format Swapchain colour format
 * @param view Returns image view
 * @return VkResult Vulkan errors
 */
VkResult vk_get_image_view(VkDevice device, VkImage image, VkFormat format,
  VkImageView *view) {

  VkImageViewCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  create_info.subresourceRange.levelCount = 1;
  create_info.subresourceRange.layerCount = 1;

  return vkCreateImageView(device, &create_info, 0, view);
}

/**
 * @brief Creates a shader module
 * 
 * @param device Vulkan device
 * @param code Shader bytecode
 * @param size Shader bytecode size in bytes
 * @param module Returns the created shader module
 * @return VkResult Vulkan errors
 */
VkResult vk_create_shader_module(VkDevice device, void *code, uint64_t size,
  VkShaderModule *module) {

  VkShaderModuleCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = size;
  create_info.pCode = code;

  return vkCreateShaderModule(device, &create_info, 0, module);
}

struct vk_state {
  VkDevice device;
  VkRenderPass render_pass;
  VkImageView views[2];
  VkFramebuffer framebuffers[2];
  struct vk_queue_family queue_family;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkQueue graphics_queue, present_queue;
  VkSwapchainKHR swapchain;
  VkPipeline pipeline;
};

/**
 * @brief Create a Vulkan pipeline layout
 * 
 * @param device Vulkan device
 * @param pipeline Returns the created pipeline layout
 * @return VkResult Vulkan errors
 */
VkResult vk_create_pipeline_layout(VkDevice device, VkPipelineLayout *pipeline) {

  VkPipelineLayoutCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  return vkCreatePipelineLayout(device, &create_info, 0, pipeline);
}

/**
 * @brief Create a Vulkan render pass
 * 
 * @param device Vulkan device
 * @param format Swapchain colour format
 * @param render_pass Returns the created render pass
 * @return VkResult Vulkan errors
 */
VkResult vk_create_render_pass(VkDevice device, VkFormat format,
  VkRenderPass *render_pass) {

  VkAttachmentDescription attachments[1] = {0};
  attachments[0].format = format;
  attachments[0].samples = 1;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference attachment_references[1] = {0};
  attachment_references[0].attachment = 0;
  attachment_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpasses[1] = {0};
  subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[0].colorAttachmentCount = SIZEOF_ARRAY(attachment_references);
  subpasses[0].pColorAttachments = attachment_references;

  VkRenderPassCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = SIZEOF_ARRAY(attachments);
  create_info.pAttachments = attachments;
  create_info.subpassCount = SIZEOF_ARRAY(subpasses);
  create_info.pSubpasses = subpasses;

  return vkCreateRenderPass(device, &create_info, 0, render_pass);
}

/**
 * @brief Creates a Vulkan graphics pipeline
 * 
 * @param device Vulkan device
 * @param vertex_stage Vertex shader
 * @param fragment_stage Fragment shader
 * @param pipeline Returns the created graphics pipeline
 * @return VkResult Vulkan errors
 */
VkResult vk_create_graphics_pipeline(VkDevice device,
  VkPipelineShaderStageCreateInfo vertex_stage,
  VkPipelineShaderStageCreateInfo fragment_stage,
  VkRenderPass render_pass, VkPipeline *pipeline) {

  // TODO: This function is too big. Split into smaller chunks
  VkPipelineLayout pipeline_layout;
  if (vk_create_pipeline_layout(device, &pipeline_layout) != VK_SUCCESS)
    DebugBreak();  // TODO: Error handling

  VkPipelineShaderStageCreateInfo stages[] =
    {vertex_stage, fragment_stage};

  VkPipelineVertexInputStateCreateInfo vertex_state = {0};
  vertex_state.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  
  vertex_state.vertexBindingDescriptionCount = 0;
  vertex_state.vertexAttributeDescriptionCount = 0;  // TODO: Temporary

  VkPipelineInputAssemblyStateCreateInfo input_assemble = {0};
  input_assemble.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assemble.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assemble.primitiveRestartEnable = 0;

  VkViewport viewport = {0};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = 1424;
  viewport.height = 720;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor = {0};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = 1424;
  scissor.extent.height = 720;

  VkPipelineViewportStateCreateInfo viewport_state = {0};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterisation = {0};
  rasterisation.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  rasterisation.depthClampEnable = 0;
  rasterisation.rasterizerDiscardEnable = 0;
  rasterisation.polygonMode = VK_POLYGON_MODE_FILL;  // TODO: Cool parameter
  rasterisation.lineWidth = 1;
  rasterisation.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterisation.frontFace = VK_FRONT_FACE_CLOCKWISE;  // TODO: Issue in future?
  rasterisation.depthBiasEnable = 0;

  VkPipelineMultisampleStateCreateInfo multisample = {0};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample.sampleShadingEnable = 0;

  VkPipelineColorBlendAttachmentState blend_attachments[1] = {0};
  blend_attachments[0].blendEnable = 0;
  blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colour_blend = {0};
  colour_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colour_blend.logicOpEnable = 0;
  colour_blend.attachmentCount = SIZEOF_ARRAY(blend_attachments);
  colour_blend.pAttachments = blend_attachments;

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = {0};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = SIZEOF_ARRAY(dynamic_states);
  dynamic_state.pDynamicStates = dynamic_states;

  VkGraphicsPipelineCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  create_info.stageCount = 2;
  create_info.pStages = stages;
  create_info.pVertexInputState = &vertex_state;
  create_info.pInputAssemblyState = &input_assemble;
  create_info.pViewportState = &viewport_state;
  create_info.pRasterizationState = &rasterisation;
  create_info.pMultisampleState = &multisample;
  create_info.pColorBlendState = &colour_blend;
  create_info.pDynamicState = &dynamic_state;
  create_info.layout = pipeline_layout;
  create_info.renderPass = render_pass;
  create_info.subpass = 0;

  return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
    1, &create_info, 0, pipeline);
}

/**
 * @brief Create a Vulkan framebuffer
 * 
 * @param device Vulkan device
 * @param render_pass Render pass
 * @param image_view Image view to use
 * @param framebuffer Returns the framebuffer
 * @return VkResult 
 */
VkResult vk_create_framebuffer(VkDevice device, VkRenderPass render_pass,
  VkImageView image_view, VkFramebuffer *framebuffer) {

  VkFramebufferCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &image_view;
  create_info.width = 1424;  // TODO: Hardcoded!
  create_info.height = 720;  // TODO: Hardcoded!
  create_info.layers = 1;

  return vkCreateFramebuffer(device, &create_info, 0, framebuffer);
}

/**
 * @brief Creates a Vulkan command pool
 * 
 * @param device Vulkan device
 * @param queue_family Queue family to use
 * @param command_pool Returns the created command pool
 * @return VkResult Vulkan errors
 */
VkResult vk_create_command_pool(VkDevice device,
  struct vk_queue_family queue_family, VkCommandPool *command_pool) {

  VkCommandPoolCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  create_info.queueFamilyIndex = queue_family.graphics;

  return vkCreateCommandPool(device, &create_info, 0, command_pool);
}

/**
 * @brief Creates a Vulkan command buffer
 * 
 * @param device Vulkan device
 * @param command_pool Command pool to use
 * @param command_buffer Returns the created command buffer
 * @return VkResult Vulkan errors
 */
VkResult vk_create_command_buffer(VkDevice device,
  VkCommandPool command_pool, VkCommandBuffer *command_buffer) {

  VkCommandBufferAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = command_pool;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = 1;

  return vkAllocateCommandBuffers(device, &allocate_info, command_buffer);
}

/**
 * @brief Call before rendering. Setup rendering frame
 * 
 * @param command_buffer Graphics command buffer
 * @param framebuffer Current framebuffer
 * @param render_pass Render pass to use
 * @param pipeline Pipeline to use
 */
void vk_begin_frame(VkCommandBuffer command_buffer, VkFramebuffer framebuffer,
  VkRenderPass render_pass, VkPipeline pipeline) {

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  
  vkBeginCommandBuffer(command_buffer, &begin_info);
  
  // TODO: Make parameter?
  VkClearValue clear_value = {0};
  clear_value.color.float32[0] = 1;
  clear_value.color.float32[1] = 0;
  clear_value.color.float32[2] = 1;
  clear_value.color.float32[3] = 1;

  VkRenderPassBeginInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = render_pass;
  render_pass_info.framebuffer = framebuffer;
  render_pass_info.renderArea.offset.x = 0;
  render_pass_info.renderArea.offset.y = 0;
  render_pass_info.renderArea.extent.width = 1424; // TODO: change
  render_pass_info.renderArea.extent.height = 720;  // TOOD: change
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_value;

  vkCmdBeginRenderPass(command_buffer, &render_pass_info,
    VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline);

  // TODO: These have already been defined in vk_create_graphics_pipeline
  VkViewport viewport = {0};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = 1424;
  viewport.height = 720;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor = {0};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = 1424;
  scissor.extent.height = 720;
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

/**
 * @brief Call after rendering is complete. Finishes and presents frame
 * 
 * @param command_buffer Graphics command buffer
 * @param swapchain Swapchain
 * @param graphics_queue Graphics queue family
 * @param present_queue 
 */
void vk_end_frame(VkCommandBuffer command_buffer, VkSwapchainKHR swapchain,
  VkQueue graphics_queue, VkQueue present_queue) {

  vkCmdEndRenderPass(command_buffer);
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {0};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 0;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;
  submit_info.signalSemaphoreCount = 0;

  vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);

  //Sleep(1000);  // TODO: this is fucked

  uint32_t image_index = 0;  // TODO: Wrong!!

  VkPresentInfoKHR present_info = {0};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 0;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &image_index;

  vkQueuePresentKHR(present_queue, &present_info);
}

/**
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 * @param hinstance Windows HINSTNACE
 * @param hwnd Windows HWND
 * @param format Swapchain colour format
 * @return struct vk_state A structure containing Vulkan details
 */
struct vk_state vk_init(struct sln_app app, VkFormat format) {

  struct vk_state state = {0};

  VkInstance instance;
  vk_create_instance(VK_API_VERSION_1_0, &instance);

  VkDebugUtilsMessengerEXT debug_messenger;
  vk_create_debug_messenger(instance, &debug_messenger);

  VkPhysicalDevice physical_device;
  vk_select_device(instance, &physical_device);

  VkSurfaceKHR surface;

#ifdef APP_WIN64
  vk_win64(instance, app.hinstance, app.hwnd, &surface);
#endif

  state.queue_family = vk_get_queue_family(physical_device,
    surface);

  vk_create_device_and_queue(physical_device, state.queue_family,
    &state.device);

  // TODO: Put in own function?
  vkGetDeviceQueue(state.device, state.queue_family.graphics, 0,
    &state.graphics_queue);

  vkGetDeviceQueue(state.device, state.queue_family.present, 0,
    &state.present_queue);

  vk_create_swapchain(state.device, surface, state.queue_family,
    format, &state.swapchain);

  vk_create_render_pass(state.device, format, &state.render_pass);

  vk_create_command_pool(state.device, state.queue_family, &state.command_pool);

  vk_create_command_buffer(state.device, state.command_pool,
    &state.command_buffer);

  VkImage *images;
  uint32_t image_count;
  vk_get_swapchain_images(state.device, state.swapchain, &images,
    &image_count);

  for (uint32_t i = 0; i < image_count; i++) {
    vk_get_image_view(state.device, images[i], format, &state.views[i]);

    vk_create_framebuffer(state.device, state.render_pass, state.views[i],
      &state.framebuffers[i]);
  }

  return state;
}
