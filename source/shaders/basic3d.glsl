::vertex
#version 330 core
#line 3

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

/*
out vec2 uv;
flat out vec3 normal;
out vec4 world_position;
out vec4 clip_position;
*/

out vertex_shader_out {
    vec4 world_position;
} vs_out;

void main() {
    gl_Position = projection * view * model * vec4(in_position, 1.0f);
    vs_out.world_position = model * vec4(in_position, 1.0f);
}


::geometry
#version 330 core
#line 41

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vertex_shader_out {
    vec4 world_position;
} gs_in[];

out geometry_shader_out {
    vec3 normal;
} gs_out;

void main() {
    vec4 v0 = gs_in[0].world_position;
    vec4 v1 = gs_in[1].world_position;
    vec4 v2 = gs_in[2].world_position;

    vec3 normal = cross(v0.xyz - v1.xyz, v2.xyz - v1.xyz);
    gs_out.normal = normalize(normal);
    
    gl_Position = gl_in[0].gl_Position; 
    EmitVertex();
    gl_Position = gl_in[1].gl_Position; 
    EmitVertex();
    gl_Position = gl_in[2].gl_Position; 
    EmitVertex();
    
    EndPrimitive();
}


::fragment
#version 330 core
#line 27

uniform vec4 color;

in geometry_shader_out {
    vec3 normal;
} fs_in;

out vec4 out_color;

void main() {
    vec3 unilateral_normal = fs_in.normal * 0.5 + 0.5; // between 0 and 1
    out_color.xyz = unilateral_normal;
    out_color.w = color.w;
}


