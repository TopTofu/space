::vertex
#version 420 core
#line 4

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform vec2 offset;
uniform vec2 scale;
uniform mat4 rotation;

out vec2 uv;

void main() {
    gl_Position = vec4(in_pos * scale + offset, 0, 1);
    uv = in_uv;
}

::fragment
#version 420 core
#line 21
         
in vec2 uv;

uniform vec4 color;

uniform sampler2D font;
uniform float u_span;
uniform float u_off;

out vec4 out_color;

void main() {
    // u_span = 0.0052083335
    vec2 s = vec2(uv.x * u_span + u_off, uv.y);
    vec4 glyph = texture(font, s);
     
    out_color = glyph * color;
    
    gl_FragDepth = 0.0;
}
