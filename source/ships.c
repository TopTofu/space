#pragma once

collision_quad cube_collision_quads[6] = {
    { .a = { 0.5, -.5, -.5 }, .b = { 0.5, 0.5, 0.5 } },
    { .a = { -.5, 0.5, -.5 }, .b = { 0.5, 0.5, 0.5 } },
    { .a = { -.5, -.5, 0.5 }, .b = { 0.5, 0.5, 0.5 } },
    { .a = { -.5, -.5, 0.5 }, .b = { -.5, 0.5, -.5 } },
    { .a = { -.5, -.5, -.5 }, .b = { 0.5, -.5, 0.5 } },
    { .a = { -.5, -.5, -.5 }, .b = { 0.5, 0.5, -.5 } }
};

static inline vec3 collision_quad_get_center(collision_quad quad) {
    return vec_add(quad.a, vec_mul(vec_sub(quad.b, quad.a), 0.5f));
}

static void render_ship(ship_info* ship) {
    for (int i = 0; i < SHIP_PART_MAX_COUNT; i++) {
        ship_part part = ship->parts[i];
        if (!part.active) { continue; }

        vec3 translation = vec_add(ship->position, part.offset);
    
        render_mesh_basic(get_type(&part).mesh, 
            .translation = translation,
            .rotation = part.rotation);
    }
}

static void ship_add_part(ship_info* ship, vec3 position, quat rotation, ship_part_type_id type_id) {
    ship_part* part = 0;
    for (int i = 0; i < SHIP_PART_MAX_COUNT; i++) {
        if (!ship->parts[i].active) {
            part = &ship->parts[i];
            break;
        }
    }
    
    if (!part) { return; }
    
    ship->part_count++;
    
    part->type_id = type_id;
    part->active = true;
    part->offset = vec_sub(position, ship->position);
    part->rotation = rotation;
}

typedef struct {
    ship_part* part;
    collision_quad quad;
    float distance;
} part_at_mouse_result;

static part_at_mouse_result get_part_at_mouse(ship_info* ship) {
    vec3 ray = ray_from_screen(global->mouse.position);
    
    part_at_mouse_result result;
    
    result.distance = 100;
    result.part = 0;
    result.quad = (collision_quad) { 0 };
    
    for (int part_id = 0; part_id < SHIP_PART_MAX_COUNT; part_id++) {
        ship_part* part = &ship->parts[part_id];
        if (!part->active) { continue; }
        
        vec3 part_position = vec_add(part->offset, ship->position);
        
        for (int i = 0; i < array_count(cube_collision_quads); i++) {
            collision_quad quad = cube_collision_quads[i];
            
            quad.a = vec_add(quad.a, part_position);
            quad.b = vec_add(quad.b, part_position);
            
            float d = intersect_ray_quad(global->editor_camera.position, ray, quad);
                        
            if (d >= 0 && d < result.distance) { 
                result.distance = d;
                result.quad = quad;
                result.part = part;
            }
        }
    }
    
    return result;
}

static void update_and_render_part_preview(ship_info* ship, ship_part_type_id type_id) {
    part_at_mouse_result get_result = get_part_at_mouse(ship);
    
    if (global->part_rotation_t < 1.0) {
        global->part_rotation_t += global->time.dt * 2.;
    } else {
        global->part_rotation_t = 1.0;
    }
    
    global->current_part_rotation = quat_slerp(global->current_part_rotation,
        global->part_rotation_t, global->current_part_rotation_target);
    
    if (get_result.part) {
        vec3 quad_offset = vec_sub(collision_quad_get_center(get_result.quad), vec_add(get_result.part->offset, ship->position));
        vec3 offset = vec_add(get_result.part->offset, vec_mul(quad_offset, 2.f)); 
        
        vec3 position = vec_add(ship->position, offset);
        
        render_mesh_basic(part_types[type_id].mesh, .translation = position, .rotation = global->current_part_rotation,
            .color = (color)RGBA(200, 100, 100, 100));
        
        if (global->mouse.left_down_this_frame) {
            ship_add_part(ship, position, global->current_part_rotation_target, type_id);
        }
    }
}

static void delete_part_at_mouse() {
    if (ship.part_count <= 1) { return; }
    part_at_mouse_result get_result = get_part_at_mouse(&ship);
    
    if (get_result.part) {
        get_result.part->active = false;
        ship.part_count--;
    }
}

static void pick_part_type_at_mouse() {
    part_at_mouse_result get_result = get_part_at_mouse(&ship);
    
    if (get_result.part) {
        global->current_part_type_id = get_result.part->type_id;
    }
}

static void init_ship_part_types(game_state* state) {
    part_types[PART_CUBE] = (ship_part_type) {
        .id = PART_CUBE,
        .mesh = make_cube_mesh(),
    };
    
    part_types[PART_THRUSTER] = (ship_part_type) {
        .id = PART_THRUSTER,
        .mesh = make_thruster_mesh(),
    };
    
    part_types[PART_TANK] = (ship_part_type) {
        .id = PART_TANK,
        .mesh = make_tank_mesh(),
    };
    
    part_types[PART_SLOPE] = (ship_part_type) {
        .id = PART_SLOPE,
        .mesh = make_slope_mesh(),
    };
    
    part_types[PART_WING] = (ship_part_type) {
        .id = PART_WING,
        .mesh = make_wing_mesh(),
    };
    
    part_types[PART_WING_TIP] = (ship_part_type) {
        .id = PART_WING_TIP,
        .mesh = make_wing_tip_mesh(),
    };
    
    part_types[PART_CONNECTOR] = (ship_part_type) {
        .id = PART_CONNECTOR,
        .mesh = make_connector_mesh(),
    };

    part_types[PART_QUARTER_TUBE] = (ship_part_type) {
        .id = PART_QUARTER_TUBE,
        .mesh = make_quarter_tube_mesh(),
    };
    
    part_types[PART_FIN] = (ship_part_type) {
        .id = PART_FIN,
        .mesh = make_fin_mesh(),
    };
    
    part_types[PART_CORNER] = (ship_part_type) {
        .id = PART_CORNER,
        .mesh = make_corner_mesh(),
    };
}

