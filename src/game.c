#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720
#define SLN_FRAMEBUFFER_COUNT 3  // TODO: temp 3 for linux

#include "file.c"
#include "world/world.h"
#include "physics/physics.h"

#ifdef SLN_VULKAN

#ifdef SLN_WIN64
#define VK_USE_PLATFORM_WIN32_KHR
#endif  // SLN_VULKAN

#ifdef SLN_X11
#define VK_USE_PLATFORM_XCB_KHR
#endif  // SLN_X11

#pragma warning(push, 0)
#include <vulkan/vulkan.h>
#pragma warning(pop)

#include "vulkan/vulkan.h"

#endif  // SLN_VULKAN

#ifdef SLN_XAUDIO2

#pragma warning(push, 0)
#include <xaudio2.h>
#pragma warning(pop)

#include "xaudio2/xaudio2.h"

#endif  // SLN_XAUDIO2

struct sln_player {
    struct transform tf;
};

struct sln_state {
    struct sln_player player;
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

    // TODO: tmp audio_init();

    app->resources.world = sln_load_sw("world.sw");

    app->game.graphics_world = graphics_load_sw(app->game.graphics,
        app->resources.world);
    app->game.physics_world = physics_load_sw(app->resources.world);

    app->game.physics_world.player.dimension = (union vector3){0.5f, 6, 0.5f};
}

/**
 * @brief Parse movements for the player
 * 
 * @param player The player
 * @param move Position change
 * @param look Look direction change
 */
void sln_player(
    struct sln_player *player,
    union vector2 move,
    union vector2 look
){
    float rotcos = cosf(player->tf.rotation.y);
    float rotsin = sinf(player->tf.rotation.y);

    player->tf.position.x += (move.x * rotcos - move.y * rotsin) / 4.0f;
    player->tf.position.z += (-move.x * rotsin - move.y * rotcos) / 4.0f;
    player->tf.position.y = -5;
    player->tf.scale = (union vector3){1, 1, 1};

    player->tf.rotation.y += look.x / 80.0f;
    player->tf.rotation.x += -look.y / 80.0f;

    if (player->tf.rotation.x > DEG_TO_RAD(90))
        player->tf.rotation.x = DEG_TO_RAD(90);
    else if (player->tf.rotation.x < -DEG_TO_RAD(90))
        player->tf.rotation.x = -DEG_TO_RAD(90);
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

    sln_player(&game->player, app->controls.move, app->controls.look);

    game->physics_world.player.centre.x = game->player.tf.position.x;
    game->physics_world.player.centre.y = game->player.tf.position.y;
    game->physics_world.player.centre.z = game->player.tf.position.z;

    physics_run(&game->physics_world);

    game->player.tf.position.x = game->physics_world.player.centre.x;
    game->player.tf.position.y = game->physics_world.player.centre.y;
    game->player.tf.position.z = game->physics_world.player.centre.z;

    graphics_render(&game->graphics, *app, game->player.tf,
        game->graphics_world);
}
