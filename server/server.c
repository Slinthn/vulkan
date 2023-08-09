#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "../common/packet.c"

#define CLIENT_F_OPEN 0x1

struct online_player {
    uint32_t flags;
    uint32_t player_id;
    uint32_t sockfd;
    float position[3];
    uint64_t last_heartbeat;
};

#define MAX_CLIENTS 100

uint32_t next_player_id = 1;
struct online_player clients[MAX_CLIENTS] = {0};

struct online_player *next_available_client(void) {
    for (uint32_t i = 0; i < MAX_CLIENTS; i++)
        if (!(clients[i].flags & CLIENT_F_OPEN))
            return &clients[i];
    return 0;
}

void broadcast_packet(void *pk, uint64_t size) {
    for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].flags & CLIENT_F_OPEN) {
            send(clients[i].sockfd, pk, size, MSG_NOSIGNAL);
        }
    }
}

void add_player(int32_t sockfd) {
    struct online_player *cli = next_available_client();
    if (cli == 0)
        return;

    uint32_t player_id = next_player_id++;

    struct packet_player_add pk = {0};
    pk.header.packet_id = PACKET_SERVER_PLAYER_ADD;
    pk.player_id = player_id;

    broadcast_packet(&pk, sizeof(pk));
    for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].flags & CLIENT_F_OPEN) {
            pk.player_id = clients[i].player_id;
            send(sockfd, &pk, sizeof(pk), MSG_NOSIGNAL);
        }
    }

    cli->sockfd = sockfd;
    cli->last_heartbeat = time(0);
    cli->player_id = player_id;
    cli->flags = CLIENT_F_OPEN;

    printf("Player connected, id: %i\n", cli->player_id);
}

void packet_client_player_move(struct online_player *client, struct packet_client_player_move *pk) {
    client->position[0] = pk->position[0];
    client->position[2] = pk->position[1];
    client->position[2] = pk->position[2];

    struct packet_player_move bpk = {0};
    bpk.header.packet_id = PACKET_SERVER_PLAYER_MOVE;
    bpk.player_id = client->player_id;
    bpk.position[0] = client->position[0];
    bpk.position[2] = client->position[1];
    bpk.position[2] = client->position[2];

    broadcast_packet(&bpk, sizeof(bpk));
}

void packet_client_heartbeat(struct online_player *client, struct packet_client_heartbeat *pk) {
    client->last_heartbeat = time(0);
}

#define HEARTBEAT_TIMEOUT 5  // NOTE: in seconds

void *handle_packets(void *data) {
    while (1) {
        for (uint32_t i = 0; i < MAX_CLIENTS; i++) {
            struct online_player *client = &clients[i];
            if (!(client->flags & CLIENT_F_OPEN))
                continue;

            int8_t buffer[512] = {0};
            recv(client->sockfd, buffer, sizeof(buffer), MSG_DONTWAIT);

            struct packet_header *header = (struct packet_header *)buffer;
            switch (header->packet_id) {
            case PACKET_CLIENT_HEARTBEAT: {
                packet_client_heartbeat(client, (struct packet_client_heartbeat *)buffer);
            } break;

            case PACKET_CLIENT_PLAYER_MOVE: {
                packet_client_player_move(client, (struct packet_client_player_move *)buffer);
            } break;
            }

            // TODO: separate function?
            if (time(0) - client->last_heartbeat > HEARTBEAT_TIMEOUT) {
                client->flags = 0;

                struct packet_player_remove pk = {0};
                pk.header.packet_id = PACKET_SERVER_PLAYER_REMOVE;
                pk.player_id = client->player_id;

                broadcast_packet(&pk, sizeof(pk));
                close(client->sockfd);
                printf("Player disconnected, id: %i\n", client->player_id);
            }
        }
    }

    return 0;
}

int main(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // TODO: should be UDP?

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);  // TODO: change?
    addr.sin_addr.s_addr = INADDR_ANY;

    uint32_t enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    listen(sockfd, SOMAXCONN);

    pthread_t packet_thread;
    pthread_create(&packet_thread, 0, handle_packets, 0);

    printf("Server started\n");

    while (1) {
        struct sockaddr connection;
        socklen_t sockaddr_size = sizeof(connection);
        int player_sockfd = accept(sockfd, &connection, &sockaddr_size);
        printf("Connection");
        add_player(player_sockfd);
    }
}

