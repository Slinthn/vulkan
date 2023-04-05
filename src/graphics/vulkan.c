#ifdef SLN_VULKAN

#include "vulkan.h"
#include "vulkan_win64.c"
#include "vulkan_render.c"
#include "vulkan_buffer.c"
#include "vulkan_shader.c"
#include "vulkan_debug.c"
#include "vulkan_image.c"

/**
 * @brief Creates a Vulkan 1.0 instance, with surface support and debug
 *   validation, if applicable
 * 
 * @param state Vulkan state
 */
void _vk_create_instance(struct vk_state *state) {

  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {0};

#ifdef SLN_DEBUG
  char *layers[] = {
    "VK_LAYER_KHRONOS_validation"
  };

  create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
  create_info.ppEnabledLayerNames = layers;
#endif  // SLN_DEBUG

  char *extensions[] = {
    "VK_KHR_surface",

#ifdef SLN_DEBUG
    "VK_EXT_debug_utils",
#endif  // SLN_DEBUG

#ifdef SLN_WIN64
    "VK_KHR_win32_surface"
#endif  // SLN_WIN64
  };

  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
  create_info.ppEnabledExtensionNames = extensions;

#ifdef SLN_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT messenger_create_info =
    _vk_populate_debug_struct();

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
void _vk_select_suitable_physical_device(struct vk_state *state) {

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
void _vk_select_suitable_queue_families(struct vk_state *state) {

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
void _vk_create_device_and_queue(struct vk_state *state) {

  float queue_priority = 1;

  VkDeviceQueueCreateInfo *queue_info =
    calloc(1, SIZEOF_ARRAY(state->queue_family.families)
      * sizeof(VkDeviceQueueCreateInfo));

  for (uint32_t i = 0; i < SIZEOF_ARRAY(state->queue_family.families); i++) {
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
void _vk_select_suitable_surface_format(struct vk_state *state) {

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

  // Fallback surface
  state->surface_format = surface_formats[0];

complete:
  free(surface_formats);
}

/**
 * @brief Create a swapchain
 * 
 * @param state Vulkan state
 */
void _vk_create_swapchain(struct vk_state *state) {

  state->extent.width = SLN_WINDOW_WIDTH;
  state->extent.height = SLN_WINDOW_HEIGHT;

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
  create_info.minImageCount = SLN_FRAMEBUFFER_COUNT;
  create_info.imageFormat = state->surface_format.format;
  create_info.imageColorSpace = state->surface_format.colorSpace;
  create_info.imageExtent = state->extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (state->queue_family.type.graphics == state->queue_family.type.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount =
      SIZEOF_ARRAY(state->queue_family.families);

    create_info.pQueueFamilyIndices = state->queue_family.families;
  }

  create_info.preTransform = surface_caps.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  create_info.clipped = 1;

  vkCreateSwapchainKHR(state->device, &create_info, 0, &state->swapchain);
}

/**
 * @brief Creates an image view for the passed image TODO:
 * 
 * @param device Vulkan device
 * @param image Image to use
 * @param format Swapchain colour format
 * @param view Returns image view
 * @return VkResult Vulkan errors
 */
void _vk_get_image_view(VkDevice device, VkImage image, VkFormat format,
  VkImageAspectFlags flags, VkImageView *view) {

  VkImageViewCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.subresourceRange.aspectMask = flags;
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
void _vk_create_framebuffer(VkDevice device, VkExtent2D extent,
  VkRenderPass render_pass, VkImageView colour_view, VkImageView depth_view,
  VkFramebuffer *framebuffer) {

    VkImageView view[] = {colour_view, depth_view};

  VkFramebufferCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = SIZEOF_ARRAY(view);
  create_info.pAttachments = view;
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
void _vk_get_swapchain_images(struct vk_state *state) {

  uint32_t image_count;
  vkGetSwapchainImagesKHR(state->device, state->swapchain, &image_count, 0);

  VkImage *images = malloc(image_count * sizeof(VkImage));

  vkGetSwapchainImagesKHR(state->device, state->swapchain,
    &image_count, images);

  struct vk_image depth_image = vk_create_depth_buffer(state);

  VkImageView depth_view;
  _vk_get_image_view(state->device, depth_image.image, VK_FORMAT_D32_SFLOAT,
      VK_IMAGE_ASPECT_DEPTH_BIT, &depth_view);

  for (uint32_t i = 0; i < image_count
    && i < SLN_FRAMEBUFFER_COUNT; i++) {
    _vk_get_image_view(state->device, images[i], state->surface_format.format,
      VK_IMAGE_ASPECT_COLOR_BIT, &state->framebuffers[i].view);

    _vk_create_framebuffer(state->device, state->extent,
      state->render_pass,
      state->framebuffers[i].view, depth_view,
      &state->framebuffers[i].framebuffer);
  }

  free(images);
}

/**
 * @brief Create a Vulkan render pass
 * 
 * @param state Vulkan state
 */
void _vk_create_render_pass(struct vk_state *state) {

  VkAttachmentDescription attachment[2] = {0};
  // Colour
  attachment[0].format = state->surface_format.format;
  attachment[0].samples = 1;
  attachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // Depth stencil
  attachment[1].format = VK_FORMAT_D32_SFLOAT;  // TODO: good?
  attachment[1].samples = 1;
  attachment[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment[1].finalLayout =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colour_reference = {0};
  colour_reference.attachment = 0;
  colour_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_reference = {0};
  depth_reference.attachment = 1;
  depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};  // TODO: check if this is good
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colour_reference;
  subpass.pDepthStencilAttachment = &depth_reference;

  VkSubpassDependency dependency = {0};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  create_info.attachmentCount = SIZEOF_ARRAY(attachment);
  create_info.pAttachments = attachment;
  create_info.subpassCount = 1;
  create_info.pSubpasses = &subpass;
  create_info.dependencyCount = 1;
  create_info.pDependencies = &dependency;

  vkCreateRenderPass(state->device, &create_info, 0, &state->render_pass);
}

/**
 * @brief Create a Vulkan command pool
 * 
 * @param state Vulkan state
 */
void _vk_create_command_pool(struct vk_state *state) {

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
void _vk_create_command_buffer(struct vk_state *state) {

  VkCommandBufferAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = state->command_pool;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = 1;

  vkAllocateCommandBuffers(state->device, &allocate_info,
    &state->command_buffer);
}

/**
 * @brief Create Vulkan semaphores
 * 
 * @param state Vulkan state
 */
void _vk_create_semaphores(struct vk_state *state) {

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
void _vk_create_fence(struct vk_state *state) {

  VkFenceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vkCreateFence(state->device, &create_info, 0, &state->render_ready_fence);
}

void _vk_initialise_surface(struct vk_state *state,
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
struct vk_state vk_init(struct vk_surface surface) {

  struct vk_state state = {0};

  _vk_create_instance(&state);

#ifdef SLN_DEBUG
  _vk_create_debug_messenger(&state);
#endif

  _vk_select_suitable_physical_device(&state);
  _vk_initialise_surface(&state, surface);
  _vk_select_suitable_surface_format(&state);
  _vk_select_suitable_queue_families(&state);
  _vk_create_device_and_queue(&state);
  _vk_create_swapchain(&state);
  _vk_create_render_pass(&state);
  _vk_create_command_pool(&state);
  _vk_create_command_buffer(&state);
  _vk_get_swapchain_images(&state);
  _vk_create_semaphores(&state);
  _vk_create_fence(&state);

  return state;
}

#endif  // SLN_VULKAN
