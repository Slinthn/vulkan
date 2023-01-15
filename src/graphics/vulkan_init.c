/**
 * @brief Vulkan implementation code
 * 
 */

#ifdef SLN_VULKAN

#include "vulkan.h"
#include "vulkan_win64.c"

#ifdef SLN_DEBUG

#pragma warning(disable:4100)
/**
 * @brief Vulkan debug message handler. Called by Vulkan when a message is to
 *   be printed to the console
 * 
 * @param severity Severity of the message
 * @param type Type of message
 * @param callback_data Data of message
 * @param user_data User-defined data (in VkDebugUtilsMessengerCreateInfoEXT)
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
 * @brief Utility function to create and populate a
 *   VkDebugUtilsMessengerCreateInfoEXT structure
 * 
 * @return VkDebugUtilsMessengerCreateInfoEXT Populated structure
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
 * @brief Initialise the debug messaging function of Vulkan
 * 
 * @param state Vulkan state
 */
void vk_create_debug_messenger(struct vk_state *state) {

  VkDebugUtilsMessengerCreateInfoEXT create_info = vk_populate_debug_struct();

  // Extension function not loaded automatically
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
    (PFN_vkCreateDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(
    state->instance, "vkCreateDebugUtilsMessengerEXT");

  vkCreateDebugUtilsMessengerEXT(state->instance, &create_info, 0,
    &state->debug_messenger);
}

#endif  // SLN_DEBUG

/**
 * @brief Creates a Vulkan 1.0 instance, with surface support and debug
 *   validation, if applicable
 * 
 * @param state Vulkan state
 */
void vk_create_instance(struct vk_state *state) {

  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = VK_API_VERSION_1_0;

  char *layers[] = {
#ifdef SLN_DEBUG
    "VK_LAYER_KHRONOS_validation"
#endif  // SLN_DEBUG
  };

  char *extensions[] = {
    "VK_KHR_surface",

#ifdef SLN_DEBUG
    "VK_EXT_debug_utils",
#endif  // SLN_DEBUG

#ifdef SLN_WIN64
    "VK_KHR_win32_surface"
#endif  // SLN_WIN64
  };

  VkInstanceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
  create_info.ppEnabledLayerNames = layers;
  create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
  create_info.ppEnabledExtensionNames = extensions;

#ifdef SLN_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT messenger_create_info =
    vk_populate_debug_struct();

  create_info.pNext = &messenger_create_info;
#endif

  vkCreateInstance(&create_info, 0, &state->instance);
}

/**
 * @brief Enumerates all graphics cards and selects a suitable physical device
 *   for the purposes of the application
 * 
 * @param state Vulkan state
 */
void vk_select_suitable_physical_device(struct vk_state *state) {

  // TODO: VkPhysicalDeviceLimits?

  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(state->instance, &device_count, 0);

  VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));

  vkEnumeratePhysicalDevices(state->instance, &device_count, devices);

  // Checks for suitability
  for (uint32_t i = 0; i < device_count; i++) {
    VkPhysicalDevice physical_device = devices[i];

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      continue;

    state->physical_device = physical_device;
    goto search_complete;
  }

  // Fallback if nothing else works
  state->physical_device = devices[0];

search_complete:
  free(devices);
}

/**
 * @brief Get indices of suitable queue families for the physical device
 * 
 * @param state Vulkan state
 */
void vk_select_suitable_queue_families(struct vk_state *state) {

  uint32_t family_count;
  vkGetPhysicalDeviceQueueFamilyProperties(state->physical_device,
    &family_count, 0);

  VkQueueFamilyProperties *properties =
    malloc(family_count * sizeof(VkQueueFamilyProperties));
  
  vkGetPhysicalDeviceQueueFamilyProperties(state->physical_device,
    &family_count, properties);

  for (uint32_t i = 0; i < family_count; i++) {
    if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      state->queue_family.type.graphics = i;

    VkBool32 present_support = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(state->physical_device, i,
      state->surface, &present_support);

    if (present_support)
      state->queue_family.type.present = i;
  }

  free(properties);
}

