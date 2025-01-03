::vertex
#version 420 core
#line 4

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform vec2 offset;
uniform vec2 scale;
uniform mat4 rotation;

out vec2 uv;

void main() {
    vec4 pos = vec4(in_position, 0, 1);
    pos = rotation * pos;

    gl_Position = vec4(pos.xy * scale + offset, 0, 1);
    uv = in_uv;
}

::fragment
#version 420 core
#line 21
         
in vec2 uv;

uniform sampler2D tex;

out vec4 out_color;

void main() {
    out_color = texture(tex, uv);

    gl_FragDepth = 0.0;
}
