#pragma once

void clear_ui() {
    global->ui.left_down = global->ui.left_up = false;
    global->ui.right_down = global->ui.right_up = false;
    global->ui.middle_down = global->ui.middle_up = false;   
}

void clear_active() {
    global->ui.active = 0;
    clear_ui();
}

void ui_frame_begin(game_state* state) {
    state->ui.left_down = state->mouse.left_down_this_frame;
    state->ui.left_up = state->mouse.left_up_this_frame;
    state->ui.right_down = state->mouse.right_down_this_frame;
    state->ui.right_up = state->mouse.right_up_this_frame;
    state->ui.middle_down = state->mouse.middle_down_this_frame;
    state->ui.middle_up = state->mouse.middle_up_this_frame;

    state->ui.hot = state->ui.hot_to_be;
    state->ui.hot_to_be = 0;
}

bool button_logic(void* id, bool over) {
    bool result = false;
    if (!any_active()) {
        if (over) { set_hot(id); }
        if (is_hot(id) && global->ui.left_down) { set_active(id); }
    }

    if (is_active(id)) {
        if (over) { set_hot(id); }
        if (global->ui.left_up) {
            if (is_hot(id)) { result = true; }
            clear_active();
        }
    }

    return result;
}

bool button_logic_down(void* id, bool over) {
    bool result = false;

    if (!any_active()) {
        if (over) { set_hot(id); }
        if (is_hot(id) && global->ui.left_down) { 
            set_active(id);
            result = true;
        }
    }

    if (is_active(id)) {
        if (over) { set_hot(id); }
        if (global->ui.left_up) { clear_active(); }
    }

    return result;
}

bool button(void* id, int x, int y, int w, int h, color c) {
    bool over = in_rect(x, y, w, h);
    bool result = button_logic(id, over); 

    if (over) { c = lighten(c); }
    ui_quad(x, y, w, h, c);

    return result;
}
