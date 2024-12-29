#pragma once

typedef struct {
    string name;
    string path;
    u32 id;
} shader_info;

typedef struct {
    u32 count;
    shader_info* shaders;
} shader_catalog;

typedef struct {
    string name;
    string path;
    u32 id;
    int w, h;
    int channels;
} texture_info;

typedef struct {
    u32 count;
    texture_info* textures;
} texture_catalog;

typedef struct {
    vec3 p;
    vec2 uv;
    vec3 normal;
} vertex;

typedef struct {
    u32 vao;
    u32 index_count;
    u32 primitive;
    
    vec3 translation;
    vec3 scale;
    quat rotation;
} mesh;

typedef struct {
    u32 id;
    int width, height;
    u32 type;
    u32 color_attachment_id;
} framebuffer_attachment;

#define FRAMEBUFFER_ATTACHMENT_MAX_COUNT 16
#define GL_COLOR_ATTACHMENT GL_COLOR_ATTACHMENT0 
typedef struct {
    u32 id;
    int color_attachment_count;
    framebuffer_attachment attachments[FRAMEBUFFER_ATTACHMENT_MAX_COUNT];
} framebuffer_info;

typedef struct {
    mat4 projection_matrix;
    
    mesh line_mesh;
    mesh quad_mesh;
    mesh cube_mesh;
    
    int polygon_mode;
    
    framebuffer_info scene_framebuffer;
    framebuffer_attachment* scene_texture;
    framebuffer_attachment* scene_depth_texture;
    framebuffer_attachment* scene_object_color_texture;
    framebuffer_attachment* scene_per_object_depth_texture;
} renderer_info;

#define shader_set_uniform(shader, name, x) _Generic((x), int: shader_set_int,      \
                                                          u32: shader_set_uint,     \
                                                          float: shader_set_float,  \
                                                          vec2: shader_set_vec2,    \
                                                          vec3: shader_set_vec3,    \
                                                          vec4: shader_set_vec4,    \
                                                          mat4: shader_set_mat4     \
                                                          ) (shader, name, x)