::vertex
#version 330 core
#line 3

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 uv;
out vec3 normal;
out vec4 world_position;
out vec4 clip_position;

void main() {
    gl_Position = projection * view * model * vec4(in_position, 1.0f);
    
    uv = in_uv;
    normal = in_normal;
    world_position = model * vec4(in_position, 1.0f);
    
    clip_position = gl_Position;
}

::fragment
#version 330 core
#line 27

in vec2 uv;
in vec3 normal;
in vec4 world_position;
in vec4 clip_position;

uniform sampler2D tex;

out vec4 out_color;

vec4 face_colors[3] = vec4[3](
    vec4(0.1, 0.1, 0.1, 1),
    vec4(0.2, 0.2, 0.2, 1),
    vec4(0.3, 0.3, 0.3, 1)
);

void main() {
    int id = gl_PrimitiveID;
    int color_id = id / 2;
    out_color = face_colors[color_id % 3];
}
