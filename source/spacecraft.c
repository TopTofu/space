#pragma 

#define SHIP_COMPONENT_MAX_CONNECTIONS 6
#define SHIP_MAX_COMPONENTS 32

typedef struct ship_component ship_component;
typedef struct component_connection component_connection;

typedef struct component_connection {
    bool valid;
    vec3 offset;

    collision_quad quad;

    ship_component* component;
    component_connection* connected_to;
} component_connection;

typedef struct ship_component {
    mesh mesh;
    vec3 translation;
    
    component_connection connections[SHIP_COMPONENT_MAX_CONNECTIONS];
} ship_component;

typedef struct {
    int component_count;
    ship_component components[SHIP_MAX_COMPONENTS];
    
    vec3 translation;
    
    ship_component* base_component;
} ship_info;


ship_info ship;

static inline vec3 connection_get_world_position(component_connection* con) {
    return vec_add(vec_add(ship.translation, con->component->translation), con->offset);
}
static inline vec3 component_get_world_position(ship_component* comp) {
    return vec_add(ship.translation, comp->translation);
}

static void render_ship_component(ship_component* comp, vec3 translation) {
    render_mesh_basic(comp->mesh, .translation = vec_add(translation, comp->translation), .color = (color)RGB_GRAY(200));
}

static void render_ship(ship_info ship) {
    for (int i = 0; i < ship.component_count; i++) {
        render_ship_component(&ship.components[i], ship.translation);
    }
}

static void make_ship_component(ship_component* comp) {
    comp->mesh = global->renderer.cube_mesh;
    comp->translation = vec3(0, 0, 0);
    
    vec3 cube_connection_offsets[SHIP_COMPONENT_MAX_CONNECTIONS] = {
        vec3(0.5, 0, 0), vec3(0, 0.5, 0), vec3(0, 0, 0.5), 
        vec3(-.5, 0, 0), vec3(0, -.5, 0), vec3(0, 0, -.5)
    };
    
    collision_quad cube_connection_quads[SHIP_COMPONENT_MAX_CONNECTIONS] = {
        { .a = vec3(0.5, -.5, -.5), .b = vec3(0.5, 0.5, 0.5) },
        { .a = vec3(-.5, 0.5, -.5), .b = vec3(0.5, 0.5, 0.5) },
        { .a = vec3(-.5, -.5, 0.5), .b = vec3(0.5, 0.5, 0.5) },
        
        { .a = vec3(-.5, -.5, 0.5), .b = vec3(-.5, 0.5, -.5) },
        { .a = vec3(-.5, -.5, -.5), .b = vec3(0.5, -.5, 0.5) },
        { .a = vec3(-.5, -.5, -.5), .b = vec3(0.5, 0.5, -.5) },
    };
    
    for (int i = 0; i < SHIP_COMPONENT_MAX_CONNECTIONS; i++) {
        comp->connections[i].valid = true;
        comp->connections[i].offset = cube_connection_offsets[i];
        comp->connections[i].component = comp;
        comp->connections[i].connected_to = 0;
        comp->connections[i].quad = cube_connection_quads[i];
    }
} 

static void ship_add_component(ship_info* ship, int connection_index, component_connection* connect_to) {
    if (connect_to->connected_to) { return; }
    if (ship->component_count >= SHIP_COMPONENT_MAX_CONNECTIONS) { return; }
    
    ship_component* comp = &ship->components[ship->component_count++];
    make_ship_component(comp);
    
    ship_component* parent = connect_to->component;
    comp->translation = vec_add(parent->translation, vec_add(
        vec_mul(comp->connections[connection_index].offset, -1.f), connect_to->offset));

    connect_to->connected_to = &comp->connections[connection_index];
    comp->connections[connection_index].connected_to = connect_to;
}

void component_at_cursor() {
    vec3 ray = ray_from_screen(global->mouse.position);
    
    float min_distance = 100;
    component_connection* closest = 0;
    for (int ship_id = 0; ship_id < ship.component_count; ship_id++) {
        ship_component* comp = &ship.components[ship_id];
        
        // for (int i = 0; i < 1; i++) {
        for (int i = 0; i < SHIP_COMPONENT_MAX_CONNECTIONS; i++) {
            component_connection* con = &comp->connections[i];
            if (con->connected_to) { continue; }
            
            vec3 total_translation = connection_get_world_position(con);
            
            collision_quad quad = con->quad;
            quad.a = vec_add(quad.a, component_get_world_position(comp));
            quad.b = vec_add(quad.b, component_get_world_position(comp));
            
            float d = intersect_ray_quad(global->camera.position, ray, quad);
            
            if (d >= 0 && d < min_distance) { 
                min_distance = d;
                closest = con;
            }
            
            // render_mesh_basic(global->renderer.cube_mesh, .scale = 0.1, .translation = quad.a, 
            //     .color = (color)RGB(200, 200, 100));
            // render_mesh_basic(global->renderer.cube_mesh, .scale = 0.1, .translation = quad.b, 
            //     .color = (color)RGB(200, 100, 200));
                
            // render_mesh_basic(global->renderer.cube_mesh, .scale = 0.05, .translation = total_translation, 
            //     .color = (color)RGB(100, 100, 200));
        }
    }
    
    // print(min_distance);
    
    
    if (closest) {
        vec3 t = connection_get_world_position(closest);
        render_mesh_basic(global->renderer.cube_mesh, .scale = 0.1, .translation = t, .color = (color)RGB(200, 100, 100));
    }
}

