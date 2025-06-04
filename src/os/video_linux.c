#include <gdfe/os/video.h>

#ifdef OS_LINUX
    #include <X11/XKBlib.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/cursorfont.h>
    #include <X11/extensions/XInput2.h>
    #include <X11/extensions/Xrandr.h>
    #include <gdfe/collections/hashmap.h>
    #include <gdfe/collections/list.h>
    #include <gdfe/event.h>
    #include <gdfe/logging.h>
    #include <gdfe/mem.h>
    #include <i_input.h>
    #include <i_video.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_xlib.h>

static Display* x_display         = NULL;
static int      x_screen          = 0;
static Window   x_root_window     = 0;
static Atom     wm_delete_window  = 0;
static Atom     wm_protocols      = 0;
static int      xi_opcode         = 0;
static u16      current_window_id = 0;

static GDF_HashMap(Window, GDF_Window) windows;

typedef struct GDF_Window_T {
    u16      id;
    Window   x_window;
    u32      client_w;
    u32      client_h;
    i32      x;
    i32      y;
    GDF_BOOL cursor_grabbed;
    Cursor   hidden_cursor;
} GDF_Window_T;

// mapping from X11 keysyms to GDF_KEYCODE
static GDF_KEYCODE translate_key(KeySym keysym)
{
    switch (keysym)
    {
    case XK_BackSpace:
        return GDF_KEYCODE_BACKSPACE;
    case XK_Tab:
        return GDF_KEYCODE_TAB;
    case XK_Return:
        return GDF_KEYCODE_ENTER;
    case XK_Pause:
        return GDF_KEYCODE_PAUSE;
    case XK_Escape:
        return GDF_KEYCODE_ESCAPE;
    case XK_space:
        return GDF_KEYCODE_SPACE;
    case XK_End:
        return GDF_KEYCODE_END;
    case XK_Home:
        return GDF_KEYCODE_HOME;
    case XK_Left:
        return GDF_KEYCODE_LEFT;
    case XK_Up:
        return GDF_KEYCODE_UP;
    case XK_Right:
        return GDF_KEYCODE_RIGHT;
    case XK_Down:
        return GDF_KEYCODE_DOWN;
    case XK_Insert:
        return GDF_KEYCODE_INSERT;
    case XK_Delete:
        return GDF_KEYCODE_DELETE;
    case XK_0:
        return GDF_KEYCODE_0;
    case XK_1:
        return GDF_KEYCODE_1;
    case XK_2:
        return GDF_KEYCODE_2;
    case XK_3:
        return GDF_KEYCODE_3;
    case XK_4:
        return GDF_KEYCODE_4;
    case XK_5:
        return GDF_KEYCODE_5;
    case XK_6:
        return GDF_KEYCODE_6;
    case XK_7:
        return GDF_KEYCODE_7;
    case XK_8:
        return GDF_KEYCODE_8;
    case XK_9:
        return GDF_KEYCODE_9;
    case XK_a:
    case XK_A:
        return GDF_KEYCODE_A;
    case XK_b:
    case XK_B:
        return GDF_KEYCODE_B;
    case XK_c:
    case XK_C:
        return GDF_KEYCODE_C;
    case XK_d:
    case XK_D:
        return GDF_KEYCODE_D;
    case XK_e:
    case XK_E:
        return GDF_KEYCODE_E;
    case XK_f:
    case XK_F:
        return GDF_KEYCODE_F;
    case XK_g:
    case XK_G:
        return GDF_KEYCODE_G;
    case XK_h:
    case XK_H:
        return GDF_KEYCODE_H;
    case XK_i:
    case XK_I:
        return GDF_KEYCODE_I;
    case XK_j:
    case XK_J:
        return GDF_KEYCODE_J;
    case XK_k:
    case XK_K:
        return GDF_KEYCODE_K;
    case XK_l:
    case XK_L:
        return GDF_KEYCODE_L;
    case XK_m:
    case XK_M:
        return GDF_KEYCODE_M;
    case XK_n:
    case XK_N:
        return GDF_KEYCODE_N;
    case XK_o:
    case XK_O:
        return GDF_KEYCODE_O;
    case XK_p:
    case XK_P:
        return GDF_KEYCODE_P;
    case XK_q:
    case XK_Q:
        return GDF_KEYCODE_Q;
    case XK_r:
    case XK_R:
        return GDF_KEYCODE_R;
    case XK_s:
    case XK_S:
        return GDF_KEYCODE_S;
    case XK_t:
    case XK_T:
        return GDF_KEYCODE_T;
    case XK_u:
    case XK_U:
        return GDF_KEYCODE_U;
    case XK_v:
    case XK_V:
        return GDF_KEYCODE_V;
    case XK_w:
    case XK_W:
        return GDF_KEYCODE_W;
    case XK_x:
    case XK_X:
        return GDF_KEYCODE_X;
    case XK_y:
    case XK_Y:
        return GDF_KEYCODE_Y;
    case XK_z:
    case XK_Z:
        return GDF_KEYCODE_Z;
    case XK_F1:
        return GDF_KEYCODE_F1;
    case XK_F2:
        return GDF_KEYCODE_F2;
    case XK_F3:
        return GDF_KEYCODE_F3;
    case XK_F4:
        return GDF_KEYCODE_F4;
    case XK_F5:
        return GDF_KEYCODE_F5;
    case XK_F6:
        return GDF_KEYCODE_F6;
    case XK_F7:
        return GDF_KEYCODE_F7;
    case XK_F8:
        return GDF_KEYCODE_F8;
    case XK_F9:
        return GDF_KEYCODE_F9;
    case XK_F10:
        return GDF_KEYCODE_F10;
    case XK_F11:
        return GDF_KEYCODE_F11;
    case XK_F12:
        return GDF_KEYCODE_F12;
    case XK_Shift_L:
        return GDF_KEYCODE_LSHIFT;
    case XK_Shift_R:
        return GDF_KEYCODE_RSHIFT;
    case XK_Control_L:
        return GDF_KEYCODE_LCONTROL;
    case XK_Control_R:
        return GDF_KEYCODE_RCONTROL;
    case XK_Alt_L:
        return GDF_KEYCODE_LALT;
    case XK_Alt_R:
        return GDF_KEYCODE_RALT;
    case XK_semicolon:
        return GDF_KEYCODE_SEMICOLON;
    default:
        LOG_WARN("Unrecognized key pressed.");
        return 0;
    }
}

