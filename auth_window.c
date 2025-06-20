#include "window.h"
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 64
#define INPUT_FIELD_HEIGHT 25
#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 30

typedef enum {
    AUTH_STATE_LOGIN,
    AUTH_STATE_REGISTER,
    AUTH_STATE_SUCCESS,
    AUTH_STATE_ERROR
} AuthState;

typedef enum {
    INPUT_USERNAME = 0,
    INPUT_PASSWORD = 1,
    INPUT_COUNT = 2
} InputField;

typedef struct {
    Window* window;
    AuthState state;

    char username[MAX_INPUT_LENGTH];
    char password[MAX_INPUT_LENGTH];
    int active_field;
    int cursor_pos[INPUT_COUNT];
    
    int username_x, username_y;
    int password_x, password_y;
    int login_btn_x, login_btn_y;
    int register_btn_x, register_btn_y;
    
    int show_password;
    char status_message[256];
    int message_timer;
    
    unsigned int bg_color;
    unsigned int field_color;
    unsigned int button_color;
    unsigned int text_color;
} AuthWindow;

void auth_window_draw(AuthWindow* auth_win);
void auth_window_handle_key(AuthWindow* auth_win, int key_code);
void auth_window_handle_click(AuthWindow* auth_win, int x, int y);
int auth_window_validate_input(AuthWindow* auth_win);
void auth_window_set_status(AuthWindow* auth_win, const char* message);

AuthWindow* auth_window_create(const char* title) {
    AuthWindow* auth_win = malloc(sizeof(AuthWindow));
    if (!auth_win) return NULL;
    
    WindowConfig config = window_config_default();
    config.title = title;
    config.width = 400;
    config.height = 300;
    config.x = 300;
    config.y = 200;
    
    auth_win->window = window_create(config);
    if (!auth_win->window) {
        free(auth_win);
        return NULL;
    }
    
    auth_win->state = AUTH_STATE_LOGIN;
    memset(auth_win->username, 0, sizeof(auth_win->username));
    memset(auth_win->password, 0, sizeof(auth_win->password));
    auth_win->active_field = INPUT_USERNAME;
    auth_win->cursor_pos[INPUT_USERNAME] = 0;
    auth_win->cursor_pos[INPUT_PASSWORD] = 0;
    auth_win->show_password = 0;
    auth_win->message_timer = 0;
    strcpy(auth_win->status_message, "Enter your credentials");
    
    auth_win->username_x = 50;
    auth_win->username_y = 80;
    auth_win->password_x = 50;
    auth_win->password_y = 120;
    auth_win->login_btn_x = 50;
    auth_win->login_btn_y = 180;
    auth_win->register_btn_x = 170;
    auth_win->register_btn_y = 180;
    
    auth_win->bg_color = 0xF0F0F0;
    auth_win->field_color = 0xFFFFFF;
    auth_win->button_color = 0x0078D4;
    auth_win->text_color = 0x000000;
    
    return auth_win;
}

int auth_window_show(AuthWindow* auth_win) {
    if (!auth_win || !auth_win->window) return 0;
    return window_show(auth_win->window);
}

int auth_window_run(AuthWindow* auth_win) {
    if (!auth_win) return 0;
    
    WindowEvent event;
    int needs_redraw = 1;
    
    printf("Authentication window started\n");
    printf("Controls:\n");
    printf("- TAB: Switch between fields\n");
    printf("- ENTER: Login/Register\n");
    printf("- F1: Toggle password visibility\n");
    printf("- ESC: Exit\n\n");
    
    while (window_poll_events(auth_win->window, &event)) {
        switch (event.type) {
            case WINDOW_EVENT_CLOSE:
                printf("Authentication window closed\n");
                return 0;
                
            case WINDOW_EVENT_PAINT:
                needs_redraw = 1;
                break;
                
            case WINDOW_EVENT_KEY_PRESS:
                auth_window_handle_key(auth_win, event.data.key.key_code);
                needs_redraw = 1;
                
                if (auth_win->state == AUTH_STATE_SUCCESS) {
                    printf("Authentication successful!\n");
                    printf("Username: %s\n", auth_win->username);
                    return 1;
                }
                break;
                
            case WINDOW_EVENT_MOUSE_CLICK:
                auth_window_handle_click(auth_win, event.data.mouse.x, event.data.mouse.y);
                needs_redraw = 1;
                break;
                
            case WINDOW_EVENT_NONE:
                break;
        }
        
        if (auth_win->message_timer > 0) {
            auth_win->message_timer--;
            if (auth_win->message_timer == 0) {
                needs_redraw = 1;
            }
        }
        
        if (needs_redraw) {
            auth_window_draw(auth_win);
            needs_redraw = 0;
        }
        
#ifdef PLATFORM_WINDOWS
        Sleep(16);
#elif defined(PLATFORM_X11)
        usleep(16000);
#endif
    }
    
    return 0;
}

