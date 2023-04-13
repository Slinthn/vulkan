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
#include "vulkan/vulkan.h"
#include "physics/physics.h"
#include "world/world.h"

struct sln_resources {
    struct sln_world world;
};

struct sln_state {
    struct transform view;
    struct point_cuboid player;
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
void sln_init(
    struct vk_surface surface
){
    vulkan = vk_init(surface);

    resources.world = sln_load_sw(vulkan.device, vulkan.physical_device,
        vulkan.command_pool, vulkan.queue.type.graphics, vulkan.sampler,
        vulkan.pool, vulkan.set_layout[1], "world.sw");

    game.player.dimension = (union vector3){0.4f, 3, 0.4f};
}

void sln_render_all_objects(
    void
){
    struct vk_model *models = resources.world.models;
    struct vk_texture *textures = resources.world.textures;
    struct sln_object *objects = resources.world.objects;

    for (uint32_t i = 0; i < SIZEOF_ARRAY(resources.world.objects); i++) {
        if (!(objects[i].flags & SLN_WORLD_FLAG_EXISTS))
            continue;

        struct vk_model model = models[objects[i].model_index];
        struct vk_texture texture = textures[objects[i].texture_index];
        push_constant_list.constants[i].index = i;
        sln_draw_model(&vulkan, model, texture,
            &push_constant_list.constants[i]);
    }
}

/**
 * @brief Render all objects TODO:
 * 
 */
void sln_render(
    struct sln_app app
){
    struct sln_object *objects = resources.world.objects;

    // Constant buffer 0
    struct vk_uniform_buffer0 buf0 = {0};
    mat4_perspective(&buf0.projection,
        SLN_WINDOW_HEIGHT / (float)SLN_WINDOW_WIDTH, DEG_TO_RAD(90),
        0.1f, 100.0f);
    mat4_transform(&buf0.view, game.view);

    mat4_orthographic(&buf0.camera_projection,
        -10, 10, -10, 10, 1, 40.0f);

    struct transform camera_view = {0};
    camera_view.position = (union vector3){0, -10, 0};
    camera_view.rotation = (union vector3){-DEG_TO_RAD(40), 0, 0};
    camera_view.scale = (union vector3){1, 1, 1};

    mat4_transform(&buf0.camera_view, camera_view);

    // Constant buffer 1
    struct vk_uniform_buffer1 buf1 = {0};
    for (uint32_t i = 0; i < SIZEOF_ARRAY(resources.world.objects); i++)
        if (objects[i].flags & SLN_WORLD_FLAG_EXISTS)
            mat4_transform(&buf1.model[i], objects[i].transform);

    // Render
    vk_render_begin(&vulkan, &buf0, &buf1);

    vk_render_shadow(&vulkan);
    sln_render_all_objects();
    vk_render_main(&vulkan, (float[4]){1, 0, 1, 1}, app.width, app.height);
    sln_render_all_objects();

    vk_render_end(vulkan);
}

/**
 * @brief Call once per frame to update the game and render
 * 
 * @param app Game information
 */
void sln_update(
    struct sln_app app
){
    float rotcos = cosf(game.view.rotation.c.y);
    float rotsin = sinf(game.view.rotation.c.y);

    union vector2 move = app.controls.move;
    union vector2 look = app.controls.look;

    game.view.position.c.x += (move.c.x * rotcos - move.c.y * rotsin) / 4.0f;
    game.view.position.c.z += (-move.c.x * rotsin - move.c.y * rotcos) / 4.0f;
    game.view.position.c.y = -3;
    game.view.scale = (union vector3){1, 1, 1};

    game.view.rotation.c.y += look.c.x / 80.0f;
    game.view.rotation.c.x += -look.c.y / 80.0f;

    if (game.view.rotation.c.x > DEG_TO_RAD(90))
        game.view.rotation.c.x = DEG_TO_RAD(90);
    else if (game.view.rotation.c.x < -DEG_TO_RAD(90))
        game.view.rotation.c.x = -DEG_TO_RAD(90);

    game.player.centre.c.x = game.view.position.c.x;
    game.player.centre.c.y = game.view.position.c.y;
    game.player.centre.c.z = game.view.position.c.z;

    physics_run(resources.world.physics, &game.player);

    game.view.position.c.x = game.player.centre.c.x;
    game.view.position.c.y = game.player.centre.c.y;
    game.view.position.c.z = game.player.centre.c.z;

    sln_render(app);
}
