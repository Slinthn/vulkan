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
#include "world/world.h"
#include "physics/physics.h"

struct sln_resources {
    struct sw_world world;
};

struct sln_state {
    struct transform view;
    struct graphics_state graphics;
    struct graphics_world graphics_world;
    struct physics_world physics_world;
};

#include "vulkan/world.c"
#include "vulkan/render.c"

// TODO: daz no good...
struct sln_state game;
struct sln_resources resources;

/**
 * @brief Initialise the game
 * 
 * @param surface OS Graphics surface
 */
void sln_init(
    struct graphics_surface surface
){
    game.graphics = graphics_init(surface);

    // TODO: no good
    resources.world = sln_load_sw(/*graphics.device, graphics.physical_device,
        graphics.command_pool, graphics.queue.type.graphics, graphics.sampler,
        graphics.pool, graphics.set_layout[1], */"world.sw");

    game.graphics_world = graphics_load_sw(game.graphics, resources.world);
    game.physics_world = physics_load_sw(resources.world);

    game.physics_world.player.dimension = (union vector3){0.4f, 3, 0.4f};
}

/**
 * @brief Call once per frame to update the game and render
 * 
 * @param app Game information
 */
void sln_update(
    struct sln_app app
){
    float rotcos = cosf(game.view.rotation.y);
    float rotsin = sinf(game.view.rotation.y);

    union vector2 move = app.controls.move;
    union vector2 look = app.controls.look;

    game.view.position.x += (move.x * rotcos - move.y * rotsin) / 4.0f;
    game.view.position.z += (-move.x * rotsin - move.y * rotcos) / 4.0f;
    game.view.position.y = -3;
    game.view.scale = (union vector3){1, 1, 1};

    game.view.rotation.y += look.x / 80.0f;
    game.view.rotation.x += -look.y / 80.0f;

    if (game.view.rotation.x > DEG_TO_RAD(90))
        game.view.rotation.x = DEG_TO_RAD(90);
    else if (game.view.rotation.x < -DEG_TO_RAD(90))
        game.view.rotation.x = -DEG_TO_RAD(90);

    game.physics_world.player.centre.x = game.view.position.x;
    game.physics_world.player.centre.y = game.view.position.y;
    game.physics_world.player.centre.z = game.view.position.z;

    physics_run(&game.physics_world);

    game.view.position.x = game.physics_world.player.centre.x;
    game.view.position.y = game.physics_world.player.centre.y;
    game.view.position.z = game.physics_world.player.centre.z;

    graphics_render(&game.graphics, app, game.view, game.graphics_world);
}
