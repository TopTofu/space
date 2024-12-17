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

uniform vec3 color;

out vec4 out_color;

void main() {
    vec3 ndc_pos = clip_position.xyz / clip_position.w;
    vec3 dx = dFdx(ndc_pos);
    vec3 dy = dFdy(ndc_pos);
    
    vec3 N = normalize(cross(dx, dy));
    N *= sign(N.z);
    
    vec3 L = vec3(1, 4, 1);
    float NdotL = dot(N, L);
    
    vec3 diffuse_color = color * NdotL;
    out_color = vec4(diffuse_color, 1.0);
}