static Cursor create_hidden_cursor()
{
    static char data[1] = { 0 };
    Pixmap      blank   = XCreateBitmapFromData(x_display, x_root_window, data, 1, 1);
    if (blank == None)
        return None;

    XColor color  = { 0 };
    Cursor cursor = XCreatePixmapCursor(x_display, blank, blank, &color, &color, 0, 0);
    XFreePixmap(x_display, blank);
    return cursor;
}

static void handle_raw_input_event(XEvent* event)
{
    XGenericEventCookie* cookie = &event->xcookie;

    if (cookie->type != GenericEvent || cookie->extension != xi_opcode)
        return;

    if (!XGetEventData(x_display, cookie))
        return;

    if (cookie->evtype == XI_RawMotion)
    {
        XIRawEvent* raw_event = (XIRawEvent*)cookie->data;

        double delta_x = 0.0, delta_y = 0.0;
        if (XIMaskIsSet(raw_event->valuators.mask, 0))
            delta_x = raw_event->raw_values[0];

        if (XIMaskIsSet(raw_event->valuators.mask, 1))
            delta_y = raw_event->raw_values[1];

        if (delta_x != 0.0 || delta_y != 0.0)
        {
            __input_process_raw_mouse_move((i32)delta_x, (i32)delta_y);
        }
    }

    XFreeEventData(x_display, cookie);
}

