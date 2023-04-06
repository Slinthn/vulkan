/**
 * @brief Initialise RawInput
 * 
 * @param window Window to attach RawInput context to
 */
void rawinput_init(HWND window)
{
    RAWINPUTDEVICE device[3] = {0};
    device[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
    device[0].hwndTarget = window;
    device[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    device[1].hwndTarget = window;
    device[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[2].usUsage = HID_USAGE_GENERIC_MOUSE;
    device[2].hwndTarget = window;
    RegisterRawInputDevices(device, SIZEOF_ARRAY(device),
            sizeof(RAWINPUTDEVICE));
}

/**
 * @brief Parse RawInput data. Should be called when a WM_INPUT event is
 *     to be handled
 * 
 * @param control Structure to return user controls in
 * @param rawinput RawInput event
 */
void rawinput_parse(struct user_controls *control, HRAWINPUT rawinput)
{
    uint32_t size;
    GetRawInputData(rawinput, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));

    RAWINPUT *data =
            VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    GetRawInputData(rawinput, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));

    switch (data->header.dwType) {
    case RIM_TYPEHID: {
        struct rawinput_dualshock4 *ds4data =
                (struct rawinput_dualshock4 *)data->data.hid.bRawData;

        rawinput_parse_dualshock4_data(control, ds4data);
    } break;
    }

    VirtualFree(data, 0, MEM_RELEASE);
}
