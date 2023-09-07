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

#define SLN_ONLINE_PLAYER_F_ONLINE 0x1

struct sln_online_player {
    uint32_t player_id;  // TODO: typedef PLAYERID
    uint32_t flags;
    struct transform tf;
};

#define MAX_ONLINE_PLAYERS 100

struct sln_world {
    struct sln_player player;
    struct sln_online_player online_players[MAX_ONLINE_PLAYERS];
};

struct sln_state {
    struct graphics_state graphics;
    struct graphics_world graphics_world;
    struct physics_world physics_world;
    struct sln_world world;
};

struct sln_resources {
    struct sw_world world;
    struct vk_model terrain;
};

struct sln_app {
    uint32_t width;
    uint32_t height;
    uint64_t sockfd;
    struct user_controls controls;
    struct sln_state game;
    struct sln_resources resources;
};

#include "array.c"
#include "vulkan/world.c"
#include "vulkan/render.c"
#include "../common/packet.c"
#include "packet.c"

struct vk_model create_terrain(struct graphics_state *gs) {
    struct vk_model res = {0};

#define TERRAIN_SIZE 100

    struct vk_vertex *vert = malloc(TERRAIN_SIZE * TERRAIN_SIZE * sizeof(struct vk_vertex));
    uint32_t indices_size = (TERRAIN_SIZE - 1) * (TERRAIN_SIZE - 1) * 6;
    uint32_t *indices = malloc(indices_size * sizeof(uint32_t));

    float gradient = 0;
    for (uint32_t i = 0; i < TERRAIN_SIZE; i++) {
        for (uint32_t j = 0; j < TERRAIN_SIZE; j++) {
#if 1
            struct vk_vertex *v = &vert[i * TERRAIN_SIZE + j];
            *v = (struct vk_vertex){0};
            v->position = (union vector3){(float)i, 11 * rand() / (float)RAND_MAX, (float)j};
            v->normal = (union vector3){0, 1, 0};

            gradient += 0.01f;
#endif
        }
        gradient -= 0.2f;
    }

    uint32_t counter = 0;
    for (uint32_t i = 0; i < indices_size; i += 6) {
#if 1
        if ((counter + 1) % TERRAIN_SIZE == 0)
            counter += 1;

        indices[i] = counter + 0;
        indices[i + 2] = counter + 1;
        indices[i + 1] = counter + 0 + TERRAIN_SIZE;
        indices[i + 3] = counter + 0 + TERRAIN_SIZE;
        indices[i + 5] = counter + 1;
        indices[i + 4] = counter + 1 + TERRAIN_SIZE;
        counter++;
#endif
    }

    res.vertex_buffer = vk_create_vertex_buffer(gs->device, gs->physical_device, vert, sizeof(struct vk_vertex) * TERRAIN_SIZE * TERRAIN_SIZE);
    res.index_buffer = vk_create_index_buffer(gs->device, gs->physical_device, indices, indices_size);

    return res;
}

/**
 * @brief Initialise the game TODO:
 * 
 * @param surface OS Graphics surface
 */
