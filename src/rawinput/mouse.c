void rawinput_parse_mouse_data(struct user_controls *control, RAWINPUT *data)
{
    control->is_controller = 0;

    control->look.c.x = (float)data->data.mouse.lLastX;
    control->look.c.y = (float)data->data.mouse.lLastY;
}
