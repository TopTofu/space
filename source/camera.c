#pragma once

#define make_view_matrix_from_camera(cam) make_view_matrix((cam)->position, (cam)->forward, (cam)->up, (cam)->right)

static void camera_reset(camera_info* cam) {
    cam->position = vec3(0, 0, 5);
    cam->up = vec3(0, 1, 0);
    cam->right = vec3(1, 0, 0);
    cam->forward = vec3(0, 0, -1);
    
}

static void camera_set_default(camera_info* cam) {
    *cam = (camera_info) { 0 };

    camera_reset(cam);
    cam->params.translation = cam->position;

    cam->fov = 90;
    cam->far = 100;
    cam->near = 0.1;
}

static void camera_move_forward(camera_info* cam, float x) {
    vec3 dir = cam->forward;
    
    cam->position = vec_add(cam->position, vec_mul(dir, x));    
}

static void camera_move_right(camera_info* cam, float x) {
    vec3 dir = cam->right;
    
    cam->position = vec_add(cam->position, vec_mul(dir, x));    
}

static void camera_move_up(camera_info* cam, float x) {
    cam->position.y += x;
}

static void camera_rotate_x(camera_info* cam, float deg) {
    float rad = DEG_TO_RAD(deg);
    
    cam->forward = vec_rotate(cam->forward, rad, cam->right);
    cam->up = vec_norm(vec_cross(cam->right, cam->forward));
}

static void camera_rotate_y(camera_info* cam, float deg) {
    float rad = DEG_TO_RAD(deg);
    
    cam->forward = vec_rotate(cam->forward, rad, cam->up);
    cam->right = vec_norm(vec_cross(cam->forward, cam->up));
}

static void camera_rotate_y_around_point(camera_info* cam, vec3 p, float deg) {
    camera_rotate_y(cam, deg);

    float distance = vec_len(vec_sub(cam->position, p));
    cam->position = vec_add(p, vec_mul(cam->forward, distance));
}

static void camera_rotate_x_around_point(camera_info* cam, vec3 p, float deg) {
    camera_rotate_x(cam, deg);
    
    float distance = vec_len(vec_sub(cam->position, p));
    cam->position = vec_add(p, vec_mul(cam->forward, distance));
}

static void update_editor_camera(game_state* state) {
    camera_info* cam = &state->camera;
    camera_reset(cam);
    
    float cam_speed = keymap_get(state, KEY_SHIFT) ? 3. : 1.;
    cam_speed *= state->time.dt;
    
    float cam_rotation_step = 100. * cam_speed;
    float cam_move_step     = 50. * cam_speed; 
    float cam_up_step       = 3. * cam_speed;
    
    float pitch_limit = 75;
        
    if (cam->controls.pitch_up) { 
        if (cam->params.rotation.x < pitch_limit) {
            cam->params.rotation.x += cam_rotation_step; 
        }
    }
    if (cam->controls.pitch_down) { 
        if (cam->params.rotation.x > -pitch_limit) {
            cam->params.rotation.x -= cam_rotation_step; 
        }
    }
    
    if (cam->controls.left) { cam->params.rotation.y -= cam_rotation_step; }
    if (cam->controls.right) { cam->params.rotation.y += cam_rotation_step; }
    
    if (cam->controls.backward) { 
        float distance_to_ship = vec_len(vec_sub(cam->position, ship.position));
        if (distance_to_ship > cam_move_step) {
            cam->params.translation.z += cam_move_step; 
        }
        
        // @Note: only because we use KEY_MOUSE_WHEEL which doesn't get an up event
        cam->controls.backward = false;
    }
    if (cam->controls.forward) { 
        float distance_to_ship = vec_len(vec_sub(cam->position, ship.position));
        if (distance_to_ship < 50) { 
            cam->params.translation.z -= cam_move_step; 
        }
        
        // @Note: only because we use KEY_MOUSE_WHEEL which doesn't get an up event
        cam->controls.forward = false;
    }
    
    cam->position = cam->params.translation;
    
    vec3 pivot = ship.position;
    
    camera_rotate_y_around_point(cam, pivot, cam->params.rotation.y);
    camera_rotate_x_around_point(cam, pivot, cam->params.rotation.x);
    
    cam->view_matrix = make_view_matrix_from_camera(cam);
}
