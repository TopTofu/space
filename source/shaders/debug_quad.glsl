::vertex
#version 330 core
#line 3

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform mat4 view;
uniform mat4 projection;

out mat4 view_projection;

void main() {
    view_projection = projection * view;
}

::geometry
#version 330 core
#line 31

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

uniform vec3 p0;
uniform vec3 p1;

in mat4 view_projection[];

void main() {
    // @Note only works if the quad is axis-aligned
    vec3 quad_normal = vec3(float(p0.x == p1.x), float(p0.y == p1.y), float(p0.z == p1.z));
    
    vec3 diagonal = (p1 - p0) / 2.;
    vec3 m = p0 + diagonal;
    
    vec3 p2 = m + cross(quad_normal, diagonal);
    vec3 p3 = m + cross(diagonal, quad_normal);
    
    gl_Position = view_projection[0] * vec4(p0, 1.);
    EmitVertex();
    gl_Position = view_projection[0] * vec4(p1, 1.);
    EmitVertex();
    gl_Position = view_projection[0] * vec4(p2, 1.);
    EmitVertex();
    EndPrimitive();
    
    gl_Position = view_projection[0] * vec4(p0, 1.);
    EmitVertex();
    gl_Position = view_projection[0] * vec4(p1, 1.);
    EmitVertex();
    gl_Position = view_projection[0] * vec4(p3, 1.);
    EmitVertex();
    EndPrimitive();
}


::fragment
#version 330 core
#line 27

uniform vec4 color;

out vec4 out_color;

void main() {
    out_color = color;
}
