#pragma once 

void keymap_set(game_state* state, u32 key, bool on) {
    u8 value = state->keymap[key / 8];
    
    // @Info: turn off the bit
    u8 mask = 0xFF & ~(1 << (key % 8)); 
    value &= mask;

    // @Info: if on is true, turn on the bit
    value |= on << (key % 8);
    
    state->keymap[key / 8] = value;
}

inline bool keymap_get(game_state* state, u32 key) {
    u8 mask = 0xFF & (1 << (key % 8)); 
    return (state->keymap[key / 8]) & mask;
}

bool default_key_down_proc(game_state* state, key_event event) {
    bool ctrl = keymap_get(state, KEY_CONTROL);
    bool shift = keymap_get(state, KEY_SHIFT);
    bool alt = keymap_get(state, KEY_ALT);
    
    switch (event.code) {
        case KEY_MOUSE_L: {
            state->mouse.left_down_this_frame = true;
        } break;
        case KEY_MOUSE_R: {
            state->mouse.right_down_this_frame = true;
        } break;
        case KEY_MOUSE_M: {
            state->mouse.middle_down_this_frame = true;
        } break;
    }
    
    // if (event.code == state->camera.key_left) { state->camera.left = true; } 
    // if (event.code == state->camera.key_right) { state->camera.right = true; } 
    // if (event.code == state->camera.key_up) { state->camera.up = true; } 
    // if (event.code == state->camera.key_down) { state->camera.down = true; } 
    
    return true;
}

bool default_key_up_proc(game_state* state, key_event event) {
    switch (event.code) {
        case KEY_MOUSE_L: {
            state->mouse.left_up_this_frame = true;
        } break;
        case KEY_MOUSE_R: {
            state->mouse.right_up_this_frame = true;
        } break;
        case KEY_MOUSE_M: {
            state->mouse.middle_up_this_frame = true;
        } break;
    }
    
    // if (event.code == state->camera.key_left)  { state->camera.left = false; } 
    // if (event.code == state->camera.key_right) { state->camera.right = false; } 
    // if (event.code == state->camera.key_up)    { state->camera.up = false; } 
    // if (event.code == state->camera.key_down)  { state->camera.down = false; } 
    
    return true;
}

void clear_mouse(game_state* state) {
    state->mouse.scroll = 0;
    
    state->mouse.left_down_this_frame = false;
    state->mouse.right_down_this_frame = false;
    state->mouse.middle_down_this_frame = false;
    
    state->mouse.left_up_this_frame = false;
    state->mouse.right_up_this_frame = false;
    state->mouse.middle_up_this_frame = false;
}

void process_input(game_state* state) {
    platform_info* platform = state->platform;
    
    clear_mouse(state);
    
    for (int i = 0; i < platform->event_count; i++) {
        event_info event = platform->events[i];
        
        switch (event.type) {
            case KEY_INPUT_EVENT: {
                u32 code = event.key_event.code;
                
                keymap_set(state, code, event.key_event.is_down);
                
                // @Info: most keystrokes are sent as both key_input aswell as text_input events
                //        (in win32 atleast). If we are currently typing, meaning a text_input is 
                //        bound and the key code can also be typed, we do not handle this event.
                //        Its corresponding text_input event will be handled instead.
                if (state->current_text_input) {
                    if (key_code_is_text_input(code)) { break; }
                    
                    bool handled = text_input_maybe_handle_control(state->current_text_input, event.key_event);
                    if (handled) { break; }
                }
                
                // @Info: First the event gets passed to current input handling proc.
                //        Its return value tells us if the event has been handled or not.
                //        If it was not handled, the event is then passed to either of the
                //        the default handlers.
                //        This gives the handling procedure the ability to block all other controls
                //        by always returning true. The consoles input handler does this for example.
                bool handled = false;
                if (state->current_input_proc) {
                    handled = state->current_input_proc(state, event.key_event);
                }                
                
                if (handled) { break; }
                
                if (event.key_event.is_down) {
                    default_key_down_proc(state, event.key_event);
                } else {
                    default_key_up_proc(state, event.key_event);
                }
            } break;
            case TEXT_INPUT_EVENT: {
                if (!state->current_text_input) { break; }
                text_input_type_char(state->current_text_input, event.text_input_event.c);
            } break;
            case MOUSE_MOVE_EVENT: {
                state->mouse.x = event.mouse_move_event.x;
                state->mouse.y = event.mouse_move_event.y;
            } break;
            case MOUSE_WHEEL_EVENT: {
                state->mouse.scroll += event.mouse_wheel_event.scroll;
            } break;
        }
    }
}


