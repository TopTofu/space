::vertex
#version 420 core
#line 4

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform vec2 offset;
uniform vec2 scale;

out vec2 uv;

void main() {
    gl_Position = vec4(in_position * scale + offset, 0, 1);
    uv = in_uv;
}

::fragment
#version 420 core
#line 21
         
in vec2 uv;

uniform sampler2D tex;

out vec4 out_color;

float get_sobel_edge();

void main() {
    float sobel = get_sobel_edge();
    vec4 green = vec4(57, 255, 20, 255) / 255.;
    
    sobel = step(0.01, sobel);
    vec4 outline = vec4(sobel) * green;
    
    out_color = outline;

    gl_FragDepth = 0.0;
}

float rgb_to_gray(vec4 rgba) {
    return 0.2989 * rgba.r + 0.5870 * rgba.g + 0.1140 * rgba.b;
}

vec3 kernelx[3] = vec3[3](
    vec3(1, 0, -1), 
    vec3(2, 0, -2), 
    vec3(1, 0, -1)
);

vec3 kernely[3] = vec3[3](
    vec3(-1, -2, -1), 
    vec3( 0,  0,  0), 
    vec3( 1,  2,  1)
);

float get_sobel_edge() {
    vec2 step = 1. / textureSize(tex, 0);
    
    float g_x = 0;
    float g_y = 0;
    
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            float xn = uv.x + step.x * (x - 1);
            float yn = uv.y + step.y * (y - 1);
            
            g_x += rgb_to_gray(texture(tex, vec2(xn, yn))) * kernelx[x][y] * 4.; // @Note this added factor removes
            g_y += rgb_to_gray(texture(tex, vec2(xn, yn))) * kernely[x][y] * 4.; //       some artifacts
        }
    }
    
    return sqrt(g_x * g_x + g_y * g_y);
}
