#pragma once

void init_font(game_state* state) {
    font_info result = { 0 };
    
    result.texture = get_texture("mono_font");
    
    result.x_advance = 6;
    result.glyph_width = 8;
    result.glyph_height = 8;
    result.glyph_count = result.texture->w / result.glyph_width;
    
    state->font = result;
}

static inline float get_font_scale_for_pixel_height(int height) {
    return height / (float)global->font.glyph_height;
}

static inline int get_text_width_single_line(string text, int height) {
    float scale = get_font_scale_for_pixel_height(height);
    return scale * text.length * global->font.x_advance;
}

static ivec2 font_get_rendered_text_dimensions(string text, int height) {
    float scale = get_font_scale_for_pixel_height(height);
    ivec2 result = { .w = 0, .h = height };
    
    int current_line_w = 0;
    int advance = global->font.x_advance * scale;
    
    for (int i = 0; i < text.length; i++) {
        if (text.data[i] == '\n') { 
            result.h += height;
            current_line_w = 0;
        } else {
            current_line_w += advance;
            if (current_line_w > result.w) { 
                result.w = current_line_w;
            }
        }
    }    

    return result;
}

typedef struct {
    int count;
    int height;

    color color;
    
    u32 break_after_width;
    
    u32 clamp_after_width;
    u32 clamp_after_height;
} render_text_args;
#define default_render_args \
    .height = 8,\
    .color = RGB(0, 0, 0),\
    .break_after_width = -1,\
    .clamp_after_width = -1,\
    .clamp_after_height = -1

#define render_text(TEXT, X, Y, ...) _render_text(TEXT, X, Y,\
    (render_text_args){ .count = (TEXT).length, default_render_args, __VA_ARGS__ })

// @Info: this returns the position where the text stops
vec2 _render_text(string text, int x, int y, render_text_args args) {
    font_info* font = &global->font;
    shader_info* shader = get_shader("font_glyph");
    
    float window_w = global->platform->window_width;
    float window_h = global->platform->window_height;
    
    float scale = get_font_scale_for_pixel_height(args.height);
    float scaled_width = font->glyph_width * scale;
    
    glUseProgram(shader->id);
    shader_bind_texture(shader, font->texture, "font", 0);
    
    float u_span = 1.f / (float)font->glyph_count;
    shader_set_uniform(shader, "u_span", u_span);
    
    shader_set_uniform(shader, "color", args.color);
    shader_set_uniform(shader, "scale", vec2(scaled_width / window_w, args.height / window_h));
    
    glBindVertexArray(global->renderer.quad_mesh.vao);
    
    // @Info: This is needed since the base quad is centered at (x, y)
    x += scaled_width / 2.f;
    y += font->glyph_height / 2.f * scale;
    
    int cur_x = x;
    int cur_width = 0;
    int cur_height = 0;
    for (int i = 0; i < args.count; i++) {
        char c = text.data[i];
        
        cur_width += font->x_advance * scale;
        
        if (c == '\n' || cur_width >= args.break_after_width) {
            y += args.height;
            cur_x = x;
            cur_width = 0;
            cur_height += args.height;
            continue;
        }
        
        if (cur_height >= args.clamp_after_height) { break; }
        if (cur_width >= args.clamp_after_width) { break; }
        
        int glyph_index = c - 32;
        float u_off = glyph_index / (float)font->glyph_count;
        shader_set_uniform(shader, "u_off", u_off);
        shader_set_uniform(shader, "offset", screen_to_ndc(vec2(cur_x, y)));
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        cur_x = x + cur_width; 
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    return vec2(cur_x, y);
}


#define render_text_centered(TEXT, X, Y, ...) _render_text_centered(TEXT, X, Y,\
    (render_text_args) { .count = (TEXT).length, default_render_args, __VA_ARGS__ })

vec2 _render_text_centered(string text, int x, int y, render_text_args args) {
    // @Todo: new line will break this
    float scale = args.height / (float)global->font.glyph_height;
    int total_length_px = scale * text.length * global->font.x_advance;
    
    return _render_text(text, x - total_length_px / 2, y, args);
}