// === TEXT INPUT
void bind_key_input_proc(key_input_proc* proc) {
    global->current_input_proc = proc;
}

void remove_key_input_proc(key_input_proc* proc) {
    assert(global->current_input_proc == proc);
    global->current_input_proc = 0;
}

text_input push_text_input(memory_arena* arena, u32 size) {
    text_input result;
    result.buffer = push_string(arena, size); 
    result.cursor = result.buffer.data;
    
    return result;
}

void text_input_clear(text_input* input) {
    input->buffer.length = 0;
    input->cursor = input->buffer.data;
}

void text_input_paste_string(text_input* input, string str) {
    string_copy(&input->buffer, str);
    input->cursor = input->buffer.data + str.length;
}

static inline bool text_input_is_full(text_input* input) {
    return (input->buffer.length >= input->buffer.size);
}

static inline bool text_input_cursor_at_end(text_input* input) {
    return (input->cursor == input->buffer.data + input->buffer.length);
}

bool text_input_type_char(text_input* input, u8 c) {
    if (text_input_is_full(input)) { return false; }
    if (!is_printable(c))          { return false; }
    
    // @Info: if the cursor is not at the end of the text
    //        we need to push every after the cursor back
    //        before we can insert the character
    if (!text_input_cursor_at_end(input)) {
        for (int i = input->buffer.length + 1; i >= input->cursor - input->buffer.data; i--) {
            input->buffer.data[i + 1] = input->buffer.data[i];
        }
    }
    
    *(input->cursor) = c;
    input->buffer.length++;
    input->cursor++;
    
    return true;
}

static inline int text_input_offset(text_input* input) {
    return input->cursor - input->buffer.data;
}

static inline void text_input_move_cursor_right(text_input* input) {
    if (!text_input_cursor_at_end(input)) { input->cursor++; }
}

static inline void text_input_move_cursor_left(text_input* input) {
    if (input->cursor > input->buffer.data) { input->cursor--; }
}

void text_input_delete_at_index(text_input* input, int pos, int count) {
    for (int i = pos; i < input->buffer.length; i++) {
        input->buffer.data[i - count] = input->buffer.data[i];
    }

    input->buffer.length -= count;
}

void text_input_delete_left(text_input* input) {
    if (input->buffer.length == 0) { return; }
    
    int cursor_pos = input->cursor - input->buffer.data;
    if (cursor_pos == 0) { return; }

    text_input_delete_at_index(input, cursor_pos, 1);
    input->cursor--;
}

void text_input_delete_right(text_input* input) {
    if (text_input_cursor_at_end(input)) { return; }
    
    int cursor_pos = input->cursor - input->buffer.data;
    text_input_delete_at_index(input, cursor_pos + 1, 1);
}

text_input* text_input_bind(text_input* new) {
    text_input* old = global->current_text_input;
    global->current_text_input = new;
    return old;
}

void text_input_remove(text_input* input) {
    assert(input == global->current_text_input);
    global->current_text_input = 0;
}

bool text_input_maybe_handle_control(text_input* input, key_event event) {
    if (!event.is_down)  { return false; }
    
    switch(event.code) {
        case KEY_BACKSPACE: {
            text_input_delete_left(input);
            return true;
        } break;
        case KEY_LEFT: {
            text_input_move_cursor_left(input);
            return true;
        } break;
        case KEY_RIGHT: {
            text_input_move_cursor_right(input);
            return true;
        } break;
    }
    
    return false;
}

bool key_code_is_text_input(u32 code) {
    switch(code) {
        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
        case KEY_A:
        case KEY_B:
        case KEY_C:
        case KEY_D:
        case KEY_E:
        case KEY_F:
        case KEY_G:
        case KEY_H:
        case KEY_I:
        case KEY_J:
        case KEY_K:
        case KEY_L:
        case KEY_M:
        case KEY_N:
        case KEY_O:
        case KEY_P:
        case KEY_Q:
        case KEY_R:
        case KEY_S:
        case KEY_T:
        case KEY_U:
        case KEY_V:
        case KEY_W:
        case KEY_X:
        case KEY_Y:
        case KEY_Z:
        case KEY_SPACE:
        case KEY_SEMICOLON:
        case KEY_PLUS:
        case KEY_COMMA:
        case KEY_MINUS:
        case KEY_PERIOD:
        case KEY_SLASH:
        case KEY_GRAVE_ACCENT:
        case KEY_BACKSLASH:
        case KEY_LEFT_BRACKET:
        case KEY_RIGHT_BRACKET:
        case KEY_APOSTROPHE:
        { return true; } break;
    }
    return false;
}


