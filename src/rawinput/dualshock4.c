#define DS4_DEADZONE 0.35

/**
 * @brief TODO:
 * 
 * @param value 
 * @return float 
 */
float rawinput_deadzone(float value)
{
    return (-DS4_DEADZONE < value && value < DS4_DEADZONE) ? 0 : value;
}

/**
 * @brief Parse DS4 data for RawInput
 * 
 * @param control Returns user controls
 * @param data Stream of bytes from controller
 */
void rawinput_parse_dualshock4_data(
    struct user_controls *control,
    struct rawinput_dualshock4 *data
){
    uint64_t actions = 0;
    if (data->buttons & (1 << 5))
        actions |= ACTION_JUMP;

    if (data->buttons & (1 << 10))
        actions |= ACTION_DESCEND;

    if (data->buttons & (1 << 11))
        actions |= ACTION_ASCEND;

    float mx = rawinput_deadzone((data->lx - 127.5f) / 127.5f);
    float my = rawinput_deadzone((data->ly - 127.5f) / 127.5f);
    float lx = rawinput_deadzone((data->rx - 127.5f) / 127.5f);
    float ly = rawinput_deadzone((data->ry - 127.5f) / 127.5f);

    // Check if any action is recorded by the controller of the user
    if (mx || my || lx || ly || actions)
        control->is_controller = 1;

    if (control->is_controller) {
        control->actions = actions;
        control->move.c.x = mx;
        control->move.c.y = my;
        control->look.c.x = lx;
        control->look.c.y = ly;
    }
}
