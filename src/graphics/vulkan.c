/**
 * @brief Vulkan code
 * 
 */

/**
 * @brief Vulkan debug messages. Should not be called directly
 * 
 */
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
  VkDebugUtilsMessageTypeFlagsEXT type,
  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
  void *user_data) {

  // TODO: Suppress warnings
  (void)severity;
  (void)type;
  (void)user_data;

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
    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
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

  VkPhysicalDevice *devices =
    malloc(device_count * sizeof(VkPhysicalDevice));
    // TODO: different memory allocation method?

  vkEnumeratePhysicalDevices(instance, &device_count, devices);

  // TODO: Better checks
  VkPhysicalDevice selected = devices[0];

  *selected_device = selected;

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
    (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance,
    "vkCreateWin32SurfaceKHR");

  return vkCreateWin32SurfaceKHR(instance, &create_info, 0, surface);
}

/**
 * @brief Create a swapchain
 * 
 * @param device Vulkan device
 * @param surface Vulkan surface
 * @param queue_family Queue families to use
 * @param swapchain Returns the created swapchain
 * @return VkResult Vulkan errors
 */
VkResult vk_create_swapchain(VkDevice device, VkSurfaceKHR surface,
  struct vk_queue_family queue_family, VkSwapchainKHR *swapchain) {

  // TODO: I am lazy to enumerate available formats
  VkSurfaceFormatKHR surface_format = {
    .format = VK_FORMAT_R8G8B8A8_SRGB,
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
  create_info.minImageCount = 2;  // TODO: Incorrect
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
 * @param view Returns image view
 * @return VkResult Vulkan errors
 */
VkResult vk_get_image_view(VkDevice device, VkImage image, VkImageView *view) {

  VkImageViewCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = VK_FORMAT_R8G8B8A8_SRGB;  // TODO: This is hardcoded!
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
 * @param render_pass Returns the created render pass
 * @return VkResult Vulkan errors
 */
VkResult vk_create_render_pass(VkDevice device, VkRenderPass *render_pass) {

  VkAttachmentDescription attachments[1] = {0};
  attachments[0].format = VK_FORMAT_R8G8B8A8_SRGB;
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
  VkPipelineShaderStageCreateInfo fragment_stage, VkPipeline *pipeline) {

  // TODO: This function is too big. Split into smaller chunks
  VkPipelineLayout pipeline_layout;
  if (vk_create_pipeline_layout(device, &pipeline_layout) != VK_SUCCESS)
    DebugBreak();  // TODO: Error handling

  VkRenderPass render_pass;
  if (vk_create_render_pass(device, &render_pass) != VK_SUCCESS)
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
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 * @param hinstance Windows HINSTNACE
 * @param hwnd Windows HWND
 * @return struct vk_state A structure containing Vulkan details
 */
struct vk_state vk_init(struct sln_app app) {

  struct vk_state state = {0};

  VkInstance instance;
  if (vk_create_instance(VK_API_VERSION_1_0, &instance) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling

  VkDebugUtilsMessengerEXT debug_messenger;
  if (vk_create_debug_messenger(instance, &debug_messenger) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling

  VkPhysicalDevice physical_device;
  vk_select_device(instance, &physical_device);

  VkSurfaceKHR surface;

#ifdef APP_WIN64
  if (vk_win64(instance, app.hinstance, app.hwnd, &surface) != VK_SUCCESS)
    DebugBreak();
#endif

  struct vk_queue_family queue_family = vk_get_queue_family(physical_device,
    surface);

  if (vk_create_device_and_queue(physical_device, queue_family,
    &state.device) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling
  
  // TODO: Put in own function
  VkQueue graphics_queue, present_queue;
  vkGetDeviceQueue(state.device, queue_family.graphics, 0, &graphics_queue);
  vkGetDeviceQueue(state.device, queue_family.present, 0, &present_queue);

  VkSwapchainKHR swapchain;
  if (vk_create_swapchain(state.device, surface, queue_family,
    &swapchain) != VK_SUCCESS)
    DebugBreak();  // TODO: Error handling

  VkImage *images;
  uint32_t image_count;
  if (vk_get_swapchain_images(state.device, swapchain, &images,
    &image_count) != VK_SUCCESS)
    DebugBreak();  // TODO: Error handling

  // TODO: Totally scuffed, idk how many images there really are!!
  VkImageView views[2] = {0};
  vk_get_image_view(state.device, images[0], &views[0]);
  vk_get_image_view(state.device, images[1], &views[1]);

  return state;
}
