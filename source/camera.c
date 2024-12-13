#pragma once

static void camera_set_default(game_state* state) {
    state->camera.position = vec3(0, 0, 0);
    state->camera.up = vec3(0, 1, 0);
    state->camera.right = vec3(1, 0, 0);
    state->camera.forward = vec3(0, 0, -1);
    state->camera.fov = 90;
    state->camera.far = 100;
    state->camera.near = 0.1;
}

static void camera_move_forward(game_state* state, float x) {
    camera_info* cam = &state->camera;
    vec3 dir = cam->forward;
    
    cam->position = vec_add(cam->position, vec_mul(dir, x));    
}

static void camera_move_right(game_state* state, float x) {
    camera_info* cam = &state->camera;
    vec3 dir = cam->right;
    
    cam->position = vec_add(cam->position, vec_mul(dir, x));    
}

static void camera_move_up(game_state* state, float x) {
    camera_info* cam = &state->camera;
    cam->position.y += x;
}

static void camera_rotate_x(game_state* state, float deg) {
    camera_info* cam = &state->camera;
    float rad = DEG_TO_RAD(deg);
    
    cam->forward = vec_rotate(cam->forward, rad, vec3(1, 0, 0));
    cam->up = vec_norm(vec_cross(cam->right, cam->forward));
}

static void camera_rotate_y(game_state* state, float deg) {
    camera_info* cam = &state->camera;
    float rad = DEG_TO_RAD(deg);
    
    cam->forward = vec_rotate(cam->forward, rad, vec3(0, 1, 0));
    cam->right = vec_norm(vec_cross(cam->forward, cam->up));
}

static void camera_rotate_around_y(game_state* state, vec3 p, float deg) {
    camera_info* cam = &state->camera;
    float rad = DEG_TO_RAD(deg);
    
    camera_rotate_y(state, rad);
    
    float distance = vec_len(vec_sub(cam->position, p));
    cam->position = vec_add(p, vec_mul(cam->forward, distance));
}
