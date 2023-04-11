void rawinput_parse_keyboard_data(struct user_controls *control, RAWINPUT *data)
{
    uint8_t down = !(data->data.keyboard.Flags & RI_KEY_BREAK);
    uint16_t key = data->data.keyboard.VKey;

    control->is_controller = 0;

    switch (key) {
    case 'W': {
        control->keypress[0] = down;
    } break;

    case 'S': {
        control->keypress[2] = down;
    } break;

    case 'D': {
        control->keypress[3] = down;
    } break;

    case 'A': {
        control->keypress[1] = down;
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

    control->move.c.x = 0;
    control->move.c.y = 0;

    if (control->keypress[0] != control->keypress[2])
        if (control->keypress[0])
            control->move.c.y = -1;
        else if (control->keypress[2])
            control->move.c.y = 1;

    if (control->keypress[1] != control->keypress[3])
        if (control->keypress[1])
            control->move.c.x = -1;
        else if (control->keypress[3])
            control->move.c.x = 1;
}