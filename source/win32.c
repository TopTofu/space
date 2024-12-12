#pragma once

#include <windows.h>
#include <dsound.h>
#include <stdio.h>

#include "vector.c"
#include "string.c"

#include "platform.h"

#define win32_handle_failed_assertion platform_handle_failed_assertion
#define win32_add_file_watch          platform_add_file_watch
#define win32_find_all_files          platform_find_all_files
#define win32_sleep                   platform_sleep

#include "game.c"

typedef struct {
    HWND window_handle;
    platform_info* platform;
} win32_window_data;

typedef struct {
    FILETIME last_write_time;
    string path;
    void (*callback)(string, void*);
    void* data;
} win32_file_watch_info;

#if DEV
    #define MAX_FILE_WATCH_COUNT 128
    win32_file_watch_info files_to_watch[MAX_FILE_WATCH_COUNT];
    int file_watch_count = 0;
#endif


static void win32_handle_failed_assertion(char* expr_str, char* file, int line) {
    report("Assertion at %s:%d failed!\n\t%s\n", file, line, expr_str);
    *(int*)0 = 0;
}

static inline void win32_sleep(u64 time) {
    Sleep(time);
}

static LRESULT CALLBACK win32_main_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    win32_window_data* window_data = (win32_window_data*)GetWindowLongPtr(window, GWLP_USERDATA);
    if (!window_data) { return DefWindowProc(window, message, wparam, lparam); }
    
    platform_info* platform = window_data->platform;
    
    LRESULT result = 0;
    switch(message) {
        case WM_CLOSE: {
            platform->is_running = false;
        } break;
        
        case WM_SIZE: {
            window_data->platform->window_width = LOWORD(lparam);
            window_data->platform->window_height = HIWORD(lparam);
            game_resize_window(window_data->platform);
        } break;
        
        case WM_CHAR: {
            event_info event = { .type=TEXT_INPUT_EVENT };
            event.text_input_event.c = wparam;
            add_event(window_data->platform, event);
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            key_event key = { .code = wparam };
            
            key.was_down = (lparam & (1 << 30)) != 0;
            key.is_down  = (lparam & (1 << 31)) == 0;
            
            key.held = (key.is_down && key.was_down);

            event_info event = { .type=KEY_INPUT_EVENT, .key_event=key };
            add_event(window_data->platform, event);
        } break;
        
        case WM_LBUTTONDOWN: {
            event_info event;
            event.type = KEY_INPUT_EVENT;
            event.key_event = (key_event){.code=1, .is_down=true, .was_down=false, .held=false };
            add_event(window_data->platform, event);
        } break;
        case WM_LBUTTONUP: {
            event_info event = { .type=KEY_INPUT_EVENT, 
                .key_event={.code=1, .is_down=false, .was_down=true, .held=false }
            };
            add_event(window_data->platform, event);
        } break;
        case WM_RBUTTONDOWN: {
            event_info event = { .type=KEY_INPUT_EVENT, 
                .key_event={.code=3, .is_down=true, .was_down=false, .held=false }
            };
            add_event(window_data->platform, event);
        } break;
        case WM_RBUTTONUP: {
            event_info event = { .type=KEY_INPUT_EVENT, 
                .key_event={.code=3, .is_down=false, .was_down=true, .held=false }
            };
            add_event(window_data->platform, event);
        } break;
        case WM_MBUTTONDOWN: {
            event_info event = { .type=KEY_INPUT_EVENT, 
                .key_event={.code=2, .is_down=true, .was_down=false, .held=false }
            };
            add_event(window_data->platform, event);
        } break;
        case WM_MBUTTONUP: {
            event_info event = { .type=KEY_INPUT_EVENT, 
                .key_event={.code=2, .is_down=false, .was_down=true, .held=false }
            };
            add_event(window_data->platform, event);
        } break;

        case WM_MOUSEMOVE: {
            event_info event;
            event.type = MOUSE_MOVE_EVENT;
            event.mouse_move_event = (mouse_move_event){ .x=LOWORD(lparam), .y=HIWORD(lparam) };
            add_event(window_data->platform, event);
        } break;
        case WM_MOUSEWHEEL: {
            event_info event;
            event.type = MOUSE_WHEEL_EVENT;
            event.mouse_wheel_event.scroll = (float)GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            add_event(window_data->platform, event);
        } break;

        default: {
            result = DefWindowProc(window, message, wparam, lparam);
        } break;
    }
    
    return result;
}

