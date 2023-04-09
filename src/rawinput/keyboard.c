void rawinput_parse_keyboard_data(struct user_controls *control, RAWINPUT *data)
{
    uint8_t down = !(data->data.keyboard.Flags & RI_KEY_BREAK);
    uint16_t key = data->data.keyboard.VKey;

    control->is_controller = 0;

    switch (key) {
    case 'W': {
        control->move.c.y = (float)-down;
    } break;

    case 'S': {
        control->move.c.y = down;
    } break;

    case 'D': {
        control->move.c.x = down;
    } break;

    case 'A': {
        control->move.c.x = (float)-down;
    } break;
    
    case VK_SHIFT: {
        if (down)
            control->actions |= ACTION_DESCEND;
        else
            control->actions ^= ACTION_DESCEND;
    } break;

    case ' ': {
        if (down)
            control->actions |= ACTION_ASCEND;
        else
            control->actions ^= ACTION_ASCEND;
    } break;
    }
}