/**
 * @brief Create a Vulkan device and also a queue for each queue family
 * 
 * @param state Vulkan state
 */
void vk_create_device_and_queue(struct vk_state *state) {

  float queue_priority = 1;

  VkDeviceQueueCreateInfo *queue_info =
    calloc(1, SIZEOF_ARRAY(
    state->queue_family.families) * sizeof(VkDeviceQueueCreateInfo));

  for (uint32_t i = 0; i < SIZEOF_ARRAY( state->queue_family.families); i++) {
    queue_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[i].queueFamilyIndex = state->queue_family.families[i];
    queue_info[i].queueCount = 1;
    queue_info[i].pQueuePriorities = &queue_priority;
  }

  char *extensions[] = {
    "VK_KHR_swapchain"
  };

  VkDeviceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = SIZEOF_ARRAY(state->queue_family.families);
  create_info.pQueueCreateInfos = queue_info;
  create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
  create_info.ppEnabledExtensionNames = extensions;
  create_info.pEnabledFeatures = 0;

  vkCreateDevice(state->physical_device, &create_info, 0, &state->device);

  free(queue_info);

  vkGetDeviceQueue(state->device, state->queue_family.type.graphics, 0,
    &state->graphics_queue);

  vkGetDeviceQueue(state->device, state->queue_family.type.present, 0,
    &state->present_queue);
}

/**
 * @brief Select a suitable surface format
 * 
 * @param state Vulkan state
 */
void vk_select_suitable_surface_format(struct vk_state *state) {

  uint32_t surface_format_count; 
  vkGetPhysicalDeviceSurfaceFormatsKHR(state->physical_device, state->surface,
    &surface_format_count, 0);

  VkSurfaceFormatKHR *surface_formats =
    malloc(surface_format_count * sizeof(VkSurfaceFormatKHR));

  vkGetPhysicalDeviceSurfaceFormatsKHR(state->physical_device, state->surface,
    &surface_format_count, surface_formats);

  for (uint32_t i = 0; i < surface_format_count; i++) {
    VkSurfaceFormatKHR surface_format = surface_formats[i];
    if (surface_format.format == VK_FORMAT_R8G8B8A8_SRGB
      && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      state->surface_format = surface_format;
      goto complete;
      }
  }

  state->surface_format = surface_formats[0];

complete:
  free(surface_formats);
}

/**
 * @brief Create a swapchain
 * 
 * @param state Vulkan state
 */
void vk_create_swapchain(struct vk_state *state) {

  VkSurfaceCapabilitiesKHR surface_caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->physical_device,
    state->surface, &surface_caps);

  state->extent.width = min(surface_caps.maxImageExtent.width,
    max(state->extent.width, surface_caps.minImageExtent.width));

  state->extent.height = min(surface_caps.maxImageExtent.height,
    max(state->extent.height, surface_caps.minImageExtent.height));

  VkSwapchainCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = state->surface;
  create_info.minImageCount = SLN_FRAMEBUFFER_COUNT;  // TODO: is this ok?
  create_info.imageFormat = state->surface_format.format;
  create_info.imageColorSpace = state->surface_format.colorSpace;
  create_info.imageExtent = state->extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (state->queue_family.type.graphics == state->queue_family.type.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = state->queue_family.families;
  }

  create_info.preTransform = surface_caps.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  create_info.clipped = 1;

  vkCreateSwapchainKHR(state->device, &create_info, 0, &state->swapchain);
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
void vk_get_image_view(VkDevice device, VkImage image, VkFormat format,
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

  vkCreateImageView(device, &create_info, 0, view);
}

/**
 * @brief Create a Vulkan framebuffer
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param render_pass Render pass
 * @param image_view Image view to use
 * @param framebuffer Returns the framebuffer
 * @return VkResult 
 */
