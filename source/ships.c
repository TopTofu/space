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
    for (int i = 0; i < ship->part_count; i++) {
        ship_part part = ship->parts[i];

        vec3 translation = vec_add(ship->position, part.offset);
    
        render_mesh_basic(get_type(&part).mesh, 
            .translation = translation,
            .rotation = part.rotation);
    }
}

static void ship_add_part(ship_info* ship, vec3 position, quat rotation, ship_part_type_id type_id) {
    if (ship->part_count >= SHIP_PART_MAX_COUNT) { return; }
    
    ship_part* part = &ship->parts[ship->part_count++];
    
    part->type_id = type_id;
    part->offset = vec_sub(position, ship->position);
    part->rotation = rotation;
}

static void update_and_render_part_preview(ship_info* ship, ship_part_type_id type_id) {
    vec3 ray = ray_from_screen(global->mouse.position);
    
    float min_distance = 100;
    ship_part* closest = 0;
    collision_quad hit_quad = { 0 };
    
    for (int part_id = 0; part_id < ship->part_count; part_id++) {
        ship_part* part = &ship->parts[part_id];
        vec3 part_position = vec_add(part->offset, ship->position);
        
        for (int i = 0; i < array_count(cube_collision_quads); i++) {
            collision_quad quad = cube_collision_quads[i];
            
            quad.a = vec_add(quad.a, part_position);
            quad.b = vec_add(quad.b, part_position);
            
            float d = intersect_ray_quad(global->camera.position, ray, quad);
                        
            if (d >= 0 && d < min_distance) { 
                min_distance = d;
                hit_quad = quad;
                closest = part;
            }
        }
    }
    
    if (closest) {
        vec3 quad_offset = vec_sub(collision_quad_get_center(hit_quad), vec_add(closest->offset, ship->position));
        vec3 offset = vec_add(closest->offset, vec_mul(quad_offset, 2.f)); 
        
        vec3 position = vec_add(ship->position, offset);
        
        quat current_part_rotation_x = quat_from_axis_angle(vec3(1, 0, 0), DEG_TO_RAD(90 * rotate_x));
        quat current_part_rotation_y = quat_from_axis_angle(vec3(0, 1, 0), DEG_TO_RAD(90 * rotate_y));
        
        quat current_part_rotation = quat_mul_quat(current_part_rotation_y, current_part_rotation_x);
        
        render_mesh_basic(part_types[type_id].mesh, .translation = position, .rotation = current_part_rotation,
            .color = (color)RGBA(200, 100, 100, 100));
        
        if (global->mouse.left_down_this_frame) {
            ship_add_part(ship, position, current_part_rotation, type_id);
            current_part_rotation = unit_quat();
        }
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
}

