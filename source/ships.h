#pragma once

#define get_type(part) (part_types[(part)->type_id])
#define SHIP_PART_MAX_COUNT 64

typedef enum {
    PART_CUBE,
    PART_THRUSTER,
    PART_TANK,
    
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
    
    vec3 offset;
    quat rotation;
} ship_part;

typedef struct {
    int part_count;
    ship_part parts[SHIP_PART_MAX_COUNT];
    
    vec3 position;
} ship_info;
ship_info ship;

ship_part_type part_types[PART_TYPE_COUNT];




