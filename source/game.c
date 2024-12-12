#pragma once

#include "game.h"
#include "memory.c"

// === utils
#define push_transient(size) push_size(&global->transient_arena, size);
#define push_permanent(size) push_size(&global->permanent_arena, size);

static inline string push_string(memory_arena* arena, u32 size) {
    return (string) {
        .data = push_size(arena, size),
        .length = 0,
        .size = size
    };
}

#define report_ingame(...) // @Todo

static string read_file(string path, memory_arena* arena) {
    char* c_path = push_transient(path.length + 1);
    memory_copy(c_path, path.data, path.length);
    c_path[path.length] = '\0';
    
    FILE* file = fopen(c_path, "r");
    if (!file) {
        report("Could not open file %s\n", c_path);
        return empty_string();
    } 
    
    fseek(file, 0, SEEK_END);
    unsigned int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    string content;
    content.data = push_size(arena, size);

    content.length = fread(content.data, 1, size, file);
    content.size = content.length;

    fclose(file);

    return content;
}

static inline vec2 get_window_dims() {
    return vec2(global->platform->window_width, global->platform->window_height);
}

static void sort(void* array, int count, int element_size, bool (*compare)(void*, void*)) {
    bool did_swaps = true;
    
    void* temp = push_transient(element_size);
    
    char* buffer = array;
    
    while(did_swaps) {
        did_swaps = false;
        
        for (int i = 0; i < count - 1; i++) {
            void* a = &buffer[i * element_size];
            void* b = &buffer[(i + 1) * element_size];
            
            if (compare(a, b)) {
                memory_copy(temp, a, element_size);
                memory_copy(a, b, element_size);
                memory_copy(b, temp, element_size);
                
                did_swaps = true;
            }
        }
    }
}

static void add_event(platform_info* platform, event_info e) {
    if (platform->event_count + 1 > MAX_EVENT_COUNT) {
        report("Event buffer is full\n");
    } else {
        platform->events[platform->event_count++] = e;
    }
}

void update_time_info(time_info* time, float dt_ms) {
    time->realtime_dt_ms = dt_ms;
    time->realtime_dt    = time->realtime_dt_ms / 1000.f;
    
    time->dt_ms = time->realtime_dt_ms * time->simulation_speed;
    time->dt    = time->dt_ms / 1000.f;
    
    time->in_milliseconds += time->dt_ms;
    time->in_seconds      += time->dt;
    
    if (time->last_second + 1 < time->in_seconds) {
        time->once_per_second = true;
        time->last_second++;
    } else {
        time->once_per_second = false;
    }
}


// === source includes
#include "render.c"
#include "input.c"
#include "ui.c"


static void game_init_memory(platform_info* platform) {
    assert(platform->permanent_storage);
    assert(sizeof(game_state) < platform->permanent_storage_size);
    
    game_state* state = platform->permanent_storage;
    
    global = state;
    global->platform = platform;
    
    state->time.simulation_speed = 1.0;
    
    init_arena(&state->permanent_arena, 
        platform->permanent_storage_size - sizeof(game_state), 
        (u8*)platform->permanent_storage + sizeof(game_state));
    init_arena(&state->transient_arena,
        platform->transient_storage_size,
        (u8*)platform->transient_storage);
    
    init_renderer(state);    
    load_all_shaders(state, "../source/shaders/");
    
    state->camera.position = vec3(0, 0, 0);
    state->camera.up = vec3(0, 1, 0);
    state->camera.right = vec3(1, 0, 0);
    state->camera.forward = vec3(0, 0, -1);
    state->camera.fov = 90;
    state->camera.far = 100;
    state->camera.near = 0.1;
}

static void game_update_and_render(platform_info* platform) {
    game_state* state = platform->permanent_storage;
    
    update_time_info(&state->time, platform->dt_ms);
    process_input(state);
    
    ui_frame_begin(state);
    

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    state->renderer.projection_matrix = make_projection_matrix(
        state->platform->window_width,  
        state->platform->window_height,
        state->camera.fov,
        state->camera.near,
        state->camera.far
    );
    
    state->camera.view_matrix = make_view_matrix(state->camera.position, 
        state->camera.forward, state->camera.up, state->camera.right);
    
    state->renderer.cube_mesh.translation.z = 5 + sin(state->time.in_seconds);
    state->renderer.cube_mesh.rotation = quat_mul_quat(quat_from_axis_angle(vec_norm(vec3(1, 1, 1)), .002), 
        state->renderer.cube_mesh.rotation);
    
    { // === render into scene texture
        glBindFramebuffer(GL_FRAMEBUFFER, state->renderer.scene_framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        {   
            render_mesh(state->renderer.cube_mesh, get_shader("basic3d"));
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    if (1) { // === render scene texture
        shader_info* shader = get_shader("scene");
        glUseProgram(shader->id);
        
        shader_bind_texture(shader, &state->renderer.scene_texture, "scene_texture", 0);
        
        glBindVertexArray(global->renderer.quad_mesh.vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void game_resize_window(platform_info* platform) {
    glViewport(0, 0, platform->window_width, platform->window_height);
}
