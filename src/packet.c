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

