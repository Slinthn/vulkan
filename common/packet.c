#define PORT 12345

// CLIENT indicates packet is travelling from client to the server
// SERVER indicates opposite direction
#define PACKET_CLIENT_HEARTBEAT 0x1
#define PACKET_SERVER_PLAYER_ADD 0x2
#define PACKET_SERVER_PLAYER_REMOVE 0x3
#define PACKET_SERVER_PLAYER_MOVE 0x4
#define PACKET_SERVER_PLAYER_REGISTERED 0x5
#define PACKET_CLIENT_PLAYER_MOVE 0x6

#pragma pack(push, 1)
struct packet_header {
    uint8_t packet_id; 
};
#pragma pack(pop)

#pragma pack(push, 1)
struct packet_client_heartbeat {
    struct packet_header header;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct packet_player_add {
    struct packet_header header;
    uint32_t player_id;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct packet_player_remove {
    struct packet_header header;
    uint32_t player_id;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct packet_client_player_move {
    struct packet_header header;
    float position[3];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct packet_player_move {
    struct packet_header header;
    uint32_t player_id;
    float position[3];
};
#pragma pack(pop)

#if 0
#pragma pack(push, 1)
struct packet_player_registered {
    struct packet_header header;
    uint32_t player_id;
};
#pragma pack(pop)
#endif
