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

#pragma pack(push, 1)
struct st_header {
    uint8_t signature[4];
    uint32_t width, height;
};
#pragma pack(pop)

struct vk_model create_terrain(struct graphics_state *gs) {
    struct vk_model res = {0};

    struct sln_file f = sln_read_file("terrain.st", 1);
    struct st_header *head = (struct st_header *)f.data;
    float *heights = (float *)((uint64_t)head + sizeof(struct st_header));

    struct vk_vertex *vert = malloc(head->width * head->height * sizeof(struct vk_vertex));
    uint32_t indices_size = (head->width - 1) * (head->height - 1) * 6;
    uint32_t *indices = malloc(indices_size * sizeof(uint32_t));

    // Positions
    for (uint32_t i = 0; i < head->width; i++) {
        for (uint32_t j = 0; j < head->height; j++) {
            struct vk_vertex *v = &vert[i * head->width + j];
            *v = (struct vk_vertex){0};
            v->position = (union vector3){(float)i, heights[i * head->width + j] / 10.0f, (float)j};
            v->normal = (union vector3){0, 1, 0};
        }
    }

    // Normals
    // Note, i don't care about the edge vertices as of right now.
    for (uint32_t i = 1; i < head->width - 1; i++) {
        for (uint32_t j = 1; j < head->height - 1; j++) {
            struct vk_vertex *v0 = &vert[(i - 1) * head->width + j];
            struct vk_vertex *v1 = &vert[(i + 1) * head->width + j];
            struct vk_vertex *v2 = &vert[i * head->width + j - 1];
            struct vk_vertex *v3 = &vert[i * head->width + j + 1];
            struct vk_vertex *v = &vert[i * head->width + j];

#if 0
            union vector3 e0 = vec3_sub(v0->position, v->position);
            union vector3 e1 = vec3_sub(v1->position, v->position);
            union vector3 e2 = vec3_sub(v2->position, v->position);
            union vector3 e3 = vec3_sub(v3->position, v->position);
            union vector3 cross0 = vec3_cross(e0, e1);
            union vector3 cross1 = vec3_cross(e0, e2);
            union vector3 cross2 = vec3_cross(e0, e3);
            union vector3 cross3 = vec3_cross(e1, e2);
            union vector3 cross4 = vec3_cross(e1, e3);
            union vector3 cross5 = vec3_cross(e2, e3);
#endif
            
#if 0
            union vector3 cross = vec3_add(cross0, cross1);
            cross = vec3_add(cross, cross2);
            cross = vec3_add(cross, cross3);
            cross = vec3_add(cross, cross4);
            cross = vec3_add(cross, cross5);
            cross.y = 2;
            cross = vec3_norm(cross);
#endif

            v->normal.x = v2->position.y - v3->position.y;
            v->normal.z = v0->position.y - v1->position.y;
            v->normal.y = 1;
            v->normal = vec3_norm(v->normal);
            //v->normal = cross;
        }
    }

    uint32_t counter = 0;
    for (uint32_t i = 0; i < indices_size; i += 6) {
#if 1
        if ((counter + 1) % head->width == 0)
            counter += 1;

        indices[i] = counter + 0;
        indices[i + 2] = counter + 1;
        indices[i + 1] = counter + 0 + head->width;
        indices[i + 3] = counter + 0 + head->width;
        indices[i + 5] = counter + 1;
        indices[i + 4] = counter + 1 + head->width;
        counter++;
#endif
    }

    res.vertex_buffer = vk_create_vertex_buffer(gs->device, gs->physical_device, vert, sizeof(struct vk_vertex) * head->width * head->height);
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
    tf.position.y = -40;
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

