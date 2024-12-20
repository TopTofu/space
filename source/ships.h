#pragma once

#define get_type(part) (part_types[(part)->type_id])
#define SHIP_PART_MAX_COUNT 128

typedef enum {
    PART_CUBE,
    PART_THRUSTER,
    PART_TANK,
    PART_SLOPE,
    PART_WING,
    PART_WING_TIP,
    PART_CONNECTOR,
    PART_QUARTER_TUBE,
    PART_FIN,
    PART_CORNER,
    
    PART_TYPE_COUNT
} ship_part_type_id;

typedef struct {
    vec3 a, b;
} collision_quad;

typedef struct {
    ship_part_type_id id;
    mesh mesh;
} ship_part_type;

typedef struct {
    ship_part_type_id type_id;
    bool active;
    
    vec3 offset;
    quat rotation;
} ship_part;

typedef struct {
    ship_part parts[SHIP_PART_MAX_COUNT];
    
    vec3 position;
} ship_info;
ship_info ship = { 0 };

ship_part_type part_types[PART_TYPE_COUNT];




