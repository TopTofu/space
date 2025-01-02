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
uniform sampler2D scene_per_object_depth;
uniform sampler2D scene_object_color;
uniform sampler2D scene_depth;

uniform float near;
uniform float far;

out vec4 out_color;

float rgb_to_gray(vec4 rgba) {
    return 0.2989 * rgba.r + 0.5870 * rgba.g + 0.1140 * rgba.b;
}

vec3 kernel_x[3] = vec3[3](
    vec3(1, 0, -1), 
    vec3(2, 0, -2), 
    vec3(1, 0, -1)
);

vec3 kernel_y[3] = vec3[3](
    vec3(-1, -2, -1), 
    vec3( 0,  0,  0), 
    vec3( 1,  2,  1)
);

float linearize_depth(float depth) {
    float ndc = depth * 2.0 - 1.0;
    float result = (2.0 * near * far) / (far + near - ndc * (far - near));
    return result;
}


/*
    @Info  if we find a pixel which is soundrounded by different colored pixels, it is likely that it's an z-fighting
           artifact, so we try to detect and fix that here
    @Speed it does result in a ton of texture reads though.. 
*/
float get_scene_color(vec2 to_sample) {
    float gray_scale = rgb_to_gray(texture(scene_texture, to_sample));
    vec2 uv_step = 1. / textureSize(scene_texture, 0);
    
    int diff_neighbor_count = 0;

    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            float xn = uv.x + uv_step.x * (x - 1);
            float yn = uv.y + uv_step.y * (y - 1);
            
            float g = rgb_to_gray(texture(scene_texture, vec2(xn, yn)));
            diff_neighbor_count += int(gray_scale != g);
        }
    }
    
    if (diff_neighbor_count >= 2) {
        gray_scale = rgb_to_gray(texture(scene_texture, to_sample + vec2(uv_step.x, uv_step.y)));
    }
    
    return gray_scale;
}

float get_edge() {
    float object_depth = texture(scene_per_object_depth, uv).x;
    object_depth = linearize_depth(object_depth) / far;
    
    float depth = texture(scene_depth, uv).x;
    depth = linearize_depth(depth) / far;
    
    vec2 uv_step = 1. / textureSize(scene_texture, 0);
    
    float sobel_x = 0;
    float sobel_y = 0;
    
    float object_depth_diff = 0;
    float depth_diff = 0;
    
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            float xn = uv.x + uv_step.x * (x - 1);
            float yn = uv.y + uv_step.y * (y - 1);
            vec2 to_sample = vec2(xn, yn);

            // float gray_scale = rgb_to_gray(texture(scene_texture, to_sample));
            float gray_scale = get_scene_color(to_sample);

            sobel_x += gray_scale * kernel_x[x][y];
            sobel_y += gray_scale * kernel_y[x][y];
            
            object_depth_diff += object_depth - (linearize_depth(texture(scene_per_object_depth, to_sample).x) / far);
            depth_diff        += depth        - (linearize_depth(texture(scene_depth,            to_sample).x) / far);
        }
    }
    
    object_depth_diff = abs(object_depth_diff);
    depth_diff        = abs(depth_diff);
    
    float sobel = sqrt(sobel_x * sobel_x + sobel_y * sobel_y);
    
    float result = float(sobel > 0.1);
    
    bool o = (object_depth_diff > 0.);
    bool d = (depth_diff > 0.001);
    
    if (result == 0.) {
        result = float(d);        
    }
    
    return result;
}


void main() {
    vec4 scene = texture(scene_texture, uv);
    vec4 object_color = texture(scene_object_color, uv);
    float object_depth = texture(scene_per_object_depth, uv).x;
    
    float edge = get_edge();
    out_color = vec4(vec3(edge), 1) * object_color;
    
    // out_color = scene;
    
    return;    
/*
    
    // float ddd = texture(scene_depth, uv).x;
    // out_color = vec4(linearize_depth(ddd) / far);
    // return;
    
    // out_color = vec4(object_depth);
    // return;
    

    float sobel = get_sobel_edge(scene_texture);
    float depth_sobel = get_sobel_edge(scene_per_object_depth);
    
    vec4 green = vec4(57, 255, 20, 255) / 255.;
    
    sobel = step(0.01, sobel);
    out_color = vec4(sobel, sobel, sobel, 1) * object_color;

    //out_color = scene;
    
    float diff = check_depth_difference();
    diff = step(0.001, diff);
    out_color = vec4(vec3(diff), 1.);
    return;
    
    //return;
    float depth = texture(scene_depth, uv).x; 
    float linear_depth = linearize_depth(depth) / far;
    
    
    out_color = vec4(vec3(linear_depth), 1.);
    
    out_color = vec4(vec3(step(0.000001, depth_sobel)), 1.);
    // out_color = vec4(depth_sobel);
    return;    
    
    
    depth_sobel = step(0.1, depth_sobel) + sobel;
    out_color = vec4(vec3((depth_sobel)), 1.) * green;
    
    out_color = texture(scene_per_object_depth, uv);
    
        
*/
}


