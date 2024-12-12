#pragma once

#define KEY_INPUT_PROC(name) bool name(game_state* state, key_event event)
typedef KEY_INPUT_PROC(key_input_proc);

#define is_printable(c) ((c) > 31 && (c) < 127)

typedef struct {
    string buffer;
    u8* cursor;
} text_input;

void keymap_set(game_state* state, u32 key, bool on);
bool keymap_get(game_state* state, u32 key);

void bind_key_input_proc(key_input_proc* proc);
void remove_key_input_proc(key_input_proc* proc);


text_input* text_input_bind(text_input* new);
void text_input_remove(text_input* input);

void text_input_paste_string(text_input* input, string str);
bool text_input_type_char(text_input* input, u8 c);
bool text_input_maybe_handle_control(text_input* input, key_event event); 
void text_input_clear(text_input* input);
text_input push_text_input(memory_arena* arena, u32 size);

static inline void text_input_move_cursor_right(text_input* input);
static inline void text_input_move_cursor_left(text_input* input);
void text_input_delete_left(text_input* input);
void text_input_delete_right(text_input* input);

bool key_code_is_text_input(u32 code);