static int win32_find_all_files(char* dir, char* format, void* memory, u64* bytes_used) {
    WIN32_FIND_DATA find_data;
    int dir_len = strlen(dir);

    char* dir_format = malloc(dir_len + strlen(format) + 1);
    strcpy(dir_format, dir);
    strcpy(dir_format + dir_len, format);

    HANDLE find = FindFirstFile(dir_format, &find_data);
    if (find == INVALID_HANDLE_VALUE) { return 0; }

    u8* dest = memory;

    int i = 0;
    do {
        if (find_data.cFileName[0] == '.') { continue; }
        int len = strlen(find_data.cFileName);
        strcpy(dest, find_data.cFileName);
        dest[len] = '\0';
        dest += len + 1;
        i++; 
    } while (FindNextFile(find, &find_data));

    free(dir_format);

    if (bytes_used) { *bytes_used = dest - memory; }

    return i;
}

static MONITORINFO win32_get_primary_monitor_info() {
    POINT zero = {0, 0};
    HMONITOR monitor_handle = MonitorFromPoint(zero, MONITOR_DEFAULTTOPRIMARY);
    
    MONITORINFO result = { .cbSize = sizeof(MONITORINFO) };
    bool ok = GetMonitorInfo(monitor_handle, &result);
    
    if (!ok) {
        report("Failed to get primary monitor info: %i", GetLastError());
    }
    
    return result;
}

static void win32_get_primary_screen_dimensions_without_taskbar(int* width, int* height) {
    RECT work_rect = win32_get_primary_monitor_info().rcWork;
    *width  = work_rect.right - work_rect.left;
    *height = work_rect.bottom - work_rect.top;
}

