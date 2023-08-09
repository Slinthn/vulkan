/**
 * @brief Parse mouse data for RawInput
 * 
 * @param control Returns user controls
 * @param data Stream of bytes from mouse
 */
void rawinput_parse_mouse_data(
    struct user_controls *control,
    RAWINPUT *data
){
    control->is_controller = 0;

    control->look.x = (float)data->data.mouse.lLastX / 5.0f; // TODO: sensitivity?
    control->look.y = (float)data->data.mouse.lLastY / 5.0f; // TODO: time delta
}
