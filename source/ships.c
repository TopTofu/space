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

static inline void ship_clear(ship_info* ship) {
    ship->part_count = 0;
    memset(ship->parts, 0, sizeof(ship->parts));
}

static void save_ship(game_state* state, ship_info* ship) {
    ship_save_slot* slot = state->saves.current_slot;
    FILE* file = fopen(slot->path, "wb");
    if (!file) { 
        report("Could not open save file at %s for saving of slot %i\n", slot->path, slot->id); 
        return; 
    }    

    fwrite(ship, sizeof(*ship), 1, file);
    fclose(file);
    
    report("Saved ship to slot %i\n", slot->id);
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
    
    save_ship(global, ship); 
}

static void load_ship(game_state* state, ship_info* ship) {
    ship_save_slot* slot = state->saves.current_slot;
    if (!slot->used) {
        ship_clear(ship);
        
        save_ship(state, ship);
        slot->used = true;
        
        ship_add_part(ship, ship->position, unit_quat(), PART_CUBE);
        
        report("Created file for slot %i\n", slot->id);
    } else {
        FILE* file = fopen(slot->path, "rb");
        if (!file) {
            report("Could not open save file %s for loading of slot %i\n", slot->path, slot->id);
            return;
        }
        
        fread(ship, sizeof(*ship), 1, file);
        fclose(file);
        
        report("Loaded ship from slot %i\n", slot->id);
    }
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
    
    save_ship(global, &ship);
}

static void pick_part_type_at_mouse() {
    part_at_mouse_result get_result = get_part_at_mouse(&ship);
    
    if (get_result.part) {
        global->current_part_type_id = get_result.part->type_id;
    }
}


static void init_ship_save_slots(game_state* state) {
    ship_saves_interface_info* saves = &state->saves;
    
    string buffer = string_buffer(32);
    for (int i = 0; i < MAX_SHIP_SAVE_SLOTS; i++) {
        string_write(&buffer, "../saves/ship");        
        string_write(&buffer, i);
        
        saves->slots[i].id = i;  
        saves->slots[i].path = to_c_str(buffer, push_permanent);
        
        FILE* file = fopen(saves->slots[i].path, "rb");
        saves->slots[i].used = (file != 0);
        
        if (file) { fclose(file); }
        
        string_clear(&buffer);
    }
    
    saves->current_slot = &saves->slots[0];
}

static void update_and_render_ship_saves_interface(game_state* state) {
    ship_saves_interface_info* saves = &state->saves;
    
    int pad = 20;
    int x = pad;
    int y = pad;
    
    int button_w = 80;
    
    void* id = saves;
    bool clicked = button(id, x, y, button_w, button_w, (color)RGB(230, 238, 156));
    if (clicked) {
        toggle(saves->is_open);  
        saves->target_t = saves->is_open ? 1. : 0.;
    }
    
    float speed = 2.0;
    
    if (saves->open_t < saves->target_t) {
        saves->open_t += speed * state->time.dt;
        saves->open_t = MIN(saves->open_t, saves->target_t);
    } else if (saves->open_t > saves->target_t) {
        saves->open_t -= speed * state->time.dt;
        saves->open_t = MAX(saves->open_t, saves->target_t);
    }
    
    if (saves->open_t <= 0.) { return; }
    
    int slot_w = 50;
    
    int closed_y = y;
    int open_y = (slot_w + pad) * (MAX_SHIP_SAVE_SLOTS - 1) + closed_y + button_w + pad;
    
    int slot_x = pad;
    
    float eased_t = ease_out_circ(saves->open_t);
    int slot_y = LERP(closed_y, open_y, eased_t);
    
    scissor(slot_x - pad, open_y + slot_w + pad, slot_w + pad * 2, open_y - closed_y - pad);
    
    for (int i = 0; i < MAX_SHIP_SAVE_SLOTS; i++) {
        ship_save_slot* slot = &saves->slots[i];
        
        if (saves->open_t >= 1.) {
            bool clicked = button(IDX(id, i + 1), slot_x, slot_y, slot_w, slot_w, (color)RGB_GRAY(255));
            
            if (clicked) {
                saves->current_slot = slot;
                
                load_ship(state, &ship);
            }
        }
        
        if (saves->current_slot == slot) {
            ui_quad(slot_x - 3, slot_y - 3, slot_w + 6, slot_w + 6, (color)RGB_GRAY(255));
        } 
        
        ui_quad(slot_x, slot_y, slot_w, slot_w, (color)RGB_GRAY(100));
        
        string buffer = string_buffer(8);
        string_write(&buffer, i);
        render_text(buffer, slot_x, slot_y, .height = 32, .color = RGB(57, 255, 20));
        
        slot_y -= slot_w + pad;
    }
    
    scissor_reset();
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
    
    // part_types[PART_WING_TIP] = (ship_part_type) {
    //     .id = PART_WING_TIP,
    //     .mesh = make_wing_tip_mesh(),
    // };
    
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
    
    part_types[PART_DRILL] = (ship_part_type) {
        .id = PART_DRILL,
        .mesh = load_obj("../data/drill.obj"),
    };
    
    part_types[PART_GRABBLER] = (ship_part_type) {
        .id = PART_GRABBLER,
        .mesh = load_obj("../data/grabbler.obj"),
    };
    
    part_types[PART_ATTACHMENT] = (ship_part_type) {
        .id = PART_ATTACHMENT,
        .mesh = load_obj("../data/attachment.obj"),
    };
}