void vk_create_framebuffer(VkDevice device, VkExtent2D extent,
  VkRenderPass render_pass, VkImageView image_view,
  VkFramebuffer *framebuffer) {

  VkFramebufferCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &image_view;
  create_info.width = extent.width;
  create_info.height = extent.height;
  create_info.layers = 1;

  vkCreateFramebuffer(device, &create_info, 0, framebuffer);
}

/**
 * @brief Creates views of framebuffers
 * 
 * @param state Vulkan state
 */
void vk_get_swapchain_images(struct vk_state *state) {

  uint32_t image_count;
  vkGetSwapchainImagesKHR(state->device, state->swapchain, &image_count, 0);

  VkImage *images = malloc(image_count * sizeof(VkImage));

  vkGetSwapchainImagesKHR(state->device, state->swapchain,
    &image_count, images);

  for (uint32_t i = 0; i < image_count
    && i < SLN_FRAMEBUFFER_COUNT; i++) {
    vk_get_image_view(state->device, images[i], state->surface_format.format,
      &state->framebuffers[i].view);

    vk_create_framebuffer(state->device, state->extent,
      state->render_pass,
      state->framebuffers[i].view,
      &state->framebuffers[i].framebuffer);
  }

  free(images);
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
void vk_create_shader_module(VkDevice device, void *code, uint64_t size,
  VkShaderModule *module) {

  VkShaderModuleCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = size;
  create_info.pCode = code;

  vkCreateShaderModule(device, &create_info, 0, module);
}

/**
 * @brief Create a Vulkan pipeline layout
 * 
 * @param device Vulkan device
 * @param pipeline Returns the created pipeline layout
 * @return VkResult Vulkan errors
 */
void vk_create_pipeline_layout(VkDevice device,
  VkPipelineLayout *pipeline) {

  VkPipelineLayoutCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  vkCreatePipelineLayout(device, &create_info, 0, pipeline);
}

/**
 * @brief Create a Vulkan render pass
 * 
 * @param state Vulkan state
 */
void vk_create_render_pass(struct vk_state *state) {

  VkAttachmentDescription attachment = {0};
  attachment.format = state->surface_format.format;
  attachment.samples = 1;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference attachment_reference = {0};
  attachment_reference.attachment = 0;
  attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachment_reference;

  VkRenderPassCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &attachment;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;

  vkCreateRenderPass(state->device, &create_info, 0, &state->render_pass);
}

/**
 * @brief Creates a Vulkan graphics pipeline
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param vertex_stage Vertex shader
 * @param fragment_stage Fragment shader
 * @param pipeline Returns the created graphics pipeline
 * @return VkResult Vulkan errors
 */
void vk_create_graphics_pipeline(VkDevice device,
  VkPipelineShaderStageCreateInfo vertex_stage,
  VkPipelineShaderStageCreateInfo fragment_stage,
  VkRenderPass render_pass, VkPipeline *pipeline) {

  VkPipelineLayout pipeline_layout;
  vk_create_pipeline_layout(device, &pipeline_layout);

  VkPipelineShaderStageCreateInfo stages[] =
    {vertex_stage, fragment_stage};

  VkVertexInputBindingDescription bind_desc = {0};
  bind_desc.binding = 0;
  bind_desc.stride = sizeof(struct vk_vertex);
  bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attributes[2] = {0};
  attributes[0].location = 0;
  attributes[0].binding = 0;
  attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
  attributes[0].offset = 0;
  attributes[1].location = 1;
  attributes[1].binding = 0;
  attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributes[1].offset = sizeof(float) * 2;

  VkPipelineVertexInputStateCreateInfo vertex_state = {0};
  vertex_state.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  
  vertex_state.vertexBindingDescriptionCount = 1;
  vertex_state.pVertexBindingDescriptions = &bind_desc;
  vertex_state.vertexAttributeDescriptionCount = 2;
  vertex_state.pVertexAttributeDescriptions = attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assemble = {0};
  input_assemble.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

  input_assemble.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assemble.primitiveRestartEnable = 0;

  VkPipelineViewportStateCreateInfo viewport_state = {0};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterisation = {0};
  rasterisation.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  rasterisation.depthClampEnable = 0;
  rasterisation.rasterizerDiscardEnable = 0;
  rasterisation.polygonMode = VK_POLYGON_MODE_FILL;
  rasterisation.lineWidth = 1;
  rasterisation.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterisation.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

  vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
    1, &create_info, 0, pipeline);
}

