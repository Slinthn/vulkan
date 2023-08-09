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
};

struct sln_app {
    uint32_t width;
    uint32_t height;
    uint64_t sockfd;
    struct user_controls controls;
    struct sln_state game;
    struct sln_resources resources;
};

#include "vulkan/world.c"
#include "vulkan/render.c"
#include "../common/packet.c"

struct sln_online_player *get_online_player(struct sln_state *state, uint32_t player_id) {
    for (uint32_t i = 0; i < MAX_ONLINE_PLAYERS; i++) {
        struct sln_online_player *op = &state->world.online_players[i];
        if (op->flags & SLN_ONLINE_PLAYER_F_ONLINE && op->player_id == player_id)
            return op;
    }
    return 0;
}

void packet_player_add(struct packet_player_add *packet, struct sln_state *state) {
    if (packet->player_id == 0 || get_online_player(state, packet->player_id) != 0)
        return;

    struct sln_online_player *available = 0;

    for (uint32_t i = 0; i < MAX_ONLINE_PLAYERS; i++) {
        struct sln_online_player *op = &state->world.online_players[i];
        if (!(op->flags & SLN_ONLINE_PLAYER_F_ONLINE)) {
            available = op;
            break;
        }
    }

    if (!available)
        return;

    available->flags = SLN_ONLINE_PLAYER_F_ONLINE;
    available->player_id = packet->player_id;
}

void packet_player_remove(struct packet_player_remove *packet, struct sln_state *state) {
    if (packet->player_id == 0)
        return;

    struct sln_online_player *player = get_online_player(state, packet->player_id);
    if (!player)
        return;

    *player = (struct sln_online_player){0};
}

void packet_player_move(struct packet_player_move *pk, struct sln_state *state) {
    if (pk->player_id == 0)
        return;

    struct sln_online_player *player = get_online_player(state, pk->player_id);
    if (!player)
        return;

    player->tf.position.x = pk->position[0];
    player->tf.position.y = pk->position[1];
    player->tf.position.z = pk->position[2];
}

DWORD packet_handler(void *data) {
    struct sln_app *app = (struct sln_app *)data;
    while (1) {
        int8_t buffer[512] = {0};
        recv(app->sockfd, buffer, sizeof(buffer), 0);
        
        struct packet_header *header = (struct packet_header *)buffer;
        switch (header->packet_id) {
        case PACKET_SERVER_PLAYER_ADD: {
            packet_player_add((struct packet_player_add *)buffer, &app->game);
        } break;
        case PACKET_SERVER_PLAYER_REMOVE: {
            packet_player_remove((struct packet_player_remove *)buffer, &app->game);
        } break;
        case PACKET_SERVER_PLAYER_MOVE: {
            packet_player_move((struct packet_player_move *)buffer, &app->game);
        } break;
        }
    }
    return 0;
}

void connect_server(struct sln_app *app) {
    PADDRINFOA addr_info;
    getaddrinfo("vulkan.slin.ie", 0, 0, &addr_info);

    struct sockaddr_in *tmp = (struct sockaddr_in *)addr_info->ai_addr;
    tmp->sin_port = htons(PORT);

    app->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(app->sockfd, (struct sockaddr *)tmp, sizeof(struct sockaddr_in));
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
    connect_server(app);
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
    player->tf.rotation.x += -look.y / 80.0f;  // TODO: sensitivity

    if (player->tf.rotation.x > DEG_TO_RAD(90))
        player->tf.rotation.x = DEG_TO_RAD(90);
    else if (player->tf.rotation.x < -DEG_TO_RAD(90))
        player->tf.rotation.x = -DEG_TO_RAD(90);
}

void send_move_packet(struct sln_app *app) {
    struct packet_client_player_move pk = {0};
    pk.header.packet_id = PACKET_CLIENT_PLAYER_MOVE;
    pk.position[0] = app->game.world.player.tf.position.x;
    pk.position[1] = app->game.world.player.tf.position.y;
    pk.position[2] = app->game.world.player.tf.position.z;
    send(app->sockfd, (const char *)&pk, sizeof(pk), 0);
}

void send_heartbeat_packet(struct sln_app *app) {
    struct packet_client_heartbeat pk = {0};
    pk.header.packet_id = PACKET_CLIENT_HEARTBEAT;
    send(app->sockfd, (const char *)&pk, sizeof(pk), 0);
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

    graphics_render(&game->graphics, *app, game->world.player.tf,
        game->graphics_world);

    send_heartbeat_packet(app);
    send_move_packet(app);
}

