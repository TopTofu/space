#pragma 

static inline vec3 slot_get_offset(component_slot_info* slot) {
    return quat_rotate_vec(slot->component->rotation, get_slot_data(slot).offset);
}

static inline vec3 slot_get_world_position(component_slot_info* slot) {
    return vec_add(vec_add(ship.translation, slot->component->offset), slot_get_offset(slot));
}

static inline vec3 component_get_world_position(ship_component* comp) {
    return vec_add(ship.translation, comp->offset);
}

static void render_ship_component(ship_component* comp, vec3 translation) {
    quat rot = quat_from_axis_angle(vec3(0, 1, 0), global->time.in_seconds);
    
    render_mesh_basic(get_type(comp).mesh, 
        .translation = vec_add(translation, comp->offset), 
        .rotation = comp->rotation, 
        .color = (color)RGB_GRAY(200));
}

static void render_ship(ship_info ship) {
    for (int i = 0; i < ship.component_count; i++) {
        render_ship_component(&ship.components[i], ship.translation);
    }
}

static void make_ship_component(ship_component* comp, int type_id) {
    comp->offset = vec3(0, 0, 0);
    
    comp->type_id = type_id;
    comp->rotation = unit_quat();
    
    ship_component_type_info type = get_type(comp);
    
    for (int i = 0; i < SHIP_COMPONENT_MAX_SLOTS; i++) {
        comp->slots[i].component = comp;
        comp->slots[i].connected_to = 0;
        comp->slots[i].index = i;
        
        comp->slots[i].valid = (i < type.slot_count);
    }
}

static void ship_add_component(ship_info* ship, component_slot_info* connect_to, int type_id) {
    if (connect_to->connected_to) { return; }
    if (ship->component_count >= SHIP_MAX_COMPONENTS) { return; }
    
    ship_component* comp = &ship->components[ship->component_count++];
    make_ship_component(comp, type_id);
    
    ship_component_type_info type = component_types[type_id];
    
    vec3 offset = slot_get_offset(connect_to);
    vec3 inv_offset_dir = vec_mul(vec_norm(offset), -1);
    
    int slot_index = -1;
    for (int i = 0; i < type.slot_count; i++) {
        if (!comp->slots[i].valid) { continue; }
    
        vec3 v = get_type(comp).slot_data[i].offset;
        v = vec_norm(v);
        
        if (vec_eq(inv_offset_dir, v)) {
            slot_index = i;
            break;
        }
    }
    
    if (slot_index < 0) {
        // @Note: if we don't find an appropriately turned slot, we take slot[0] and rotate the component
        vec3 v = get_type(comp).slot_data[0].offset;
        v = vec_norm(v);
        comp->rotation = quat_from_unit_vectors(v, inv_offset_dir);
        
        vec3 temp = quat_rotate_vec(comp->rotation, v);
        slot_index = 0;
    }
    
    ship_component* parent = connect_to->component;
    
    vec3 own_slot_offset = get_type(comp).slot_data[slot_index].offset;
    own_slot_offset = quat_rotate_vec(comp->rotation, own_slot_offset);
    
    vec3 to_connect_offset = get_slot_data(connect_to).offset;
    to_connect_offset = quat_rotate_vec(connect_to->component->rotation, to_connect_offset);
    
    comp->offset = vec_add(parent->offset, vec_add(vec_mul(own_slot_offset, -1.f), to_connect_offset));

    connect_to->connected_to = &comp->slots[slot_index];
    comp->slots[slot_index].connected_to = connect_to;
}

void component_place_preview(ship_component_type_id type_id) {
    vec3 ray = ray_from_screen(global->mouse.position);
    
    float min_distance = 100;
    component_slot_info* closest = 0;
    for (int ship_id = 0; ship_id < ship.component_count; ship_id++) {
        ship_component* comp = &ship.components[ship_id];
        
        for (int i = 0; i < SHIP_COMPONENT_MAX_SLOTS; i++) {
            component_slot_info* slot = &comp->slots[i];
            if (slot->connected_to) { continue; }
            if (!slot->valid) { continue; }
            
            vec3 total_translation = slot_get_world_position(slot);
            
            collision_quad quad = get_slot_data(slot).quad;
            quad.a = vec_add(quad.a, component_get_world_position(comp));
            quad.b = vec_add(quad.b, component_get_world_position(comp));
            
            float d = intersect_ray_quad(global->camera.position, ray, quad);
            
            if (d >= 0 && d < min_distance) { 
                min_distance = d;
                closest = slot;
            }
        }
    }
    
    if (closest) {
        vec3 t = slot_get_world_position(closest);
        vec3 offset = get_slot_data(closest).offset;
        
        render_mesh_basic(component_types[type_id].mesh, .translation = vec_add(t, offset), 
            .color = (color)RGBA(200, 100, 100, 100));
            
        if (global->mouse.left_down_this_frame) {
            ship_add_component(&ship, closest, type_id); 
        }
    }
}

static void init_ship_component_types(game_state* state) {
    component_types[COMPONENT_CUBE] = (ship_component_type_info) {
        .id = COMPONENT_CUBE,
        .mesh = make_cube_mesh(),

        .slot_data[0].offset = vec3(0.5, 0, 0), 
        .slot_data[1].offset = vec3(0, 0.5, 0), 
        .slot_data[2].offset = vec3(0, 0, 0.5), 
        .slot_data[3].offset = vec3(-.5, 0, 0), 
        .slot_data[4].offset = vec3(0, -.5, 0), 
        .slot_data[5].offset = vec3(0, 0, -.5),

        .slot_data[0].quad = (collision_quad) { .a = vec3(0.5, -.5, -.5), .b = vec3(0.5, 0.5, 0.5) },
        .slot_data[1].quad = (collision_quad) { .a = vec3(-.5, 0.5, -.5), .b = vec3(0.5, 0.5, 0.5) },
        .slot_data[2].quad = (collision_quad) { .a = vec3(-.5, -.5, 0.5), .b = vec3(0.5, 0.5, 0.5) },
        .slot_data[3].quad = (collision_quad) { .a = vec3(-.5, -.5, 0.5), .b = vec3(-.5, 0.5, -.5) },
        .slot_data[4].quad = (collision_quad) { .a = vec3(-.5, -.5, -.5), .b = vec3(0.5, -.5, 0.5) },
        .slot_data[5].quad = (collision_quad) { .a = vec3(-.5, -.5, -.5), .b = vec3(0.5, 0.5, -.5) },
        
        .slot_count = 6,
    };
    
    component_types[COMPONENT_THRUSTER] = (ship_component_type_info) {
        .id = COMPONENT_THRUSTER,
        .mesh = make_thruster_mesh(),

        .slot_data[0].offset = vec3(0, -.5, 0), 
        .slot_data[1].offset = vec3(0, .5, 0), 

        .slot_data[0].quad = (collision_quad) { .a = vec3(-.5, -.5, -.5), .b = vec3(0.5, -.5, 0.5) },
        .slot_data[1].quad = (collision_quad) { .a = vec3(-.5, .5, -.5), .b = vec3(0.5, .5, 0.5) },

        .slot_count = 2,
    };
}