void auth_window_draw(AuthWindow* auth_win) {
    if (!auth_win) return;
    
    window_clear(auth_win->window);
    
    const char* title = (auth_win->state == AUTH_STATE_REGISTER) ? 
        "Register New Account" : "Login";
    window_draw_text(auth_win->window, 50, 30, title);
    
    window_draw_text(auth_win->window, auth_win->username_x, auth_win->username_y - 20, "Username:");
    
    unsigned int username_bg = (auth_win->active_field == INPUT_USERNAME) ? 
        0xE6F3FF : auth_win->field_color;
    window_draw_rectangle(auth_win->window, auth_win->username_x, auth_win->username_y, 
        200, INPUT_FIELD_HEIGHT, username_bg);
    
    window_draw_text(auth_win->window, auth_win->username_x + 5, auth_win->username_y + 5, 
        auth_win->username);
    
    if (auth_win->active_field == INPUT_USERNAME) {
        int cursor_x = auth_win->username_x + 5 + (auth_win->cursor_pos[INPUT_USERNAME] * 8);
        window_draw_rectangle(auth_win->window, cursor_x, auth_win->username_y + 3, 
            1, INPUT_FIELD_HEIGHT - 6, 0x000000);
    }
    
    window_draw_text(auth_win->window, auth_win->password_x, auth_win->password_y - 20, "Password:");
    
    unsigned int password_bg = (auth_win->active_field == INPUT_PASSWORD) ?
        0xE6F3FF : auth_win->field_color;
    window_draw_rectangle(auth_win->window, auth_win->password_x, auth_win->password_y, 
        200, INPUT_FIELD_HEIGHT, password_bg);
    
    if (auth_win->show_password) {
        window_draw_text(auth_win->window, auth_win->password_x + 5, auth_win->password_y + 5, 
            auth_win->password);
    } else {
        char masked[MAX_INPUT_LENGTH];
        int len = strlen(auth_win->password);
        for (int i = 0; i < len && i < MAX_INPUT_LENGTH - 1; i++) {
            masked[i] = '*';
        }
        masked[len] = '\0';
        window_draw_text(auth_win->window, auth_win->password_x + 5, auth_win->password_y + 5, 
            masked);
    }
    
    if (auth_win->active_field == INPUT_PASSWORD) {
        int cursor_x = auth_win->password_x + 5 + (auth_win->cursor_pos[INPUT_PASSWORD] * 8);
        window_draw_rectangle(auth_win->window, cursor_x, auth_win->password_y + 3, 
            1, INPUT_FIELD_HEIGHT - 6, 0x000000);
    }
    
    window_draw_rectangle(auth_win->window, auth_win->login_btn_x, auth_win->login_btn_y, 
        BUTTON_WIDTH, BUTTON_HEIGHT, auth_win->button_color);
    window_draw_text(auth_win->window, auth_win->login_btn_x + 35, auth_win->login_btn_y + 8, 
        "Login");

    unsigned int register_color = (auth_win->state == AUTH_STATE_REGISTER) ? 
        0x00AA00 : 0x666666;
    window_draw_rectangle(auth_win->window, auth_win->register_btn_x, auth_win->register_btn_y, 
        BUTTON_WIDTH, BUTTON_HEIGHT, register_color);
    const char* register_text = (auth_win->state == AUTH_STATE_REGISTER) ? 
        "Create" : "Register";
    window_draw_text(auth_win->window, auth_win->register_btn_x + 25, auth_win->register_btn_y + 8, 
        register_text);
    
    if (strlen(auth_win->status_message) > 0) {
        unsigned int msg_color = (auth_win->state == AUTH_STATE_ERROR) ? 0xFF0000 : 0x006600;
        window_draw_text(auth_win->window, 50, 230, auth_win->status_message);
    }
    
    window_draw_text(auth_win->window, 50, 260, "TAB: Switch | F1: Show/Hide Password | ESC: Exit");
    
    window_present(auth_win->window);
}

