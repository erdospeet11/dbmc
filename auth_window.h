#ifndef AUTH_WINDOW_H
#define AUTH_WINDOW_H

#include "window.h"

#define MAX_INPUT_LENGTH 64

typedef struct AuthWindow AuthWindow;

AuthWindow* auth_window_create(const char* title);
int auth_window_show(AuthWindow* auth_win);
int auth_window_run(AuthWindow* auth_win);
int auth_window_get_credentials(AuthWindow* auth_win, char* username, char* password, 
                               int username_size, int password_size);
void auth_window_destroy(AuthWindow* auth_win);

#endif // AUTH_WINDOW_H
