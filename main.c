#include "window.h"
#include "auth_window.h"

int run_main_application(const char* username) {
    printf("=== Welcome to the Main Application ===\n");
    printf("Logged in as: %s\n", username);
    
    WindowConfig config = window_config_default();
    config.title = "Main Application - Cross-Platform Window";
    config.width = 800;
    config.height = 600;
    
    Window* window = window_create(config);
    if (!window) {
        printf("Failed to create main application window!\n");
        return 1;
    }
    
    if (!window_show(window)) {
        printf("Failed to show main application window!\n");
        window_destroy(window);
        return 1;
    }
    
    printf("Main application window created successfully!\n");
#ifdef PLATFORM_WINDOWS
    printf("Platform: Windows (WinAPI)\n");
    printf("Controls: ESC to exit, click for coordinates\n");
#elif defined(PLATFORM_X11)
    printf("Platform: Linux/Unix (X11)\n");
    printf("Controls: 'q' to quit, click for coordinates\n");
#endif
    
    WindowEvent event;
    int needs_redraw = 1;
    
    while (window_poll_events(window, &event)) {
        switch (event.type) {
            case WINDOW_EVENT_CLOSE:
                printf("Main application window close requested\n");
                goto cleanup;
                
            case WINDOW_EVENT_PAINT:
                needs_redraw = 1;
                break;
                
            case WINDOW_EVENT_KEY_PRESS:
                printf("Key pressed: %d\n", event.data.key.key_code);
#ifdef PLATFORM_WINDOWS
                if (event.data.key.key_code == VK_ESCAPE) {
                    goto cleanup;
                }
#elif defined(PLATFORM_X11)
                if (event.data.key.key_code == 'q' || event.data.key.key_code == 'Q') {
                    goto cleanup;
                }
#endif
                break;
                
            case WINDOW_EVENT_MOUSE_CLICK:
                printf("Mouse clicked at (%d, %d) - Button: %d\n", 
                    event.data.mouse.x, event.data.mouse.y, event.data.mouse.button);
                break;
                
            case WINDOW_EVENT_NONE:
                break;
        }
        
        if (needs_redraw) {
            window_clear(window);
            
            char welcome_msg[256];
            snprintf(welcome_msg, sizeof(welcome_msg), "Welcome %s! Main Application Running", username);
            window_draw_text(window, 50, 50, welcome_msg);
            
            window_draw_text(window, 50, 80, "This is the main application window");
            window_draw_text(window, 50, 110, "You have successfully authenticated!");
            
#ifdef PLATFORM_WINDOWS
            window_draw_text(window, 50, 140, "Running on Windows - Press ESC to exit");
#elif defined(PLATFORM_X11)
            window_draw_text(window, 50, 140, "Running on Linux/Unix - Press 'q' to quit");
#endif
            
            window_draw_text(window, 50, 170, "Click anywhere to see coordinates");
            
            window_draw_rectangle(window, 200, 200, 100, 50, 0xFF0000);
            
            window_draw_rectangle(window, 320, 200, 100, 50, 0x00FF00);
            
            window_draw_rectangle(window, 440, 200, 100, 50, 0x0000FF);
            
            char user_info[256];
            snprintf(user_info, sizeof(user_info), "Authenticated User: %s", username);
            window_draw_text(window, 50, 300, user_info);
            window_draw_text(window, 50, 330, "Application features unlocked!");
            
            window_present(window);
            needs_redraw = 0;
        }

#ifdef PLATFORM_WINDOWS
        Sleep(1);
#elif defined(PLATFORM_X11)
        usleep(1000);
#endif
    }
    
cleanup:
    printf("Cleaning up main application and exiting...\n");
    window_destroy(window);
    return 0;
}

int main() {
    printf("=== Application Startup - Authentication Required ===\n");
    
    AuthWindow* auth_win = auth_window_create("Login Required - Main Application");
    if (!auth_win) {
        printf("Failed to create authentication window!\n");
        return 1;
    }
    
    if (!auth_window_show(auth_win)) {
        printf("Failed to show authentication window!\n");
        auth_window_destroy(auth_win);
        return 1;
    }
    
    printf("Please login to access the main application...\n");
    
    int auth_result = auth_window_run(auth_win);
    
    if (auth_result == 1) {
        char username[MAX_INPUT_LENGTH];
        char password[MAX_INPUT_LENGTH];
        
        if (auth_window_get_credentials(auth_win, username, password, 
                                      sizeof(username), sizeof(password))) {
            printf("\n=== Authentication Successful ===\n");
            printf("Welcome, %s!\n", username);
            
            auth_window_destroy(auth_win);
            
            return run_main_application(username);
        } else {
            printf("Failed to retrieve credentials!\n");
            auth_window_destroy(auth_win);
            return 1;
        }
    } else {
        printf("\nAuthentication cancelled or failed.\n");
        printf("Access denied - exiting application.\n");
        auth_window_destroy(auth_win);
        return 1;
    }
}