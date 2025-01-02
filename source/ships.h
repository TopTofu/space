#pragma once

#define get_type(part) (part_types[(part)->type_id])
#define SHIP_PART_MAX_COUNT 512

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
    PART_STEP,
    PART_DRILL,
    
    PART_GRABBLER,
    PART_ATTACHMENT,
    
    PART_GRABBER,
    
    PART_ROUND_PLATE,
    PART_TRI_PLATE,
    PART_RIGHT_TRI_PLATE,
    PART_SLANT,
    
    PART_BOARD,
    
    PART_TANK_TURN,
    PART_TANK_TURN2,
    PART_TANK_TURN3,
    
    PART_QUARTER_TUBE_TURN,
        
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
    int part_count;
    ship_part parts[SHIP_PART_MAX_COUNT];
    
    vec3 position;
    vec3 target_position;
    float pos_t;
} ship_info;
ship_info ship = { 0 };

ship_part_type part_types[PART_TYPE_COUNT];

typedef struct {
    int id;
    char* path;
    bool used;
} ship_save_slot;

#define MAX_SHIP_SAVE_SLOTS 8
typedef struct {
    bool is_open;
    ship_save_slot slots[MAX_SHIP_SAVE_SLOTS];
    ship_save_slot* current_slot;
    
    float open_t;
    float target_t;
} ship_saves_interface_info;