void sln_init(
    struct sln_app *app,
    struct graphics_surface surface
){
    // TODO: tmp connect_server(app);
    app->game.graphics = graphics_init(surface);

    app->game.graphics.anim = load_animation("armature.sa");

    // TODO: tmp audio_init();

    //app->resources.world = sln_load_sw("world.sw");
    app->resources.world.model_count = 1;
    app->resources.world.models[0].filename[0] = 'f';
    app->resources.world.models[0].filename[1] = 'r';
    app->resources.world.models[0].filename[2] = 'o';
    app->resources.world.models[0].filename[3] = 'g';
    app->resources.world.models[0].filename[4] = 'g';
    app->resources.world.models[0].filename[5] = 'y';
    app->resources.world.models[0].filename[6] = '.';
    app->resources.world.models[0].filename[7] = 's';
    app->resources.world.models[0].filename[8] = 'm';

    app->resources.world.object_count = 1;
    app->resources.world.objects[0].model_index = 0;
    app->resources.world.objects[0].texture_index = 0;
    app->resources.world.objects[0].scale[0] = 1;
    app->resources.world.objects[0].scale[1] = 1;
    app->resources.world.objects[0].scale[2] = 1;

    app->game.graphics_world = graphics_load_sw(app->game.graphics,
        app->resources.world);
    app->game.physics_world = physics_load_sw(app->resources.world);

    app->game.physics_world.player.dimension = (union vector3){0.5f, 6, 0.5f};

    app->resources.terrain = create_terrain(&app->game.graphics);
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

#if 1
    player->tf.rotation.y += look.x / 80.0f;
    player->tf.rotation.x += -look.y / 80.0f;  // TODO: sensitivity

    if (player->tf.rotation.x > DEG_TO_RAD(90))
        player->tf.rotation.x = DEG_TO_RAD(90);
    else if (player->tf.rotation.x < -DEG_TO_RAD(90))
        player->tf.rotation.x = -DEG_TO_RAD(90);
#endif
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

    sln_player(&game->world.player, app->controls.move, app->controls.look);

    game->physics_world.player.centre.x = game->world.player.tf.position.x;
    game->physics_world.player.centre.y = game->world.player.tf.position.y;
    game->physics_world.player.centre.z = game->world.player.tf.position.z;

    physics_run(&game->physics_world);

    game->world.player.tf.position.x = game->physics_world.player.centre.x;
    game->world.player.tf.position.y = game->physics_world.player.centre.y;
    game->world.player.tf.position.z = game->physics_world.player.centre.z;

#if 0
    // TODO: temp, objects 50+ are online players. if more than 50 players,
    // then fuck.
    for (uint32_t i = 0; i < MAX_ONLINE_PLAYERS; i++) {
        struct sln_online_player *op = &game->world.online_players[i];
        if (!(op->flags & SLN_ONLINE_PLAYER_F_ONLINE)) {
            game->graphics_world.objects[50 + i].flags = 0;
            continue;
        }

        game->graphics_world.objects[50 + i].flags = VK_FLAG_EXISTS;
        game->graphics_world.objects[50 + i].model = &game->graphics_world.models[0];
        game->graphics_world.objects[50 + i].texture = &game->graphics_world.textures[0];
        game->graphics_world.objects[50 + i].transform.position.x = op->tf.position.x;
        game->graphics_world.objects[50 + i].transform.position.y = op->tf.position.y;
        game->graphics_world.objects[50 + i].transform.position.z = op->tf.position.z;

        game->graphics_world.objects[50 + i].transform.scale.x = 1;
        game->graphics_world.objects[50 + i].transform.scale.y = 1;
        game->graphics_world.objects[50 + i].transform.scale.z = 1;
    }
#endif

    struct transform tf = {0};
    tf.position.x = game->world.player.tf.position.x;
    tf.position.z = game->world.player.tf.position.z;
    tf.position.y = -20;
    tf.rotation.x = game->world.player.tf.rotation.x;
    tf.rotation.y = game->world.player.tf.rotation.y;
    tf.rotation.z = game->world.player.tf.rotation.z;
    //tf.rotation.x = DEG_TO_RAD(-90);
    tf.scale = (union vector3){1, 1, 1};

    game->graphics_world.objects[50].model = &app->resources.terrain;
    game->graphics_world.objects[50].texture = &game->graphics_world.textures[0];
    game->graphics_world.objects[50].transform.position = (union vector3){0, -5, 0};
    game->graphics_world.objects[50].transform.rotation = (union vector3){0};
    game->graphics_world.objects[50].transform.scale = (union vector3){1, 1, 1};
    game->graphics_world.objects[50].flags = VK_FLAG_EXISTS;

    graphics_render(&game->graphics, *app, tf, game->graphics_world);

    //send_heartbeat_packet(app);
    //send_move_packet(app);
}

