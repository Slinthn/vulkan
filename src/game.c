#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720
#define SLN_FRAMEBUFFER_COUNT 3  // TODO: temp 3 for linux

#ifdef SLN_VULKAN
#pragma warning(push, 0)
#ifdef SLN_WIN64
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(SLN_X11)
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.h>
#pragma warning(pop)
#endif  // SLN_VULKAN

#include "file.c"
#include "vulkan/vulkan.h"
#include "world/world.h"
#include "physics/physics.h"

struct sln_state {
    struct transform view;
    struct graphics_state graphics;
    struct graphics_world graphics_world;
    struct physics_world physics_world;
};

struct sln_resources {
    struct sw_world world;
};

struct sln_app {
    uint32_t width;
    uint32_t height;
    struct user_controls controls;
    struct sln_state game;
    struct sln_resources resources;
};

#include "vulkan/world.c"
#include "vulkan/render.c"

// TODO: daz no good...

/**
 * @brief Initialise the game TODO:
 * 
 * @param surface OS Graphics surface
 */
void sln_init(
    struct sln_app *app,
    struct graphics_surface surface
){
    app->game.graphics = graphics_init(surface);

    // TODO: no good
    app->resources.world = sln_load_sw(/*graphics.device, graphics.physical_device,
        graphics.command_pool, graphics.queue.type.graphics, graphics.sampler,
        graphics.pool, graphics.set_layout[1], */"world.sw");

    app->game.graphics_world = graphics_load_sw(app->game.graphics,
        app->resources.world);
    app->game.physics_world = physics_load_sw(app->resources.world);

    app->game.physics_world.player.dimension = (union vector3){0.4f, 3, 0.4f};
}

/**
 * @brief Call once per frame to update the game and render
 * 
 * @param app Game information
 */
void sln_update(
    struct sln_app *app
){
    struct sln_state *game = &app->game;

    float rotcos = cosf(game->view.rotation.y);
    float rotsin = sinf(game->view.rotation.y);

    union vector2 move = app->controls.move;
    union vector2 look = app->controls.look;

    game->view.position.x += (move.x * rotcos - move.y * rotsin) / 4.0f;
    game->view.position.z += (-move.x * rotsin - move.y * rotcos) / 4.0f;
    game->view.position.y = -3;
    game->view.scale = (union vector3){1, 1, 1};

    game->view.rotation.y += look.x / 80.0f;
    game->view.rotation.x += -look.y / 80.0f;

    if (game->view.rotation.x > DEG_TO_RAD(90))
        game->view.rotation.x = DEG_TO_RAD(90);
    else if (game->view.rotation.x < -DEG_TO_RAD(90))
        game->view.rotation.x = -DEG_TO_RAD(90);

    game->physics_world.player.centre.x = game->view.position.x;
    game->physics_world.player.centre.y = game->view.position.y;
    game->physics_world.player.centre.z = game->view.position.z;

    physics_run(&game->physics_world);

    game->view.position.x = game->physics_world.player.centre.x;
    game->view.position.y = game->physics_world.player.centre.y;
    game->view.position.z = game->physics_world.player.centre.z;

    graphics_render(&game->graphics, *app, game->view, game->graphics_world);
}