void auth_window_handle_key(AuthWindow* auth_win, int key_code) {
    if (!auth_win) return;
    
    char* current_field = (auth_win->active_field == INPUT_USERNAME) ? 
        auth_win->username : auth_win->password;
    int* cursor_pos = &auth_win->cursor_pos[auth_win->active_field];
    int field_len = strlen(current_field);
    
    switch (key_code) {
#ifdef PLATFORM_WINDOWS
        case VK_ESCAPE:
#elif defined(PLATFORM_X11)
        case 'q': case 'Q':
#endif
            auth_window_set_status(auth_win, "Exiting...");
            break;
            
#ifdef PLATFORM_WINDOWS
        case VK_TAB:
#elif defined(PLATFORM_X11) 
        case '\t':
#endif
            auth_win->active_field = (auth_win->active_field + 1) % INPUT_COUNT;
            break;
            
#ifdef PLATFORM_WINDOWS
        case VK_RETURN:
#elif defined(PLATFORM_X11)
        case '\n': case '\r':
#endif
            if (auth_window_validate_input(auth_win)) {
                if (auth_win->state == AUTH_STATE_REGISTER) {
                    auth_window_set_status(auth_win, "Account created successfully!");
                    auth_win->state = AUTH_STATE_LOGIN;
                } else {
                    if (strlen(auth_win->username) > 0 && strlen(auth_win->password) > 0) {
                        auth_win->state = AUTH_STATE_SUCCESS;
                        auth_window_set_status(auth_win, "Login successful!");
                    } else {
                        auth_win->state = AUTH_STATE_ERROR;
                        auth_window_set_status(auth_win, "Invalid credentials!");
                    }
                }
            }
            break;
            
#ifdef PLATFORM_WINDOWS
        case VK_F1:
#elif defined(PLATFORM_X11)
        case '`':
#endif
            auth_win->show_password = !auth_win->show_password;
            auth_window_set_status(auth_win, auth_win->show_password ? 
                "Password visible" : "Password hidden");
            break;
            
#ifdef PLATFORM_WINDOWS
        case VK_BACK:
#elif defined(PLATFORM_X11)
        case '\b': case 127:
#endif
            if (field_len > 0 && *cursor_pos > 0) {
                memmove(&current_field[*cursor_pos - 1], &current_field[*cursor_pos], 
                    field_len - *cursor_pos + 1);
                (*cursor_pos)--;
            }
            break;
            
#ifdef PLATFORM_WINDOWS
        case VK_LEFT:
#elif defined(PLATFORM_X11)
        case 'h':
#endif
            if (*cursor_pos > 0) (*cursor_pos)--;
            break;
            
#ifdef PLATFORM_WINDOWS
        case VK_RIGHT:
#elif defined(PLATFORM_X11)
        case 'l':
#endif
            if (*cursor_pos < field_len) (*cursor_pos)++;
            break;
            
        default:
            if (field_len < MAX_INPUT_LENGTH - 1) {
#ifdef PLATFORM_WINDOWS
                if (key_code >= 32 && key_code <= 126) {
                    char ch = (char)key_code;
#elif defined(PLATFORM_X11)
                if (key_code >= 32 && key_code <= 126) {
                    char ch = (char)key_code;
#endif
                    memmove(&current_field[*cursor_pos + 1], &current_field[*cursor_pos], 
                        field_len - *cursor_pos + 1);
                    current_field[*cursor_pos] = ch;
                    (*cursor_pos)++;
#ifdef PLATFORM_WINDOWS
                }
#elif defined(PLATFORM_X11)
                }
#endif
            }
            break;
    }
}