static void process_x_event(XEvent* event)
{
    GDF_Window* window_p = GDF_HashmapGet(windows, &event->xany.window);
    if (!window_p && event->type != GenericEvent)
        return;

    GDF_Window window = window_p ? *window_p : NULL;

    switch (event->type)
    {
    case ClientMessage:
        {
            if (event->xclient.data.l[0] == wm_delete_window)
            {
                GDF_EventContext ctx = {};
                GDF_EventFire(GDF_EVENT_INTERNAL_APP_QUIT, window, ctx);
            }
            break;
        }
    case ConfigureNotify:
        {
            if (window &&
                (event->xconfigure.width != window->client_w ||
                    event->xconfigure.height != window->client_h))
            {
                window->client_w = event->xconfigure.width;
                window->client_h = event->xconfigure.height;

                GDF_EventContext ctx;
                ctx.data.u16[0] = window->client_w;
                ctx.data.u16[1] = window->client_h;
                GDF_EventFire(GDF_EVENT_INTERNAL_WINDOW_RESIZE, window, ctx);
            }

            if (window && (event->xconfigure.x != window->x || event->xconfigure.y != window->y))
            {
                window->x = event->xconfigure.x;
                window->y = event->xconfigure.y;

                GDF_EventContext ctx;
                ctx.data.i16[0] = window->x;
                ctx.data.i16[1] = window->y;
                GDF_EventFire(GDF_EVENT_INTERNAL_WINDOW_MOVE, window, ctx);
            }
            break;
        }
    case FocusIn:
        {
            if (window)
            {
                GDF_EventContext ctx;
                ctx.data.b = GDF_TRUE;
                GDF_EventFire(GDF_EVENT_INTERNAL_WINDOW_FOCUS_CHANGE, window, ctx);
            }
            break;
        }
    case FocusOut:
        {
            if (window)
            {
                GDF_EventContext ctx;
                ctx.data.b = GDF_FALSE;
                GDF_EventFire(GDF_EVENT_INTERNAL_WINDOW_FOCUS_CHANGE, window, ctx);
            }
            break;
        }
    case KeyPress:
    case KeyRelease:
        {
            GDF_BOOL    pressed = (event->type == KeyPress);
            KeySym      keysym  = XkbKeycodeToKeysym(x_display, event->xkey.keycode, 0, 0);
            GDF_KEYCODE key     = translate_key(keysym);

            if (key != 0)
            {
                __input_process_key(key, pressed);
            }
            break;
        }
    case MotionNotify:
        {
            __input_process_mouse_move(event->xmotion.x, event->xmotion.y);
            break;
        }
    case ButtonPress:
    case ButtonRelease:
        {
            GDF_BOOL pressed = (event->type == ButtonPress);

            if (event->xbutton.button == Button4 || event->xbutton.button == Button5)
            {
                // scroll wheel
                if (pressed)
                {
                    i32 delta = (event->xbutton.button == Button4) ? 1 : -1;
                    __input_process_mouse_wheel(delta);
                }
            }
            else
            {
                // mouse buttons
                GDF_MBUTTON mouse_button = GDF_MBUTTON_MAX;
                switch (event->xbutton.button)
                {
                case Button1:
                    mouse_button = GDF_MBUTTON_LEFT;
                    break;
                case Button2:
                    mouse_button = GDF_MBUTTON_MIDDLE;
                    break;
                case Button3:
                    mouse_button = GDF_MBUTTON_RIGHT;
                    break;
                }

                if (mouse_button != GDF_MBUTTON_MAX)
                {
                    __input_process_button(mouse_button, pressed);
                }
            }
            break;
        }
    case GenericEvent:
        {
            handle_raw_input_event(event);
            break;
        }
    }
}

GDF_BOOL gdfe_windowing_init()
{
    x_display = XOpenDisplay(NULL);
    if (!x_display)
    {
        LOG_FATAL("Failed to open X display");
        return GDF_FALSE;
    }

    x_screen      = DefaultScreen(x_display);
    x_root_window = RootWindow(x_display, x_screen);

    // set up WM atoms
    wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    wm_protocols     = XInternAtom(x_display, "WM_PROTOCOLS", False);

    // init XInput2 (raw input)
    int event, error;
    if (XQueryExtension(x_display, "XInputExtension", &xi_opcode, &event, &error))
    {
        int major = 2, minor = 0;
        if (XIQueryVersion(x_display, &major, &minor) == Success)
        {
            XIEventMask   evmasks[1];
            unsigned char mask[(XI_LASTEVENT + 7) / 8];
            memset(mask, 0, sizeof(mask));
            XISetMask(mask, XI_RawMotion);

            evmasks[0].deviceid = XIAllMasterDevices;
            evmasks[0].mask_len = sizeof(mask);
            evmasks[0].mask     = mask;

            XISelectEvents(x_display, x_root_window, evmasks, 1);
            LOG_DEBUG("XInput2 raw input initialized");
        }
    }

    windows = GDF_HashmapCreate(Window, GDF_Window, GDF_FALSE);
    return GDF_TRUE;
}

void gdfe_windowing_shutdown()
{
    if (x_display)
    {
        XCloseDisplay(x_display);
        x_display = NULL;
    }
}

