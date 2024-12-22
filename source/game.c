#pragma once

#include "game.h"
#include "memory.c"

int debug_index_count = 12;


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

static void update_time_info(time_info* time, float dt_ms) {
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

static inline vec2 screen_to_ndc(vec2 screen) {
    return vec2(2.0 * screen.x                                     / global->platform->window_width - 1.0,
                2.0 * (global->platform->window_height - screen.y) / global->platform->window_height - 1.0);
}

static inline float distance_ray_point(vec3 ray_origin, vec3 ray_dir, vec3 p) {
    return vec_len(vec_cross(ray_dir, vec_sub(p, ray_origin)));
}

static inline vec3 intersect_ray_point(vec3 ray_origin, vec3 ray_dir, vec3 p) {
    return vec_add(ray_origin, vec_mul(ray_dir, vec_dot(ray_dir, vec_sub(p, ray_origin))));
}

static vec3 ray_from_screen(vec2 screen) {
    vec2 ndc = screen_to_ndc(screen);
    vec4 clip_space = vec4(ndc.x, ndc.y, -1, 1);
    
    mat4 inverse_projection = mat4_inv(global->renderer.projection_matrix);
    
    vec4 temp = vec_transform(inverse_projection, clip_space);
    vec4 eye_space = vec4(temp.x, temp.y, -1, 0);
    
    mat4 inverse_view = mat4_inv(global->current_camera->view_matrix);
    vec3 result = vec_transform(inverse_view, eye_space).xyz;
    
    result = vec_norm(result);
    
    return result;
}

static float intersect_ray_plane(vec3 ray_origin, vec3 ray_dir, vec3 plane_origin, vec3 plane_normal) {
    float dot = vec_dot(ray_dir, plane_normal);
    if (ABS(dot) == 0) { return -1.; }

    float result = vec_dot(vec_sub(plane_origin, ray_origin), plane_normal) / dot;

    return result;
}


// @Info: we get the intersection between the ray and the plan the quad is on 
//        and check if the intersection point is between the quads defining points
static float intersect_ray_quad(vec3 ray_origin, vec3 ray_dir, collision_quad quad) {
    // @Speed: i feel like most of the branches could go.. 
    // @Note: since the quad is axis-aligned, the normal is the axis where quad.min and quad.max are the same
    vec3 plane_normal = quad.a.x == quad.b.x ? vec3(1, 0, 0) :
                        quad.a.y == quad.b.y ? vec3(0, 1, 0) :
                        vec3(0, 0, 1);
                        
    float distance_to_plane = intersect_ray_plane(ray_origin, ray_dir, quad.a, plane_normal);
    vec3 intersection = vec_add(vec_mul(ray_dir, distance_to_plane), ray_origin);
    
    bool in_quad = false;
    
    if (plane_normal.x == 1.f) {
        float min_y = quad.a.y < quad.b.y ? quad.a.y : quad.b.y;
        float min_z = quad.a.z < quad.b.z ? quad.a.z : quad.b.z;
        
        float max_y = quad.a.y > quad.b.y ? quad.a.y : quad.b.y;
        float max_z = quad.a.z > quad.b.z ? quad.a.z : quad.b.z;
        
        in_quad = (min_y <= intersection.y) && (min_z <= intersection.z) 
            && (max_y >= intersection.y) && (max_z >= intersection.z);
    } else if (plane_normal.y == 1.f) {
        float min_x = quad.a.x < quad.b.x ? quad.a.x : quad.b.x;
        float min_z = quad.a.z < quad.b.z ? quad.a.z : quad.b.z;
        
        float max_x = quad.a.x > quad.b.x ? quad.a.x : quad.b.x;
        float max_z = quad.a.z > quad.b.z ? quad.a.z : quad.b.z;
        
        in_quad = (min_x <= intersection.x) && (min_z <= intersection.z) 
            && (max_x >= intersection.x) && (max_z >= intersection.z);
    } else {
        float min_x = quad.a.x < quad.b.x ? quad.a.x : quad.b.x;
        float min_y = quad.a.y < quad.b.y ? quad.a.y : quad.b.y;
        
        float max_x = quad.a.x > quad.b.x ? quad.a.x : quad.b.x;
        float max_y = quad.a.y > quad.b.y ? quad.a.y : quad.b.y;
        
        in_quad = (min_x <= intersection.x) && (min_y <= intersection.y) 
            && (max_x >= intersection.x) && (max_y >= intersection.y);
    }     

    if (in_quad) { return distance_to_plane; }
    return -1;
}

// === source includes
#include "render.c"
#include "camera.c"
#include "ui.c"
#include "ships.c"
#include "input.c"

static bool editor_controls(game_state* state, key_event event) { 
    camera_info* cam = &state->editor_camera;

    bool result = true;
    
    switch (event.code) {
        case KEY_A: {
            cam->controls.left = event.is_down;
        } break;
        case KEY_D: {
            cam->controls.right = event.is_down;
        } break;
        
        case KEY_MOUSE_DOWN: {
            cam->controls.backward = event.is_down;
        } break;
        case KEY_MOUSE_UP: {
            cam->controls.forward = event.is_down;
        } break;
        
        case KEY_W: {
            cam->controls.pitch_up = event.is_down;
        } break;
        case KEY_S: {
            cam->controls.pitch_down = event.is_down;
        } break;
        
        default: { result = false; } break;
    }
    
    if (result) { return result; }
    
    if (!event.is_down) { return result; }
    result = true;
    
    switch (event.code) {
        case KEY_C: {
            camera_set_default(cam);
        } break;
        
        case KEY_Q: {
            pick_part_type_at_mouse();
        } break;
        
        case KEY_X: {
            delete_part_at_mouse();
        } break;
        
        case KEY_R: {
            vec3 axis = vec3(1, 0, 0);
            float rad = DEG_TO_RAD(90);
            
            if (!vec_eq(state->current_part_rotation, state->current_part_rotation_target)) {
                state->current_part_rotation = state->current_part_rotation_target;
            }
            
            state->current_part_rotation_target = quat_mul_quat(quat_from_axis_angle(axis, rad), state->current_part_rotation);
            state->part_rotation_t = 0;
        } break;
        case KEY_T: {
            vec3 axis = vec3(0, 1, 0);
            float rad = DEG_TO_RAD(90);
            
            if (!vec_eq(state->current_part_rotation, state->current_part_rotation_target)) {
                state->current_part_rotation = state->current_part_rotation_target;
            }
            
            state->current_part_rotation_target = quat_mul_quat(quat_from_axis_angle(axis, rad), state->current_part_rotation);
            state->part_rotation_t = 0;
        } break;

        default: { result = false; } break;
    }
    
    return result;
}

framebuffer_info icon_fb;
static void update_and_render_part_buttons() {
    int pad = 15;
    int button_w = 100;
    int x = pad;
    int y = global->platform->window_height - button_w - pad;
    
    quat model_rotation = unit_quat();
    model_rotation = quat_mul_quat(quat_from_axis_angle(vec3(0, 1, 0), 
        DEG_TO_RAD(global->time.in_seconds * 15.)), model_rotation);
    model_rotation = quat_mul_quat(quat_from_axis_angle(vec3(1, 0, 0), DEG_TO_RAD(-30)), model_rotation);
    
    void* id = update_and_render_part_buttons;
    
    for (int i = 0; i < PART_TYPE_COUNT; i++) {
        ship_part_type type = part_types[i];
        
        if (global->current_part_type_id == i) { ui_quad(x - 2, y - 2, button_w + 4, button_w + 4, (color)white()); }
        
        {   
            glBindFramebuffer(GL_FRAMEBUFFER, icon_fb.id);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, 100, 100);
         
            shader_info* shader = get_shader("game_object");
            glUseProgram(shader->id);
            
            mat4 model = make_model_matrix(vec3(0, 0, 5), vec3(2, 2, 2), model_rotation);
            mat4 view  = make_view_matrix(vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0), vec3(1, 0, 0));
            mat4 proj  = make_projection_matrix(icon_fb.attachments[0].width, icon_fb.attachments[0].height, 40, 0.1, 100);

            shader_set_uniform(shader, "model", model);
            shader_set_uniform(shader, "view", view);
            shader_set_uniform(shader, "projection", proj);
            shader_set_uniform(shader, "color", (color)white());
            
            glBindVertexArray(type.mesh.vao);
            glDrawElements(type.mesh.primitive, type.mesh.index_count, GL_UNSIGNED_INT, 0);
            
            glBindVertexArray(0);
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, global->platform->window_width, global->platform->window_height);
        }
        
        bool clicked = button(IDX(id, i), x, y, button_w, button_w, (color)RGB(100, 100, 150));
        if (clicked) { 
            global->current_part_type_id = i; 
           
            global->current_part_rotation = (quat) { 0, 0, 0, 1 };
            global->current_part_rotation_target = (quat) { 0, 0, 0, 1 };
        }
        
        ui_quad_textured(x, y, button_w, button_w, icon_fb.attachments[0].id, get_shader("part_icon"));
        
        x += button_w + pad;
    }
    
}

