#include "window.h"

// =============================================================================
// WINDOWS API IMPLEMENTATION
// =============================================================================
#ifdef PLATFORM_WINDOWS

static Window* g_current_window = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (g_current_window == NULL) {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    switch (uMsg) {
        case WM_DESTROY:
            g_current_window->is_running = 0;
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                g_current_window->is_running = 0;
                PostQuitMessage(0);
            }
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window* window_create(WindowConfig config) {
    Window* window = malloc(sizeof(Window));
    if (!window) return NULL;
    
    window->config = config;
    window->is_running = 1;
    window->hInstance = GetModuleHandle(NULL);
    
    const char CLASS_NAME[] = "CrossPlatformWindow";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = window->hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    static int class_registered = 0;
    if (!class_registered) {
        if (!RegisterClass(&wc)) {
            free(window);
            return NULL;
        }
        class_registered = 1;
    }
    
    window->hwnd = CreateWindowEx(
        0, CLASS_NAME, config.title, WS_OVERLAPPEDWINDOW,
        config.x, config.y, config.width, config.height,
        NULL, NULL, window->hInstance, NULL
    );
    
    if (window->hwnd == NULL) {
        free(window);
        return NULL;
    }
    
    window->hdc = GetDC(window->hwnd);
    g_current_window = window;
    
    return window;
}

int window_show(Window* window) {
    if (!window) return 0;
    ShowWindow(window->hwnd, SW_SHOW);
    UpdateWindow(window->hwnd);
    return 1;
}

int window_poll_events(Window* window, WindowEvent* event) {
    if (!window) return 0;
    
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        switch (msg.message) {
            case WM_QUIT:
                event->type = WINDOW_EVENT_CLOSE;
                return 1;
                    case WM_KEYDOWN:
            if (msg.wParam == VK_ESCAPE || msg.wParam == VK_TAB || msg.wParam == VK_RETURN ||
                msg.wParam == VK_BACK || msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT ||
                msg.wParam == VK_F1 || msg.wParam == VK_UP || msg.wParam == VK_DOWN) {
                event->type = WINDOW_EVENT_KEY_PRESS;
                event->data.key.key_code = msg.wParam;
                return 1;
            }
            break;
        case WM_CHAR:
            if (msg.wParam >= 32 && msg.wParam <= 126) {
                event->type = WINDOW_EVENT_KEY_PRESS;
                event->data.key.key_code = msg.wParam;
                return 1;
            }
            break;
            case WM_LBUTTONDOWN:
                event->type = WINDOW_EVENT_MOUSE_CLICK;
                event->data.mouse.x = LOWORD(msg.lParam);
                event->data.mouse.y = HIWORD(msg.lParam);
                event->data.mouse.button = 1;
                return 1;
        }
    }
    
    event->type = WINDOW_EVENT_NONE;
    return window->is_running;
}

void window_clear(Window* window) {
    if (!window) return;
    RECT rect;
    GetClientRect(window->hwnd, &rect);
    FillRect(window->hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
}

void window_draw_text(Window* window, int x, int y, const char* text) {
    if (!window) return;
    SetTextColor(window->hdc, RGB(0, 0, 0));
    SetBkMode(window->hdc, TRANSPARENT);
    TextOut(window->hdc, x, y, text, strlen(text));
}

void window_draw_rectangle(Window* window, int x, int y, int width, int height, unsigned int color) {
    if (!window) return;
    HBRUSH hBrush = CreateSolidBrush(color);
    RECT rect = {x, y, x + width, y + height};
    FillRect(window->hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

void window_present(Window* window) {
    if (!window) return;
}

void window_destroy(Window* window) {
    if (!window) return;
    
    if (window->hdc) ReleaseDC(window->hwnd, window->hdc);
    if (window->hwnd) DestroyWindow(window->hwnd);
    
    if (g_current_window == window) {
        g_current_window = NULL;
    }
    
    free(window);
}

#endif // PLATFORM_WINDOWS

// =============================================================================
// X11 IMPLEMENTATION
// =============================================================================
#ifdef PLATFORM_X11

Window* window_create(WindowConfig config) {
    Window* window = malloc(sizeof(Window));
    if (!window) return NULL;
    
    window->config = config;
    window->is_running = 1;
    
    window->display = XOpenDisplay(NULL);
    if (window->display == NULL) {
        free(window);
        return NULL;
    }
    
    window->screen = DefaultScreen(window->display);
    
    window->window = XCreateSimpleWindow(
        window->display,
        RootWindow(window->display, window->screen),
        config.x, config.y, config.width, config.height,
        1,
        BlackPixel(window->display, window->screen),
        WhitePixel(window->display, window->screen)
    );
    
    XStoreName(window->display, window->window, config.title);
    
    XSelectInput(window->display, window->window, 
        ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    
    window->gc = XCreateGC(window->display, window->window, 0, NULL);
    XSetForeground(window->display, window->gc, BlackPixel(window->display, window->screen));
    
    return window;
}

int window_show(Window* window) {
    if (!window) return 0;
    XMapWindow(window->display, window->window);
    XFlush(window->display);
    return 1;
}

int window_poll_events(Window* window, WindowEvent* event) {
    if (!window) return 0;
    
    if (XPending(window->display)) {
        XEvent xevent;
        XNextEvent(window->display, &xevent);
        
        switch (xevent.type) {
            case Expose:
                event->type = WINDOW_EVENT_PAINT;
                return 1;
                
            case KeyPress: {
                char buffer[10];
                KeySym keysym;
                XLookupString(&xevent.xkey, buffer, sizeof(buffer), &keysym, NULL);
                
                if (buffer[0] == 'q' || buffer[0] == 'Q') {
                    window->is_running = 0;
                    event->type = WINDOW_EVENT_CLOSE;
                } else {
                    event->type = WINDOW_EVENT_KEY_PRESS;
                    event->data.key.key_code = buffer[0];
                }
                return 1;
            }
            
            case ButtonPress:
                event->type = WINDOW_EVENT_MOUSE_CLICK;
                event->data.mouse.x = xevent.xbutton.x;
                event->data.mouse.y = xevent.xbutton.y;
                event->data.mouse.button = xevent.xbutton.button;
                return 1;
                
            case DestroyNotify:
                window->is_running = 0;
                event->type = WINDOW_EVENT_CLOSE;
                return 1;
        }
    }
    
    event->type = WINDOW_EVENT_NONE;
    return window->is_running;
}

void window_clear(Window* window) {
    if (!window) return;
    XClearWindow(window->display, window->window);
}

void window_draw_text(Window* window, int x, int y, const char* text) {
    if (!window) return;
    XDrawString(window->display, window->window, window->gc, x, y, text, strlen(text));
}

void window_draw_rectangle(Window* window, int x, int y, int width, int height, unsigned int color) {
    if (!window) return;
    XSetForeground(window->display, window->gc, color);
    XFillRectangle(window->display, window->window, window->gc, x, y, width, height);
    XSetForeground(window->display, window->gc, BlackPixel(window->display, window->screen));
}

void window_present(Window* window) {
    if (!window) return;
    XFlush(window->display);
}

void window_destroy(Window* window) {
    if (!window) return;
    
    XFreeGC(window->display, window->gc);
    XDestroyWindow(window->display, window->window);
    XCloseDisplay(window->display);
    free(window);
}

#endif // PLATFORM_X11

// =============================================================================
// COMMON FUNCTIONS
// =============================================================================

WindowConfig window_config_default(void) {
    WindowConfig config = {
        .x = 100,
        .y = 100,
        .width = 800,
        .height = 600,
        .title = "Cross-Platform Window"
    };
    return config;
} 