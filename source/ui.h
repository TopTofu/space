#pragma once

typedef struct {
    bool left_down, left_up;
    bool right_down, right_up;
    bool middle_down, middle_up;

    void* active;
    void* hot, *hot_to_be;
    
    int drag_x, drag_y;
    
    text_input text_input;
} ui_state;

#define in_rect(X, Y, W, H) (global->mouse.x >= (X) &&          \
                             global->mouse.x <= ((X) + (W)) &&  \
                             global->mouse.y >= (Y) &&          \
                             global->mouse.y <= ((Y) + (H)))

#define any_active()    (global->ui.active != 0)
#define is_active(id)   (global->ui.active == (id))
#define is_hot(id)      (global->ui.hot == (id))

#define set_active(id)  (global->ui.active = (id))
#define set_hot(id)     (global->ui.hot_to_be = (id))

#define ID2(id)     ((char*)(id) + 1)
#define ID3(id)     ((char*)(id) + 2)
#define ID4(id)     ((char*)(id) + 3)
#define IDX(id, x)  ((char*)(id) + (x))
