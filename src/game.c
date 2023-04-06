#define SLN_WINDOW_WIDTH 1920
#define SLN_WINDOW_HEIGHT 1080
#define SLN_FRAMEBUFFER_COUNT 2

#ifdef SLN_VULKAN
#pragma warning(push, 0)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma warning(pop)
#endif  // SLN_VULKAN

#include "file.c"
#include "graphics/vulkan.h"

struct sln_model {
    struct vk_buffer vertex_buffer;
    struct vk_index_buffer index_buffer;
};

struct sln_resources {
    struct sln_model model, model2;
};

#pragma pack(push, 1)
struct sln_vertex {
    float position[3];
    float texture[2];
    float normal[3];
};
#pragma pack(pop)

#include "model/sm.c"

// TODO: daz no good...
static struct vk_state vulkan;
static struct sln_resources resources;
static struct vk_push_contant0_list push_constant_list;

/**
 * @brief Load the models for the game
 * 
 */
void sln_load_models(void)
{
    resources.model = sln_load_sm(vulkan.device, vulkan.physical_device,
            "tower.sm");
    resources.model2 = sln_load_sm(vulkan.device, vulkan.physical_device,
            "cube.sm");
}

/**
 * @brief Initialise the game
 * 
 * @param surface OS Vulkan surface
 */
void sln_init(struct vk_surface surface)
{
    vulkan = vk_init(surface);
    sln_load_models();
}

/**
 * @brief TODO:
 * 
 */
void sln_draw_model(struct sln_model model, struct vk_push_constant0 *constant)
{
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(vulkan.command_buffer, 0, 1,
        &model.vertex_buffer.buffer, offsets);

    vkCmdBindIndexBuffer(vulkan.command_buffer,
            model.index_buffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(vulkan.command_buffer, vulkan.shader.pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct vk_push_constant0),
            constant);

    vkCmdDrawIndexed(vulkan.command_buffer, model.index_buffer.index_count, 1,
            0, 0, 0);
}

void sln_render(void)
{
    push_constant_list.constants[0].index = 0;
    push_constant_list.constants[0].index = 1;

    sln_draw_model(resources.model, &push_constant_list.constants[0]);
    sln_draw_model(resources.model2, &push_constant_list.constants[1]);
}

/**
 * @brief Call once per frame to update the game and render
 * 
 * @param app Game information
 */
void sln_update(struct sln_app app)
{
    static struct transform model = {0};
    model.position.c.x += app.controls.move.c.x / 10.0f;
    model.position.c.z += -app.controls.move.c.y / 10.0f;

    if (app.controls.actions & ACTION_JUMP) {
        model.position.c.y = sinf(model.rotation.c.y * 3) / 3.0f;
        model.rotation.c.y += 0.1f;
        model.rotation.c.z += 0.05f;
        model.rotation.c.x += 0.02f;
    }
    model.scale = (union vector3){1, 1, 1};

    struct transform view = {0};
    view.position = (union vector3){0, 0, -4};
    view.scale = (union vector3){1, 1, 1};

    struct vk_uniform_buffer0 buf0 = {0};
    mat4_perspective(&buf0.projection,
            SLN_WINDOW_HEIGHT / (float)SLN_WINDOW_WIDTH, DEG_TO_RAD(90),
            0.1f, 1000.0f);

    mat4_transform(&buf0.view, view);

    struct vk_uniform_buffer1 buf1 = {0};
    mat4_transform(&buf1.model[0], model);

    static struct transform model2 = {0};
    model2.scale = (union vector3){1, 1, 1};
    model2.position = (union vector3){0, 0, 5};
    model2.rotation = (union vector3){0, DEG_TO_RAD(90), 0};

    mat4_transform(&buf1.model[1], model2);

    vk_render_begin(&vulkan, (float[4]){1, 1, 1, 1}, &buf0, &buf1,
            app.width, app.height);
    sln_render();
    vk_render_end(vulkan);
}
