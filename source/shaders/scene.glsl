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

// float get_sobel_edge(sampler2D tex) {
//     vec2 step = 1. / textureSize(tex, 0);
    
//     float g_x = 0;
//     float g_y = 0;
    
//     for (int x = 0; x < 3; x++) {
//         for (int y = 0; y < 3; y++) {
//             float xn = uv.x + step.x * (x - 1);
//             float yn = uv.y + step.y * (y - 1);
            
//             g_x += rgb_to_gray(texture(tex, vec2(xn, yn))) * kernelx[x][y] * 4.; // @Note this added factor removes
//             g_y += rgb_to_gray(texture(tex, vec2(xn, yn))) * kernely[x][y] * 4.; //       some artifacts
//         }
//     }
    
//     return sqrt(g_x * g_x + g_y * g_y);
// }

float linearize_depth(float depth) {
    float ndc = depth * 2.0 - 1.0;
    float result = (2.0 * near * far) / (far + near - ndc * (far - near));
    return result;
}

float check_depth_difference() {
    float object_depth = texture(scene_depth, uv).x;
    float linear_depth = linearize_depth(object_depth) / far;
    
    float result = 0.;
    
    vec2 step = 1. / textureSize(scene_per_object_depth, 0);
    
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            float xn = uv.x + step.x * (x - 1);
            float yn = uv.y + step.y * (y - 1);
            
            float d = texture(scene_depth, vec2(xn, yn)).x;
            d = linearize_depth(d) / far;
            
            result += linear_depth - d;
            
            // g_x += rgb_to_gray(texture(tex, vec2(xn, yn))) * kernelx[x][y] * 4.; // @Note this added factor removes
            // g_y += rgb_to_gray(texture(tex, vec2(xn, yn))) * kernely[x][y] * 4.; //       some artifacts
        }
    }
    
    return result;
    return float(result > 0);
}

float get_edge() {
    float object_depth = texture(scene_per_object_depth, uv).x;
    // object_depth = linearize_depth(object_depth) / far;
    
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

            sobel_x += rgb_to_gray(texture(scene_texture, to_sample)) * kernel_x[x][y];
            sobel_y += rgb_to_gray(texture(scene_texture, to_sample)) * kernel_y[x][y];
            
            object_depth_diff += object_depth - ((texture(scene_per_object_depth, to_sample).x) );
            depth_diff        += depth        - (linearize_depth(texture(scene_depth,            to_sample).x) / far);
        }
    }
    
    object_depth_diff = abs(object_depth_diff);
    depth_diff        = abs(depth_diff);
    
    float sobel = sqrt(sobel_x * sobel_x + sobel_y * sobel_y);
    
    float result = float(sobel > 0.00001);
    
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