/**
 * @brief Create a Vulkan command pool
 * 
 * @param state Vulkan state
 */
void vk_create_command_pool(struct vk_state *state) {

  VkCommandPoolCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  create_info.queueFamilyIndex = state->queue_family.type.graphics;

  vkCreateCommandPool(state->device, &create_info, 0, &state->command_pool);
}

/**
 * @brief Create a Vulkan command buffer
 * 
 * @param state Vulkan state
 */
void vk_create_command_buffer(struct vk_state *state) {

  VkCommandBufferAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = state->command_pool;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = 1;

  vkAllocateCommandBuffers(state->device, &allocate_info,
    &state->command_buffer);
}

/**
 * @brief Create a Vulkan semaphore
 * 
 * @param state Vulkan state
 */
void vk_create_semaphore(struct vk_state *state) {

  VkSemaphoreCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  vkCreateSemaphore(state->device, &create_info, 0,
    &state->image_ready_semaphore);

  vkCreateSemaphore(state->device, &create_info, 0,
    &state->render_ready_semaphore);
}

/**
 * @brief Create a Vulkan fence
 * 
 * @param state Vulkan state
 */
void vk_create_fence(struct vk_state *state) {

  VkFenceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vkCreateFence(state->device, &create_info, 0, &state->render_ready_fence);
}

void vk_initialise_surface(struct vk_state *state,
  struct vk_surface appsurface) {

#ifdef SLN_WIN64
  vk_win64(state, appsurface);
#else
  #error "No Vulkan surface has been selected."
#endif
}

/**
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 * @param hinstance Windows HINSTNACE
 * @param hwnd Windows HWND
 * @param extent Preferred dimensions of framebuffer
 * @param format Swapchain colour format
 * @return struct vk_state A structure containing Vulkan details
 */
struct vk_state vk_init(struct vk_initialise_info init_info) {

  struct vk_state state = {0};
  state.extent = init_info.extent;

  vk_create_instance(&state);

#ifdef SLN_DEBUG
  vk_create_debug_messenger(&state);
#endif

  vk_select_suitable_physical_device(&state);
  vk_initialise_surface(&state, init_info.surface);
  vk_select_suitable_surface_format(&state);
  vk_select_suitable_queue_families(&state);
  vk_create_device_and_queue(&state);
  vk_create_swapchain(&state);
  vk_create_render_pass(&state);
  vk_create_command_pool(&state);
  vk_create_command_buffer(&state);
  vk_get_swapchain_images(&state);
  vk_create_semaphore(&state);
  vk_create_semaphore(&state);
  vk_create_fence(&state);

  return state;
}

struct vk_shader vk_create_shader(struct vk_shader_info shader_info) {

  struct vk_shader shader = {0};

  VkPipelineShaderStageCreateInfo vertex_stage = {0};
  vertex_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage.pName = "main";

  vk_create_shader_module(shader_info.device, shader_info.vertex_data,
    shader_info.vertex_data_size, &vertex_stage.module);

  VkPipelineShaderStageCreateInfo fragment_stage = {0};
  fragment_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage.pName = "main";

  vk_create_shader_module(shader_info.device, shader_info.fragment_data,
    shader_info.fragment_data_size, &fragment_stage.module);
  
  vk_create_graphics_pipeline(shader_info.device, vertex_stage, fragment_stage,
    shader_info.render_pass, &shader.pipeline);

  return shader;
}