mesh m;

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
    
    load_all_shaders(state, "../source/shaders/");

    camera_set_default(&state->editor_camera);
    state->current_camera = &state->editor_camera;
    
    init_renderer(state);    

    init_ship_part_types(state);

    ship_add_part(&ship, ship.position, unit_quat(), PART_CUBE);
    
    // m = make_wing_mesh();
    init_framebuffer(&icon_fb);
    framebuffer_add_attachment(&icon_fb, GL_COLOR_ATTACHMENT, 100, 100, .wrap_t = GL_CLAMP_TO_EDGE, .wrap_s = GL_CLAMP_TO_EDGE);
    framebuffer_add_attachment(&icon_fb, GL_DEPTH_ATTACHMENT, 100, 100);
    
    state->current_part_rotation = (quat) { 0, 0, 0, 1 };
    state->current_part_rotation_target = (quat) { 0, 0, 0, 1 };
    
    bind_key_input_proc(editor_controls);
}

static void game_update_and_render(platform_info* platform) {
    game_state* state = platform->permanent_storage;
    
    update_time_info(&state->time, platform->dt_ms);
    process_input(state);
    
    ui_frame_begin(state);
    
    update_editor_camera(state);
    

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    state->renderer.projection_matrix = make_projection_matrix(
        state->platform->window_width,  
        state->platform->window_height,
        state->current_camera->fov,
        state->current_camera->near,
        state->current_camera->far
    );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    { // === render into scene texture
        glBindFramebuffer(GL_FRAMEBUFFER, state->renderer.scene_framebuffer.id);
        u32 draw_buffers[] = { 
            state->renderer.scene_texture->color_attachment_id, 
            state->renderer.scene_per_object_depth_texture->color_attachment_id,
        };
        glDrawBuffers(array_count(draw_buffers), draw_buffers);
        
        float scene_clear_color[] = { 0., 0., 0., 1. };
        glClearTexImage(state->renderer.scene_texture->id, 0, GL_RGBA, GL_FLOAT, scene_clear_color);
        float per_object_depth_clear_color[] = { 1., 1., 1., 1. };
        glClearTexImage(state->renderer.scene_per_object_depth_texture->id, 0, GL_RGBA, GL_FLOAT, scene_clear_color);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        {   
            render_ship(&ship);
            update_and_render_part_preview(&ship, state->current_part_type_id);
            
            debug_render_quad(vec3(-10, -1, -10), vec3(10, -1, 10), (color)RGBA(255, 255, 255, 100));
            
            // render_mesh_basic(m, .translation = vec3(0, 0, 5));
            // .rotation = quat_from_axis_angle(vec3(0, 1, 0), state->time.in_seconds));
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if (1) { // === render scene texture
        shader_info* shader = get_shader("scene");
        glUseProgram(shader->id);
        
        shader_bind_texture(shader, state->renderer.scene_texture, "scene_texture", 0);
        shader_bind_texture(shader, state->renderer.scene_per_object_depth_texture, "scene_per_object_depth", 1);
        shader_bind_texture(shader, state->renderer.scene_depth_texture, "scene_depth", 2);
        
        shader_set_uniform(shader, "far", state->current_camera->far);
        shader_set_uniform(shader, "near", state->current_camera->near);
        
        glBindVertexArray(global->renderer.quad_mesh.vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    update_and_render_part_buttons();
}

static void game_resize_window(platform_info* platform) {
    // @Todo: resize framebuffer
    glViewport(0, 0, platform->window_width, platform->window_height);
}
