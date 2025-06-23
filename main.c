#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <commctrl.h>
    #pragma comment(lib, "comctl32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "gdi32.lib")
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/keysym.h>
    #include <unistd.h>
#endif

// UI Control IDs
#define ID_SQL_INPUT    1001
#define ID_EXECUTE_BTN  1002
#define ID_TABLES_LIST  1003
#define ID_TABLE_VIEW   1004

// Window dimensions
#define WINDOW_WIDTH    1000
#define WINDOW_HEIGHT   700

// Global variables
#ifdef _WIN32
HWND g_hWnd = NULL;
HWND g_hSqlInput = NULL;
HWND g_hExecuteBtn = NULL;
HWND g_hTablesList = NULL;
HWND g_hTableView = NULL;
HINSTANCE g_hInstance = NULL;
#elif defined(__linux__)
Display* g_display = NULL;
Window g_window;
int g_screen;
#endif

char g_sqlCommand[1024] = {0};

#ifdef _WIN32
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            //sql input field
            g_hSqlInput = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
                10, 10, 600, 100,
                hwnd, (HMENU)ID_SQL_INPUT, g_hInstance, NULL
            );

            //execute button
            g_hExecuteBtn = CreateWindow(
                "BUTTON",
                "Execute SQL",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                620, 10, 100, 30,
                hwnd, (HMENU)ID_EXECUTE_BTN, g_hInstance, NULL
            );

            //tables list
            g_hTablesList = CreateWindow(
                "LISTBOX",
                "",
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
                10, 120, 200, 500,
                hwnd, (HMENU)ID_TABLES_LIST, g_hInstance, NULL
            );

            //TODO: load the tables from disk
            //sample tables
            SendMessage(g_hTablesList, LB_ADDSTRING, 0, (LPARAM)"products");
            SendMessage(g_hTablesList, LB_ADDSTRING, 0, (LPARAM)"users");
            SendMessage(g_hTablesList, LB_ADDSTRING, 0, (LPARAM)"orders");

            //table view
            InitCommonControls();
            g_hTableView = CreateWindow(
                WC_LISTVIEW,
                "",
                WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
                220, 120, 750, 500,
                hwnd, (HMENU)ID_TABLE_VIEW, g_hInstance, NULL
            );

            ListView_SetExtendedListViewStyle(g_hTableView, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

            //add columns to table view
            LVCOLUMN lvc = {0};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH;
            lvc.cx = 100;
            lvc.pszText = "Column 1";
            ListView_InsertColumn(g_hTableView, 0, &lvc);
            
            lvc.pszText = "Column 2";
            ListView_InsertColumn(g_hTableView, 1, &lvc);
            
            lvc.pszText = "Column 3";
            ListView_InsertColumn(g_hTableView, 2, &lvc);

            //TODO: load columns data from disk
            //sample data
            LVITEM lvi = {0};
            lvi.mask = LVIF_TEXT;
            lvi.iItem = 0;
            lvi.iSubItem = 0;
            lvi.pszText = "Row 1 Col 1";
            ListView_InsertItem(g_hTableView, &lvi);
            ListView_SetItemText(g_hTableView, 0, 1, "Row 1 Col 2");
            ListView_SetItemText(g_hTableView, 0, 2, "Row 1 Col 3");

            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_EXECUTE_BTN: {
                    GetWindowText(g_hSqlInput, g_sqlCommand, sizeof(g_sqlCommand));
                    MessageBox(hwnd, g_sqlCommand, "SQL Command", MB_OK | MB_ICONINFORMATION);
                    SetWindowText(g_hSqlInput, "");
                    break;
                }
                
                case ID_TABLES_LIST: {
                    if (HIWORD(wParam) == LBN_SELCHANGE) {
                        int sel = SendMessage(g_hTablesList, LB_GETCURSEL, 0, 0);
                        if (sel != LB_ERR) {
                            char tableName[256];
                            SendMessage(g_hTablesList, LB_GETTEXT, sel, (LPARAM)tableName);
                            
                            char msg[512];
                            sprintf(msg, "Selected table: %s", tableName);
                            SetWindowText(hwnd, msg);
                        }
                    }
                    break;
                }
            }
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            
            if (g_hTableView) {
                SetWindowPos(g_hTableView, NULL, 220, 120, width - 240, height - 140, 
                           SWP_NOZORDER);
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int CreateWindowsWindow() {
    const char* className = "DBMCWindow";
    
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    g_hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        className,
        "Database Management Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, g_hInstance, NULL
    );

    if (g_hWnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);

    return 1;
}