void vk_render_begin(struct vk_state *state, float clear_color[4]) {

  vkWaitForFences(state->device, 1, &state->render_ready_fence, 1, UINT64_MAX);
  vkResetFences(state->device, 1, &state->render_ready_fence);

  vkAcquireNextImageKHR(state->device, state->swapchain, UINT64_MAX,
    state->image_ready_semaphore, VK_NULL_HANDLE, &state->current_image_index);

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  
  vkResetCommandBuffer(state->command_buffer, 0);

  vkBeginCommandBuffer(state->command_buffer, &begin_info);

  VkClearValue clear_value = {0};
  clear_value.color.float32[0] = clear_color[0];
  clear_value.color.float32[1] = clear_color[1];
  clear_value.color.float32[2] = clear_color[2];
  clear_value.color.float32[3] = clear_color[3];

  VkRenderPassBeginInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = state->render_pass;
  render_pass_info.framebuffer =
    state->framebuffers[state->current_image_index].framebuffer;

  render_pass_info.renderArea.offset.x = 0;
  render_pass_info.renderArea.offset.y = 0;
  render_pass_info.renderArea.extent = state->extent;
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_value;

  vkCmdBeginRenderPass(state->command_buffer, &render_pass_info,
    VK_SUBPASS_CONTENTS_INLINE);
}

/**
 * @brief Call after rendering is complete. Finishes and presents frame
 * 
 * @param command_buffer Graphics command buffer
 * @param swapchain Swapchain
 * @param graphics_queue Graphics queue family
 * @param present_queue 
 */
void vk_render_end(struct vk_state state) {

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

  vkQueueSubmit(state.graphics_queue, 1,
    &submit_info, state.render_ready_fence);

  VkPresentInfoKHR present_info = {0};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &state.render_ready_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &state.swapchain;
  present_info.pImageIndices = &state.current_image_index;

  vkQueuePresentKHR(state.present_queue, &present_info);
}

void vk_render_bind_shader(struct vk_state state,
  struct vk_shader shader) {

  vkCmdBindPipeline(state.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    shader.pipeline);
}

void vk_render_set_viewport(struct vk_state state,
  uint32_t width, uint32_t height) {

  VkViewport viewport = {0};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)width;
  viewport.height = (float)height;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;
  vkCmdSetViewport(state.command_buffer, 0, 1, &viewport);

  VkRect2D scissor = {0};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = width;
  scissor.extent.height = height;
  vkCmdSetScissor(state.command_buffer, 0, 1, &scissor);
}

uint32_t vk_find_suitable_memory_type(VkMemoryRequirements requirements,
  VkPhysicalDeviceMemoryProperties properties, uint32_t required_flags) {

  for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
    VkMemoryPropertyFlags flags = properties.memoryTypes[i].propertyFlags;
    if ((requirements.memoryTypeBits & (1 << i))
      && (flags & required_flags) == required_flags)
      return i;
  }

  return UINT32_MAX;
}

struct vk_buffer vk_create_buffer(struct vk_buffer_info buffer_info) {
  
  struct vk_buffer buffer = {0};

  VkBufferCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.size = buffer_info.size;
  create_info.usage = buffer_info.usage;
  create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
  create_info.queueFamilyIndexCount =
    SIZEOF_ARRAY(buffer_info.queue_family.families);

  create_info.pQueueFamilyIndices = buffer_info.queue_family.families;

  vkCreateBuffer(buffer_info.device, &create_info, 0, &buffer.buffer);

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(buffer_info.device, buffer.buffer,
    &requirements);

  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(buffer_info.physical_device, &properties);

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex =
    vk_find_suitable_memory_type(requirements, properties, buffer_info.flags);

  vkAllocateMemory(buffer_info.device, &allocate_info, 0, &buffer.memory);

  vkBindBufferMemory(buffer_info.device, buffer.buffer, buffer.memory, 0);

  return buffer;
}

#endif  // SLN_VULKAN
