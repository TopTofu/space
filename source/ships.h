#pragma once

#define SHIP_COMPONENT_MAX_SLOTS 6
#define SHIP_MAX_COMPONENTS 32

typedef struct ship_component ship_component;
typedef struct component_slot_info component_slot_info;

typedef enum {
    COMPONENT_CUBE,
    COMPONENT_THRUSTER,
    COMPONENT_TANK,
    
    COMPONENT_TYPE_COUNT
} ship_component_type_id;

typedef struct {
    vec3 a, b;
} collision_quad;

typedef struct component_slot_info {
    bool valid;
    int index;

    ship_component* component;
    component_slot_info* connected_to;
} component_slot_info;

typedef struct ship_component {
    ship_component_type_id type_id;
    vec3 offset;
    quat rotation;
    
    component_slot_info slots[SHIP_COMPONENT_MAX_SLOTS];
} ship_component;

typedef struct {
    int component_count;
    ship_component components[SHIP_MAX_COMPONENTS];
    
    vec3 translation;
    
    ship_component* base_component;
} ship_info;
ship_info ship;

typedef struct {
    vec3 offset;
    collision_quad quad;
} component_slot_data;

typedef struct {
    ship_component_type_id id;
    int slot_count;
    
    component_slot_data slot_data[SHIP_COMPONENT_MAX_SLOTS];
    mesh mesh;
} ship_component_type_info;
ship_component_type_info component_types[COMPONENT_TYPE_COUNT];


#define get_type(comp) (component_types[(comp)->type_id])
#define get_slot_data(slot) (get_type((slot)->component).slot_data[(slot)->index])