void auth_window_handle_click(AuthWindow* auth_win, int x, int y) {
    if (!auth_win) return;
    
    if (x >= auth_win->username_x && x <= auth_win->username_x + 200 &&
        y >= auth_win->username_y && y <= auth_win->username_y + INPUT_FIELD_HEIGHT) {
        auth_win->active_field = INPUT_USERNAME;
        int char_pos = (x - auth_win->username_x - 5) / 8;
        int max_pos = strlen(auth_win->username);
        auth_win->cursor_pos[INPUT_USERNAME] = (char_pos < 0) ? 0 : 
            (char_pos > max_pos) ? max_pos : char_pos;
    }
    
    else if (x >= auth_win->password_x && x <= auth_win->password_x + 200 &&
             y >= auth_win->password_y && y <= auth_win->password_y + INPUT_FIELD_HEIGHT) {
        auth_win->active_field = INPUT_PASSWORD;
        int char_pos = (x - auth_win->password_x - 5) / 8;
        int max_pos = strlen(auth_win->password);
        auth_win->cursor_pos[INPUT_PASSWORD] = (char_pos < 0) ? 0 : 
            (char_pos > max_pos) ? max_pos : char_pos;
    }
    
    else if (x >= auth_win->login_btn_x && x <= auth_win->login_btn_x + BUTTON_WIDTH &&
             y >= auth_win->login_btn_y && y <= auth_win->login_btn_y + BUTTON_HEIGHT) {
        auth_win->state = AUTH_STATE_LOGIN;
        auth_window_handle_key(auth_win, 
#ifdef PLATFORM_WINDOWS
            VK_RETURN
#elif defined(PLATFORM_X11)
            '\n'
#endif
        );
    }
    
    else if (x >= auth_win->register_btn_x && x <= auth_win->register_btn_x + BUTTON_WIDTH &&
             y >= auth_win->register_btn_y && y <= auth_win->register_btn_y + BUTTON_HEIGHT) {
        if (auth_win->state == AUTH_STATE_REGISTER) {
            auth_window_handle_key(auth_win,
#ifdef PLATFORM_WINDOWS
                VK_RETURN
#elif defined(PLATFORM_X11)
                '\n'
#endif
            );
        } else {
            auth_win->state = AUTH_STATE_REGISTER;
            auth_window_set_status(auth_win, "Fill in details to create new account");
        }
    }
}

int auth_window_validate_input(AuthWindow* auth_win) {
    if (!auth_win) return 0;
    
    if (strlen(auth_win->username) == 0) {
        auth_win->state = AUTH_STATE_ERROR;
        auth_window_set_status(auth_win, "Username cannot be empty!");
        return 0;
    }
    
    if (strlen(auth_win->password) < 3) {
        auth_win->state = AUTH_STATE_ERROR;
        auth_window_set_status(auth_win, "Password must be at least 3 characters!");
        return 0;
    }
    
    return 1;
}

void auth_window_set_status(AuthWindow* auth_win, const char* message) {
    if (!auth_win || !message) return;
    
    strncpy(auth_win->status_message, message, sizeof(auth_win->status_message) - 1);
    auth_win->status_message[sizeof(auth_win->status_message) - 1] = '\0';
    auth_win->message_timer = 180;
    
    printf("Status: %s\n", message);
}

int auth_window_get_credentials(AuthWindow* auth_win, char* username, char* password, 
                               int username_size, int password_size) {
    if (!auth_win || !username || !password) return 0;
    
    if (auth_win->state == AUTH_STATE_SUCCESS) {
        strncpy(username, auth_win->username, username_size - 1);
        username[username_size - 1] = '\0';
        strncpy(password, auth_win->password, password_size - 1);
        password[password_size - 1] = '\0';
        return 1;
    }
    
    return 0;
}

void auth_window_destroy(AuthWindow* auth_win) {
    if (!auth_win) return;
    
    if (auth_win->window) {
        window_destroy(auth_win->window);
    }
    
    memset(auth_win->username, 0, sizeof(auth_win->username));
    memset(auth_win->password, 0, sizeof(auth_win->password));
    
    free(auth_win);
}