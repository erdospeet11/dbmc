#ifndef WINDOW_H
#define WINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #include <windows.h>
#elif defined(__linux__) || defined(__unix__)
    #define PLATFORM_X11
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <unistd.h>
#endif

typedef struct {
    int x, y;           
    int width, height;  
    const char* title;
} WindowConfig;

typedef struct {
    WindowConfig config;
    int is_running;
    
#ifdef PLATFORM_WINDOWS
    HWND hwnd;
    HINSTANCE hInstance;
    HDC hdc;
#elif defined(PLATFORM_X11)
    Display* display;
    Window window;
    GC gc;
    int screen;
#endif
} Window;

typedef enum {
    WINDOW_EVENT_NONE = 0,
    WINDOW_EVENT_CLOSE,
    WINDOW_EVENT_PAINT,
    WINDOW_EVENT_KEY_PRESS,
    WINDOW_EVENT_MOUSE_CLICK
} WindowEventType;

typedef struct {
    WindowEventType type;
    union {
        struct {
            int key_code;
        } key;
        struct {
            int x, y;
            int button;
        } mouse;
    } data;
} WindowEvent;

Window* window_create(WindowConfig config);
int window_show(Window* window);
int window_poll_events(Window* window, WindowEvent* event);
void window_clear(Window* window);
void window_draw_text(Window* window, int x, int y, const char* text);
void window_draw_rectangle(Window* window, int x, int y, int width, int height, unsigned int color);
void window_present(Window* window);
void window_destroy(Window* window);

WindowConfig window_config_default(void);

#endif // WINDOW_H 