GDF_Window GDF_WinCreate(i16 x_, i16 y_, i16 w, i16 h, const char* title)
{
    if (!x_display)
        return NULL;

    GDF_Window window      = GDF_Malloc(sizeof(GDF_Window_T), GDF_MEMTAG_APPLICATION);
    window->id             = current_window_id++;
    window->client_w       = w;
    window->client_h       = h;
    window->cursor_grabbed = GDF_FALSE;

    // TODO! handle window centering
    i16 x     = (x_ == GDF_WIN_CENTERED) ? 300 : x_;
    i16 y     = (y_ == GDF_WIN_CENTERED) ? 300 : y_;
    window->x = x;
    window->y = y;

    XSetWindowAttributes attrs = { 0 };
    attrs.background_pixel     = BlackPixel(x_display, x_screen);
    attrs.event_mask           = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
        ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;

    window->x_window =
        XCreateWindow(x_display, x_root_window, x, y, w, h, 0, DefaultDepth(x_display, x_screen),
            InputOutput, DefaultVisual(x_display, x_screen), CWBackPixel | CWEventMask, &attrs);

    if (!window->x_window)
    {
        LOG_FATAL("Failed to create X window");
        GDF_Free(window);
        return NULL;
    }

    // sets window title
    XStoreName(x_display, window->x_window, title);

    XSetWMProtocols(x_display, window->x_window, &wm_delete_window, 1);

    // create invis cursor to hide it
    window->hidden_cursor = create_hidden_cursor();

    XMapWindow(x_display, window->x_window);
    XFlush(x_display);

    GDF_ASSERT(GDF_HashmapInsert(windows, &window->x_window, &window, NULL));

    return window;
}

GDF_BOOL GDF_WinDestroy(GDF_Window window)
{
    GDF_ASSERT(GDF_HashmapRemove(windows, &window->x_window, NULL));

    if (window->hidden_cursor != None)
    {
        XFreeCursor(x_display, window->hidden_cursor);
    }

    XDestroyWindow(x_display, window->x_window);
    XFlush(x_display);
    GDF_Free(window);
    return GDF_TRUE;
}

GDF_BOOL GDF_WinSetPos(GDF_Window window, i16 dest_x, i16 dest_y)
{
    XMoveWindow(x_display, window->x_window, dest_x, dest_y);
    XFlush(x_display);
    window->x = dest_x;
    window->y = dest_y;
    return GDF_TRUE;
}

void GDF_WinGetPos(GDF_Window window, i16* x, i16* y)
{
    if (!window)
        return;

    *x = window->x;
    *y = window->y;
}

void GDF_WinGetSize(GDF_Window window, u16* w, u16* h)
{
    if (!window)
        return;
    *w = window->client_w;
    *h = window->client_h;
}

void GDF_WinShowCursor(GDF_Window window)
{
    if (!window || !x_display)
        return;

    XUndefineCursor(x_display, window->x_window);
    XFlush(x_display);
}

void GDF_WinHideCursor(GDF_Window window)
{
    if (window->hidden_cursor != None)
    {
        XDefineCursor(x_display, window->x_window, window->hidden_cursor);
    }
    XFlush(x_display);
}

void GDF_WinGrabCursor(GDF_Window window, GDF_Rect rect)
{
    if (!window || !x_display)
        return;

    GDF_WinHideCursor(window);

    XGrabPointer(x_display, window->x_window, True,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync,
        window->x_window, None, CurrentTime);

    window->cursor_grabbed = GDF_TRUE;
    XFlush(x_display);
}

void GDF_WinReleaseCursor(GDF_Window window)
{
    XUngrabPointer(x_display, CurrentTime);
    XUndefineCursor(x_display, window->x_window);
    window->cursor_grabbed = GDF_FALSE;
    XFlush(x_display);
}

GDF_BOOL pump_messages()
{
    while (XPending(x_display))
    {
        XEvent event;
        XNextEvent(x_display, &event);
        process_x_event(&event);
    }

    return GDF_TRUE;
}

void GDF_GetDisplayInfo(GDF_DisplayInfo* display_info)
{
    display_info->screen_width  = DisplayWidth(x_display, x_screen);
    display_info->screen_height = DisplayHeight(x_display, x_screen);
}

void GDF_VK_GetRequiredExtensionNames(const char*** names_list)
{
    GDF_ListPush(*names_list, &"VK_KHR_xlib_surface");
}

    #include <i_render/vk_os.h>
GDF_BOOL GDF_VK_CreateSurface(GDF_Window window, GDF_VkRenderContext* context)
{
    VkXlibSurfaceCreateInfoKHR create_info = { VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR };
    create_info.dpy                        = x_display;
    create_info.window                     = window->x_window;

    VkResult result = vkCreateXlibSurfaceKHR(
        context->instance, &create_info, context->device.allocator, &context->surface);

    if (result != VK_SUCCESS)
    {
        LOG_ERR("Vulkan surface creation failed.");
        return GDF_FALSE;
    }

    LOG_DEBUG("Created Vulkan surface.");
    return GDF_TRUE;
}

#endif
