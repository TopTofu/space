#pragma once

#define DEV 1

#undef near
#undef far

typedef unsigned char       u8;
typedef unsigned short      u16;  
typedef unsigned int        u32;  
typedef unsigned long long  u64;

typedef char      s8;
typedef short     s16;  
typedef int       s32;  
typedef long long s64;

typedef u8 bool;
#define false 0
#define true 1

#define toggle(b) ((b) = !(b)) 
#define stringify(x) #x

#define array_count(a) (sizeof(a) / sizeof(a[0]))

#define kilobytes(n) ((n) * 1024ull)
#define megabytes(n) (kilobytes(n) * 1024ull)
#define gigabytes(n) (megabytes(n) * 1024ull)
#define terabytes(n) (gigabytes(n) * 1024ull)

#if 1
    #define report(...) printf(__VA_ARGS__)
#else
    #define report(...)
#endif

#if DEV
    #define assert(expr)\
        if (!(expr)) { platform_handle_failed_assertion(stringify(expr), __FILE__, __LINE__); }
#else
    #define assert(...)
#endif

#define print(x) printf("%s = ", #x);\
                 printf(_get_print_type_format(x), x);\
                 printf("\n");
#define _get_print_type_format(X) _Generic((X), u8 :    "%hhu", \
                                                u16:    "%hu",  \
                                                u32:    "%u",   \
                                                u64:    "%I64u",\
                                                s8 :    "%hhd", \
                                                s16:    "%hd",  \
                                                s32:    "%d",   \
                                                s64:    "%lld", \
                                                float:  "%f",   \
                                                double: "%Lf",  \
                                                char*:  "%s",   \
                                                void*:  "%p")
                                                
// === external includes                                 
#include "extern/glad.c"
#include "extern/wglext.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(X) assert(X)
#include "extern/stb_image.h"

// === events
enum {
    KEY_INPUT_EVENT,
    TEXT_INPUT_EVENT,
    MOUSE_MOVE_EVENT,
    MOUSE_WHEEL_EVENT,
    WINDOW_RESIZE_EVENT,

    EVENT_TYPE_COUNT
} event_type;

typedef struct {
    u32 code;
    bool is_down;
    bool was_down;
    bool held;
    bool ctrl;
    bool shift;
    bool alt;
} key_event;

typedef struct {
    int width, height;
} window_resize_event;

typedef struct {
    int x, y;
} mouse_move_event;

typedef struct {
    float scroll;
} mouse_wheel_event;

typedef struct {
    u8 c;
} text_input_event;

typedef struct {
    u32 type;
    
    union {
        mouse_wheel_event   mouse_wheel_event;
        mouse_move_event    mouse_move_event;
        window_resize_event window_resize_event;
        key_event           key_event;
        text_input_event    text_input_event;
    };
} event_info;

typedef struct {
    void* base;
    u64 size;
    u64 used;

    u64 _saved[8];
    u32 _saved_count;
} memory_arena;

typedef struct {
    float dt;
    float dt_ms;
    float in_seconds;
    float in_milliseconds;
    
    float simulation_speed;
    
    float realtime_dt;
    float realtime_dt_ms;

    bool once_per_second;
    u32 last_second;
} time_info;

// === 3D camera
typedef struct {
    union {
        struct { float x, y, z; };
        vec3 position;
    };
    
    float step;
    
    vec3 forward;
    vec3 up;
    vec3 right;
    
    mat4 view_matrix;
    float near, far;
    float fov;
    
    struct {
        bool up, down;
        bool right, left;
        bool forward, backward;
        bool pitch_up, pitch_down;
    } controls;
    
    struct {
        vec3 rotation;
        vec3 translation;
    } params;
} camera_info;

#define MAX_EVENT_COUNT 128
typedef struct {
    u64 permanent_storage_size;
    void* permanent_storage;
    
    u64 transient_storage_size;
    void* transient_storage;
    
    event_info events[MAX_EVENT_COUNT];
    int event_count;
    
    double dt_ms;
    u32 current_time;
    
    int window_width;
    int window_height;
    
    bool is_running;
} platform_info;

typedef struct {
    union {
        struct { float x, y; };
        vec2 position;
    };

    bool left, right, middle;
    bool left_down_this_frame, left_up_this_frame;
    bool right_down_this_frame, right_up_this_frame;
    bool middle_down_this_frame, middle_up_this_frame;

    float scroll;
} mouse_info;

typedef struct game_state game_state;

// === header includes
#include "keycodes.h"
#include "render.h"
#include "input.h"
#include "ui.h"
#include "ships.h"

typedef struct game_state {
    platform_info* platform;
    
    memory_arena permanent_arena;
    memory_arena transient_arena;

    camera_info camera;
    mouse_info mouse;
    u8 keymap[32];
    
    time_info time;
    
    ui_state ui;
    
    renderer_info renderer;
    shader_catalog shaders;
    texture_catalog textures;
    
    text_input* current_text_input;
    key_input_proc* current_input_proc;
    
    quat current_part_rotation;
    quat current_part_rotation_target;
    float part_rotation_t;
    
    int current_part_type_id;
} game_state;

game_state* global;
