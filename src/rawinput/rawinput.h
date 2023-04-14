#define ACTION_JUMP 0x1
#define ACTION_DESCEND 0x2
#define ACTION_ASCEND 0x4

#pragma pack(push, 1)
struct ri_ds4 {
  uint8_t reportid;
  uint8_t lx;
  uint8_t ly;
  uint8_t rx;
  uint8_t ry;
  uint16_t buttons;
  uint8_t counter;
  uint8_t l2;
  uint8_t r2;
};
#pragma pack(pop)

struct user_controls {
  union vector2 move;
  union vector2 look;
  uint64_t actions;
  uint64_t is_controller;
  uint8_t keypress[4];
  uint32_t unused0;
};

#include "keyboard.c"
#include "mouse.c"
#include "dualshock4.c"
#include "rawinput.c"
