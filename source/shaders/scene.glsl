::vertex
#version 420 core
#line 4

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

out vec2 uv;

vec2 positions[4] = vec2[4](
    vec2(-1, -1),
    vec2( 1, -1),
    vec2( 1,  1),
    vec2(-1,  1)
);

void main() {
    gl_Position = vec4(positions[gl_VertexID], 0, 1);
    uv = in_uv;
}

::fragment
#version 420 core
#line 26

in vec2 uv;

uniform sampler2D scene_texture;

out vec4 out_color;

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
    vec2 step = 1. / textureSize(scene_texture, 0);
    
    float g_x = 0;
    float g_y = 0;
    
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            float xn = uv.x + step.x * (x - 1);
            float yn = uv.y + step.y * (y - 1);
            
            g_x += rgb_to_gray(texture(scene_texture, vec2(xn, yn))) * kernelx[x][y];
            g_y += rgb_to_gray(texture(scene_texture, vec2(xn, yn))) * kernely[x][y];
        }
    }
    
    return sqrt(g_x * g_x + g_y * g_y);
}

void main() {
    vec4 scene = texture(scene_texture, uv);

    float sobel = get_sobel_edge();
    
    vec4 green = vec4(57, 255, 20, 255) / 255.;
    
    sobel = step(0.1, sobel);
    out_color = vec4(sobel, sobel, sobel, 1) * green;

    // out_color = scene;
}

