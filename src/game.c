#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720
#define SLN_FRAMEBUFFER_COUNT 2

#ifdef SLN_VULKAN
#pragma warning(push, 0)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma warning(pop)
#endif  // SLN_VULKAN

#include "file.c"
#include "graphics/vulkan.h"
#include "world/world.h"

struct sln_resources {
    struct sln_world world;
};

struct sln_state {
    struct transform view;
};

// TODO: daz no good...
struct vk_state vulkan;
struct sln_state game;
struct sln_resources resources;
struct vk_push_contant0_list push_constant_list;

/**
 * @brief Initialise the game
 * 
 * @param surface OS Vulkan surface
 */
void sln_init(struct vk_surface surface)
{
    vulkan = vk_init(surface);

    resources.world = sln_load_sw(vulkan.device, vulkan.physical_device, "world.sw");
}

/**
 * @brief Render all objects TODO:
 * 
 */
void sln_render(struct sln_app app)
{
    struct vk_model *models = resources.world.models;
    struct sln_object *objects = resources.world.objects;

    // Constant buffer 0
    struct vk_uniform_buffer0 buf0 = {0};
    mat4_perspective(&buf0.projection,
            SLN_WINDOW_HEIGHT / (float)SLN_WINDOW_WIDTH, DEG_TO_RAD(90),
            0.1f, 1000.0f);
    mat4_transform(&buf0.view, game.view);

    // Constant buffer 1
    struct vk_uniform_buffer1 buf1 = {0};

    for (uint32_t i = 0; i < SIZEOF_ARRAY(resources.world.objects); i++)
        if (objects[i].flags & SLN_WORLD_FLAG_EXISTS)
            mat4_transform(&buf1.model[i], objects[i].transform);

    // Render
    vk_render_begin(&vulkan, (float[4]){1, 1, 1, 1}, &buf0, &buf1,
            app.width, app.height);

    for (uint32_t i = 0; i < SIZEOF_ARRAY(resources.world.objects); i++) {
        if (!(objects[i].flags & SLN_WORLD_FLAG_EXISTS))
            continue;

        struct vk_model model = models[objects[i].model_index];
        push_constant_list.constants[i].index = i;
        sln_draw_model(&vulkan, model, &push_constant_list.constants[i]);
    }

    vk_render_end(vulkan);
}

/**
 * @brief Call once per frame to update the game and render
 * 
 * @param app Game information
 */
void sln_update(struct sln_app app)
{
    float rotcos = cosf(game.view.rotation.c.y);
    float rotsin = sinf(game.view.rotation.c.y);

    game.view.position.c.x += (app.controls.move.c.x * rotcos - app.controls.move.c.y * rotsin) / 10.0f;
    game.view.position.c.z += (-app.controls.move.c.x * rotsin - app.controls.move.c.y * rotcos) / 10.0f;
    game.view.scale = (union vector3){1, 1, 1};

    game.view.rotation.c.y += app.controls.look.c.x / 80.0f;
    game.view.rotation.c.x += -app.controls.look.c.y / 80.0f;

    if (game.view.rotation.c.x > DEG_TO_RAD(90))
        game.view.rotation.c.x = DEG_TO_RAD(90);
    else if (game.view.rotation.c.x < -DEG_TO_RAD(90))
        game.view.rotation.c.x = -DEG_TO_RAD(90);

    sln_render(app);
}