static void win32_check_for_messages(HWND window_handle) {
    MSG message;

    while (PeekMessage(&message, window_handle, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

static inline FILETIME win32_get_last_write_time(string file_path) {
    char _path[256];
    memory_copy(_path, file_path.data, file_path.length);
    _path[file_path.length] = 0;

    FILETIME result = { 0 };
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(_path, GetFileExInfoStandard, &data)) {
        result = data.ftLastWriteTime;
    }
    
    return result;
}

static void win32_add_file_watch(string path, void (*callback)(string, void*), void* data) {
#if DEV
    assert(MAX_FILE_WATCH_COUNT > file_watch_count);
    files_to_watch[file_watch_count++] = (win32_file_watch_info){ 
        .last_write_time = win32_get_last_write_time(path), 
        .path = path, 
        .callback = callback,
        .data = data };
#endif
}


static void win32_check_file_watchers() {
    for (int i = 0; i < file_watch_count; i++) {
        win32_file_watch_info* watch = &files_to_watch[i];
        FILETIME write_time = win32_get_last_write_time(watch->path);
        
        if (CompareFileTime(&write_time, &watch->last_write_time) != 0) {
            watch->callback(watch->path, watch->data);
            watch->last_write_time = write_time;
        }
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR args, int show_code) {
#if DEV
    AllocConsole();
    AttachConsole(ATTACH_PARENT_PROCESS);
    freopen("CONOUT$", "w+", stdout);
#endif
    
    platform_info platform = { 0 };
    platform.is_running = 1;
    
    { // === allocate game memory
        platform.permanent_storage_size = megabytes(512);
        platform.transient_storage_size = megabytes(512);
        
#if DEV
        LPVOID game_memory_base = (LPVOID)terabytes(1);
#else
        LPVOID game_memory_base = 0;
#endif

        platform.permanent_storage = VirtualAlloc(game_memory_base,
            platform.permanent_storage_size + platform.transient_storage_size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE);
        
        if (!platform.permanent_storage) {
            report("Failed to allocate the games memory\n");
            return 1;        
        }
        
        platform.transient_storage = (u8*)platform.permanent_storage + platform.permanent_storage_size;
    }

    int screen_width, screen_height;
    win32_get_primary_screen_dimensions_without_taskbar(&screen_width, &screen_height);
    
    win32_window_data window_data;
    window_data.platform = &platform;
    
    { // === init window
        WNDCLASSEX window_class = { 0 };
        
        window_class.cbSize         = sizeof(WNDCLASSEX);
        window_class.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        window_class.hInstance      = instance;
        window_class.lpfnWndProc    = win32_main_window_proc;
        window_class.lpszClassName  = "main_window_class";
        window_class.hCursor        = LoadCursor(0, IDC_ARROW);
    
        if (!RegisterClassEx(&window_class)) {
            report("Could not register the window class! code: %i\n", GetLastError());
            return 1;
        }
        
        int window_base_x = screen_width * 0.05;
        int window_base_y = screen_height * 0.05;
        
        platform.window_width = screen_width - 2 * window_base_x;
        platform.window_height = screen_height - 2 * window_base_y;
        
        RECT rc = { 0, 0, platform.window_width, platform.window_height };
        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        AdjustWindowRect(&rc, style, 0);
        
        HWND window_handle = CreateWindow(window_class.lpszClassName,
            "Space",
            style,
            window_base_x,
            window_base_y,
            rc.right - rc.left,
            rc.bottom - rc.top,
            0, 0, instance, 0);
        
        if (!window_handle) {
            report("Could not create the main window. code: %i\n", GetLastError());
            return 1;
        }
        
        window_data.window_handle = window_handle;
        SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR)&window_data);
        
        ShowWindow(window_handle, SW_NORMAL);
        UpdateWindow(window_handle);  
    }
    
    HDC device_context = GetDC(window_data.window_handle);
    HGLRC render_context;
    { // === init opengl
        PIXELFORMATDESCRIPTOR pixel_format = {0};
        pixel_format.nSize = sizeof(pixel_format);
        pixel_format.nVersion = 1;
        pixel_format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pixel_format.cColorBits = 32;
        pixel_format.cAlphaBits = 8;
        pixel_format.iLayerType = PFD_MAIN_PLANE;

        int suggested_pixel_format_index = ChoosePixelFormat(device_context, &pixel_format);
        if (!suggested_pixel_format_index) {
            report("Did not get a pixel format index\n");
            return 1;
        }
        
        PIXELFORMATDESCRIPTOR suggested_pixel_format;
        DescribePixelFormat(device_context, suggested_pixel_format_index, 
                sizeof(suggested_pixel_format), &suggested_pixel_format);
        SetPixelFormat(device_context, suggested_pixel_format_index, &suggested_pixel_format);

        // @Info: In order to create a proper Opengl 3.2+ context we need to get
        //        the extension function wglCreateContextAttribsARB().
        //        For this to work though, we need have a context already...
        //        So we create this dummy context to get the function pointer,
        //        unbind and delete it before binding the actual context.
        //         
        //        Because Windows can not change the pixel format for a given device_context,
        //        we need to the pixel format we want even before we create the dummy context.
        HGLRC dummy_context = wglCreateContext(device_context);
        wglMakeCurrent(device_context, dummy_context);

        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        int attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 4,
			WGL_CONTEXT_FLAGS_ARB, 0,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
        };
        
        render_context = wglCreateContextAttribsARB(device_context, 0, attribs);
        wglMakeCurrent(0, 0);
        wglDeleteContext(dummy_context);

        if(!wglMakeCurrent(device_context, render_context)) {
            report("Could not make the OpenGL context current\n");
            return 1;
        }
        
        gladLoadGL();
    }
    
    game_init_memory(&platform);
    
    FILETIME system_time;
    GetSystemTimeAsFileTime(&system_time);
    
    u32 previous_time = system_time.dwLowDateTime;
    platform.current_time = previous_time;
    
    while (platform.is_running) {
        win32_check_for_messages(window_data.window_handle);
        win32_check_file_watchers();
        
        GetSystemTimeAsFileTime(&system_time);
        previous_time = platform.current_time;
        platform.current_time = system_time.dwLowDateTime;
        platform.dt_ms = (platform.current_time - previous_time) / 10000.0;
        
        game_update_and_render(&platform);
        
        platform.event_count = 0;
        SwapBuffers(device_context);
    }
    
    return 0;
}
