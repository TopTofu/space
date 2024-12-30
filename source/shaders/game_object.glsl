::vertex
#version 330 core
#line 3

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 translation;

out vertex_shader_out {
    vec4 world_position;
    flat float object_depth;
} vs_out;

void main() {
    gl_Position = projection * view * model * vec4(in_position, 1.0f);
    vs_out.world_position = model * vec4(in_position, 1.0f);
    
    // @Note object depth is in [0, 1] where 0 is at the near plane and 1 is at the far plane
    vec4 clip_space = projection * view * vec4(translation, 1.f);
    vs_out.object_depth = clip_space.z / clip_space.w;
}


::geometry
#version 330 core
#line 26

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vertex_shader_out {
    vec4 world_position;
    flat float object_depth;
} gs_in[];

out geometry_shader_out {
    vec3 normal;
    flat float object_depth;
} gs_out;

void main() {
    vec4 v0 = gs_in[0].world_position;
    vec4 v1 = gs_in[1].world_position;
    vec4 v2 = gs_in[2].world_position;

    vec3 normal = cross(v0.xyz - v1.xyz, v2.xyz - v1.xyz);
    gs_out.normal = normalize(normal);
    gs_out.object_depth = gs_in[0].object_depth;
    
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
#line 70

uniform vec4 color;
uniform float normal_factor;

in geometry_shader_out {
    vec3 normal;
    flat float object_depth;
} fs_in;

layout(location = 0) out vec4 surface_normal;
layout(location = 1) out vec4 object_depth;
layout(location = 2) out vec4 object_color;

void main() {
    vec3 unilateral_normal = fs_in.normal * 0.5 + 0.5; // between 0 and 1
    surface_normal.xyz = unilateral_normal * normal_factor;
    surface_normal.w = color.w;
    
    object_depth = vec4(vec3(fs_in.object_depth), 1.);
    object_color = color;
}


