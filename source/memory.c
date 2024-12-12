#pragma once

void init_arena(memory_arena* arena, u64 size, void* base) {
    arena->base = base;
    arena->size = size;
    arena->used = 0;
    arena->_saved_count = 0;
}

void clear_arena(memory_arena* arena) {
    arena->used = 0;
    arena->_saved_count = 0;
}

void save_arena(memory_arena* arena) {
    assert(arena->_saved_count < array_count(arena->_saved));
    arena->_saved[arena->_saved_count++] = arena->used;
}

void restore_arena(memory_arena* arena) {
    assert(arena->_saved_count > 0);
    arena->used = arena->_saved[--arena->_saved_count];
}

void* push_size(memory_arena* arena, u64 size) {
    assert(arena->used + size <= arena->size);
    void* result = (u8*)arena->base + arena->used;
    arena->used += size;
    return result;
}