#elif defined(__linux__)
void DrawUI() {
    XClearWindow(g_display, g_window);
    
    XDrawRectangle(g_display, g_window, DefaultGC(g_display, g_screen), 10, 10, 600, 100);
    XDrawRectangle(g_display, g_window, DefaultGC(g_display, g_screen), 620, 10, 100, 30);
    XDrawRectangle(g_display, g_window, DefaultGC(g_display, g_screen), 10, 120, 200, 500);
    XDrawRectangle(g_display, g_window, DefaultGC(g_display, g_screen), 220, 120, 750, 500);

    XDrawString(g_display, g_window, DefaultGC(g_display, g_screen), 10, 130, "Tables:", 7);
    XDrawString(g_display, g_window, DefaultGC(g_display, g_screen), 220, 130, "Table Data:", 11);
    XDrawString(g_display, g_window, DefaultGC(g_display, g_screen), 620, 25, "Execute", 7);
    
    XFlush(g_display);
}

int CreateX11Window() {
    g_display = XOpenDisplay(NULL);
    if (g_display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 0;
    }

    g_screen = DefaultScreen(g_display);
    
    g_window = XCreateSimpleWindow(
        g_display,
        RootWindow(g_display, g_screen),
        10, 10,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        1,
        BlackPixel(g_display, g_screen),
        WhitePixel(g_display, g_screen)
    );

    XSelectInput(g_display, g_window, 
                 ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);

    XMapWindow(g_display, g_window);
    XStoreName(g_display, g_window, "Database Management Client");

    return 1;
}

void HandleX11Events() {
    XEvent event;
    
    while (1) {
        XNextEvent(g_display, &event);
        
        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0) {
                    DrawUI();
                }
                break;
                
            case KeyPress: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape) {
                    return;
                }
                break;
            }
            
            case ButtonPress: {
                int x = event.xbutton.x;
                int y = event.xbutton.y;
                
                if (x >= 620 && x <= 720 && y >= 10 && y <= 40) {
                    printf("Execute button clicked\n");
                }
                else if (x >= 10 && x <= 210 && y >= 120 && y <= 620) {
                    printf("Tables list clicked\n");
                }
                break;
            }
            
            case ClientMessage:
                return;
        }
    }
}
#endif

int InitializeWindow() {
#ifdef _WIN32
    g_hInstance = GetModuleHandle(NULL);
    return CreateWindowsWindow();
#elif defined(__linux__)
    return CreateX11Window();
#else
    fprintf(stderr, "Unsupported platform\n");
    return 0;
#endif
}

void RunEventLoop() {
#ifdef _WIN32
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#elif defined(__linux__)
    HandleX11Events();
#endif
}

void Cleanup() {
#ifdef _WIN32
#elif defined(__linux__)
    if (g_display) {
        XDestroyWindow(g_display, g_window);
        XCloseDisplay(g_display);
    }
#endif
}

int main() {
    printf("Database Management Client\n");
    printf("Platform: ");
#ifdef _WIN32
    printf("Windows\n");
#elif defined(__linux__)
    printf("Linux\n");
#else
    printf("Unknown\n");
#endif

    if (!InitializeWindow()) {
        fprintf(stderr, "Failed to initialize window\n");
        return 1;
    }

    printf("Window created successfully\n");
    printf("Controls:\n");
    printf("- SQL Input Field: Enter SQL commands\n");
    printf("- Execute Button: Run SQL commands\n");
    printf("- Tables List: View available tables\n");
    printf("- Table View: Display query results\n");
#ifdef __linux__
    printf("- ESC key: Exit application\n");
#endif

    RunEventLoop();
    
    Cleanup();
    return 0;
} 