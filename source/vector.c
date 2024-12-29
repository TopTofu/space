#pragma once

#include <math.h>

typedef union vec2 {
    float elements[2];
    struct { float x, y; };
    struct { float u, v; };
    struct { float q, r; };
    struct { float width, height; };
} vec2;

typedef union vec3 {
    float elements[3];
    struct { float x, y, z; };
    struct { float r, g, b; };
    struct { float h, s, v; };
    struct { 
        vec2 xy;
        float _ignored0;
    };
    struct {
        float _ignored1;
        vec2 yz;
    };
} vec3;

typedef union vec4 {
    float elements[4];
    struct {
        union {
            vec3 xyz;
            struct { float x, y, z; };
        };
        float w;
    };
    struct {
        union {
            vec3 rgb;
            vec3 hsv;
            struct { float r, g, b; };
            struct { float h, s, v; };
        };
        float a;
    };
    struct {
        vec2 xy;
        vec2 zw;
    };
    struct {
        float _ignored0;
        vec2 yz;
        float _ignored1;
    };
} vec4;

typedef vec4 color;

typedef union { 
    struct { int x, y; };
    struct { int q, r; };
    struct { int w, h; };
} ivec2;

typedef union {
    struct { int x, y, z; };
    struct { int q, r, level; };
} ivec3;

typedef struct {
    int x, y, z, w;
} ivec4;

typedef union {
    float elements[2][2];
    vec2 columns[2];
} mat2;

typedef union {
    float elements[3][3];
    vec3 columns[3];
} mat3;

typedef union {
    float elements[4][4];
    vec4 columns[4];
} mat4;

#define FLOAT32_MAX 3.402823e+38
#define FLOAT32_MIN 1.175494e-38

#define vec2(x, y)          ((vec2){x, y})
#define vec3(x, y, z)       ((vec3){x, y, z})
#define vec4(x, y, z, w)    ((vec4){x, y, z, w})

#define ivec2(x, y)         ((ivec2){x, y})
#define ivec3(x, y, z)      ((ivec3){x, y, z})
#define ivec4(x, y, z, w)   ((ivec4){x, y, z, w})

#define RGBA(r,g,b,a) {(r)/255.f, (g)/255.f, (b)/255.f, (a)/255.f}

#undef RGB // @TODO only needed on windows
#define RGB(r, g, b) RGBA(r, g, b, 255)
#define RGB_GRAY(x) RGB(x, x, x)

#define BLACK RGB_GRAY(0)
#define WHITE RGB_GRAY(255)

#define RGB_UNPACK(C) (C).r, (C).g, (C).b
#define RGBA_UNPACK(C) RGB_UNPACK(C), (C).a

#define white() RGB_GRAY(255)
#define black() RGB_GRAY(0)

#define PI 3.14159265f
#define DEG_TO_RAD(x) ((x) * 0.01745329252f)
#define RAD_TO_DEG(x) ((x) * 57.295779513f) 

#define CLAMP(x, a, b) MIN(MAX(x, a), b)
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(a) ((a) >= 0.0 ? (a) : -(a))
#define ROUND(a) ((a) < 0.0 ? (int)(a) - 0.5 : (int)(a) + 0.5)
#define MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define FMOD(x, y) ((x) - (round((x)/(y)) * (y))) // @Todo: this is broken!
#define SQRTF(x) sqrtf(x)

#define FLOOR(x) floor(x)
//((x) >= 0 ? (int)(x) : ((int)(x) - 1))

#define LERP(x, y, a) ((x) * (1.-(a)) + (a)*(y)) // maps from x -> y
#define INV_LERP(x, y, a) (((a) - (x)) / ((y) - (x))) // maps from 0 -> 1
#define STEP(edge, a) ((a) < (edge) ? 0 : 1)
#define SMOOTH_STEP(edge0, edge1, a) hermite_interpolation(edge0, edge1, a)

float hermite_interpolation(float e0, float e1, float a) {
    float t = CLAMP((a - e0) / (e1 - e0), 0, 1);
    return t * t * (3.f - 2.f * t);
}

vec2 vec2_clamp(vec2 v, float a, float b) {
    return (vec2){CLAMP((v).x, a, b), CLAMP((v).y, a, b)};
}
vec3 vec3_clamp(vec3 v, float a, float b) {
    return (vec3){CLAMP((v).x, a, b), CLAMP((v).y, a, b), CLAMP((v).z, a, b)};
}
vec4 vec4_clamp(vec4 v, float a, float b) {
    return (vec4){CLAMP((v).x, a, b), CLAMP((v).y, a, b), CLAMP((v).z, a, b), CLAMP((v).w, a, b)};
}

#define vec_clamp(v, a, b) _Generic((v), vec2: vec2_clamp,\
                                         vec3: vec3_clamp,\
                                         vec4: vec4_clamp)(v, a, b)
                                         
vec2 vec2_floor(vec2 v) {
    return (vec2){FLOOR(v.x), FLOOR(v.y)};
}                                         
                                         
vec3 vec3_floor(vec3 v) {
    return (vec3){FLOOR(v.x), FLOOR(v.y), FLOOR(v.z)};
}                                         
                                         
vec4 vec4_floor(vec4 v) {
    return (vec4){FLOOR(v.x), FLOOR(v.y), FLOOR(v.z), FLOOR(v.w)};
}                                         
                                         
#define vec_floor(v) _Generic((v), vec2: vec2_floor,\
                                   vec3: vec3_floor,\
                                   vec4: vec4_floor)(v)

#define color_clamp(c) vec_clamp((c), 0, 1)

#define lighten(c) (color_clamp(vec_mul(c, 1.2f)))
#define darken(c) (vec_mul(c, 0.9f))

// @TODO VEC_LERP, FLOOR, CEIL

#define vec_add(a, b) _Generic((b), float: _Generic((a), vec2: add_vec2_f,      \
                                                         vec3: add_vec3_f,      \
                                                         vec4: add_vec4_f,      \
                                                         ivec2: add_ivec2_i,    \
                                                         ivec3: add_ivec3_i,    \
                                                         ivec4: add_ivec4_i),   \
                                    default: _Generic((a),  vec2: add_vec2_v,   \
                                                            vec3: add_vec3_v,   \
                                                            vec4: add_vec4_v,   \
                                                            ivec2: add_ivec2_v, \
                                                            ivec3: add_ivec3_v, \
                                                            ivec4: add_ivec4_v  \
                                                            ))(a,b)

#define vec_mul(a, b) _Generic((b), float: _Generic((a), vec2: mul_vec2_f,      \
                                                         vec3: mul_vec3_f,      \
                                                         vec4: mul_vec4_f,      \
                                                         ivec2: mul_ivec2_i,    \
                                                         ivec3: mul_ivec3_i),   \
                                    int:  _Generic((a),  vec2: mul_vec2_f,      \
                                                         vec3: mul_vec3_f,      \
                                                         vec4: mul_vec4_f,      \
                                                         ivec2: mul_ivec2_i,    \
                                                         ivec3: mul_ivec3_i),   \
                                    default: _Generic((a),  vec2: mul_vec2_v,   \
                                                            vec3: mul_vec3_v,   \
                                                            vec4: mul_vec4_v,   \
                                                            ivec2: mul_ivec2_v, \
                                                            ivec3: mul_ivec3_v  \
                                                            ))(a,b)

#define vec_sub(a, b) _Generic((b), float: _Generic((a), vec2: sub_vec2_f,      \
                                                         vec3: sub_vec3_f,      \
                                                         vec4: sub_vec4_f,      \
                                                         ivec2: sub_ivec2_i,    \
                                                         ivec3: sub_ivec3_i),   \
                                    default: _Generic((a),  vec2: sub_vec2_v,   \
                                                            vec3: sub_vec3_v,   \
                                                            vec4: sub_vec4_v,   \
                                                            ivec2: sub_ivec2_v, \
                                                            ivec3: sub_ivec3_v  \
                                                            ))(a,b)

#define vec_div(a, b) _Generic((b), float: _Generic((a), vec2: div_vec2_f, \
                                                         vec3: div_vec3_f, \
                                                         vec4: div_vec4_f), \
                                    default: _Generic((a),  vec2: div_vec2_v,\
                                                            vec3: div_vec3_v,\
                                                            vec4: div_vec4_v \
                                                            ))(a,b)

#define vec_eq(a, b)  _Generic((a), vec2: eq_vec2, vec3: eq_vec3, vec4: eq_vec4)(a, b)
#define vec_dot(a, b) _Generic((a), vec2: vec2_dot, vec3: vec3_dot, vec4: vec4_dot)(a, b)
#define vec_len(v) SQRTF(vec_dot(v, v))
#define vec_norm(v) vec_div(v, vec_len(v))
#define vec_cross(a, b) (vec3_cross)(a, b)


typedef struct {
    char* name;
    int prec;
    int new_line;
} print_vec_args;
#define vec_print(v, ...) _Generic((v), vec2: print_vec2, vec3: print_vec3, vec4: print_vec4)(v, (print_vec_args){.name=#v, .new_line=true, __VA_ARGS__})

#define MAT4(v0, v1, v2, v3) (mat4){.columns={(v0), (v1), (v2), (v3)}}
#define MAT4_ID() (mat4){.columns={(vec4){1, 0, 0, 0}, (vec4){0, 1, 0, 0}, (vec4){0, 0, 1, 0}, (vec4){0, 0, 0, 1}}}

vec2 add_vec2_v(vec2 a, vec2 b) {
    return (vec2){a.x + b.x, a.y + b.y};
}
vec2 add_vec2_f(vec2 a, float f) {
    return (vec2){a.x + f, a.y + f};
}
vec3 add_vec3_v(vec3 a, vec3 b) {
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}
vec3 add_vec3_f(vec3 a, float f) {
    return (vec3){a.x + f, a.y + f, a.z + f};
}
vec4 add_vec4_v(vec4 a, vec4 b) {
    return (vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
vec4 add_vec4_f(vec4 a, float f) {
    return (vec4){a.x + f, a.y + f, a.z + f, a.w + f};
}
ivec2 add_ivec2_v(ivec2 a, ivec2 b) {
    return (ivec2) { a.x + b.x, a.y + b.y };
}
ivec2 add_ivec2_i(ivec2 a, int i) {
    return (ivec2) { a.x + i, a.y + i };
}
ivec3 add_ivec3_v(ivec3 a, ivec3 b) {
    return (ivec3) { a.x + b.x, a.y + b.y, a.z + b.z };
}
ivec3 add_ivec3_i(ivec3 a, int i) {
    return (ivec3) { a.x + i, a.y + i, a.z + i };
}
ivec4 add_ivec4_v(ivec4 a, ivec4 b) {
    return (ivec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
ivec4 add_ivec4_i(ivec4 a, int i) {
    return (ivec4) { a.x + i, a.y + i, a.z + i, a.w + i };
}



vec2 mul_vec2_v(vec2 a, vec2 b) {
    return (vec2){a.x * b.x, a.y * b.y};
}
vec2 mul_vec2_f(vec2 a, float f) {
    return (vec2){a.x * f, a.y * f};
}
vec3 mul_vec3_v(vec3 a, vec3 b) {
    return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}
vec3 mul_vec3_f(vec3 a, float f) {
    return (vec3){a.x * f, a.y * f, a.z * f};
}
vec4 mul_vec4_v(vec4 a, vec4 b) {
    return (vec4){a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}
vec4 mul_vec4_f(vec4 a, float f) {
    return (vec4){a.x * f, a.y * f, a.z * f, a.w * f};
}
ivec2 mul_ivec2_i(ivec2 a, int i) {
    return (ivec2) { a.x * i, a.y * i };
}
ivec2 mul_ivec2_v(ivec2 a, ivec2 b) {
    return (ivec2) { a.x * b.x, a.y * b.y };
}
ivec3 mul_ivec3_i(ivec3 a, int i) {
    return (ivec3) { a.x * i, a.y * i, a.z * i };
}
ivec3 mul_ivec3_v(ivec3 a, ivec3 b) {
    return (ivec3) { a.x * b.x, a.y * b.y, a.z * b.z };
}


vec2 sub_vec2_v(vec2 a, vec2 b) {
    return (vec2){a.x - b.x, a.y - b.y};
}
vec2 sub_vec2_f(vec2 a, float f) {
    return (vec2){a.x - f, a.y - f};
}
vec3 sub_vec3_v(vec3 a, vec3 b) {
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}
vec3 sub_vec3_f(vec3 a, float f) {
    return (vec3){a.x - f, a.y - f, a.z - f};
}
vec4 sub_vec4_v(vec4 a, vec4 b) {
    return (vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
vec4 sub_vec4_f(vec4 a, float f) {
    return (vec4){a.x - f, a.y - f, a.z - f, a.w - f};
}
ivec2 sub_ivec2_i(ivec2 a, int i) {
    return (ivec2) { a.x - i, a.y - i };
}
ivec2 sub_ivec2_v(ivec2 a, ivec2 b) {
    return (ivec2) { a.x - b.x, a.y - b.y };
}
ivec3 sub_ivec3_i(ivec3 a, int i) {
    return (ivec3) { a.x - i, a.y - i, a.z - i };
}
ivec3 sub_ivec3_v(ivec3 a, ivec3 b) {
    return (ivec3) { a.x - b.x, a.y - b.y, a.z - b.z };
}


vec2 div_vec2_v(vec2 a, vec2 b) {
    return (vec2){a.x / b.x, a.y / b.y};
}
vec2 div_vec2_f(vec2 a, float f) {
    return (vec2){a.x / f, a.y / f};
}
vec3 div_vec3_v(vec3 a, vec3 b) {
    return (vec3){a.x / b.x, a.y / b.y, a.z / b.z};
}
vec3 div_vec3_f(vec3 a, float f) {
    return (vec3){a.x / f, a.y / f, a.z / f};
}
vec4 div_vec4_v(vec4 a, vec4 b) {
    return (vec4){a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}
vec4 div_vec4_f(vec4 a, float f) {
    return (vec4){a.x / f, a.y / f, a.z / f, a.w / f};
}


int eq_vec2(vec2 a, vec2 b) {
    return (a.x == b.x) && (a.y == b.y);
}
int eq_vec3(vec3 a, vec3 b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}
int eq_vec4(vec4 a, vec4 b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}


float vec4_dot(vec4 a, vec4 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}
float vec3_dot(vec3 a, vec3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}
float vec2_dot(vec2 a, vec2 b) {
    return (a.x * b.x) + (a.y * b.y);
}


vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){
        .x = a.y*b.z - b.y*a.z,
        .y = a.z*b.x - b.z*a.x,
        .z = a.x*b.y - b.x*a.y
    };
}

vec2 vec2_lerp(vec2 a, vec2 b, float t) {
    return vec2(LERP(a.x, b.x, t), LERP(a.y, b.y, t));
}

vec3 vec3_lerp(vec3 a, vec3 b, float t) {
    return vec3(LERP(a.x, b.x, t), LERP(a.y, b.y, t), LERP(a.z, b.z, t));
}

vec4 vec4_lerp(vec4 a, vec4 b, float t) {
    return vec4(LERP(a.x, b.x, t), LERP(a.y, b.y, t), LERP(a.z, b.z, t), LERP(a.w, b.w, t));
}

#define vec_lerp(a, b, t) _Generic((a), vec2: vec2_lerp,\
                                        vec3: vec3_lerp,\
                                        vec4: vec4_lerp) (a, b, t)

#define vec_mix(a, a_mix, b, b_mix) _Generic((a), vec4: vec4_mix)(a, a_mix, b, b_mix)

vec4 vec4_mix(vec4 a, float a_mix, vec4 b, float b_mix) {
    return vec_add(vec_mul(a, a_mix), vec_mul(b, b_mix));
}

void print_vec2(vec2 a, print_vec_args args) {
    int prec = args.prec ? args.prec : 2;
    printf("%s = ", args.name);
    printf("{.x=%.*f, .y=%.*f}", prec, a.x, prec, a.y);
    if (args.new_line) { printf("\n"); }
}

void print_vec3(vec3 a, print_vec_args args) {
    int prec = args.prec ? args.prec : 2;
    printf("%s = ", args.name);
    printf("{.x=%.*f, .y=%.*f, .z=%.*f}", prec, a.x, prec, a.y, prec, a.z);
    if (args.new_line) { printf("\n"); }
}

void print_vec4(vec4 a, print_vec_args args) {
    int prec = args.prec ? args.prec : 2;
    printf("%s = ", args.name);
    printf("{.x=%.*f, .y=%.*f, .z=%.*f, .w=%.*f}", prec, a.x, prec, a.y, prec, a.z, prec, a.w);
    if (args.new_line) { printf("\n"); }
}


static inline mat4 translate(mat4 m, vec3 v) {
    m.columns[3] = vec_add(m.columns[3], vec4(v.x, v.y, v.z, 0));
    return m;
}

mat4 make_ortho_matrix(float w, float h) {
    // return get_perspective_matrix_base(0, w, h, 0, -1, 1);
    mat4 result;
    result.columns[0] = (vec4){2/w,      0,  0, 0};
    result.columns[1] = (vec4){  0, 2/(-h),  0, 0};
    result.columns[2] = (vec4){  0,      0, -1, 0};
    result.columns[3] = (vec4){ -1,      1,  0, 1};
    return result;
}
 
mat4 make_view_matrix(vec3 position, vec3 forward, vec3 up, vec3 right) {
    //http://www.songho.ca/opengl/gl_camera.html#lookat
    mat4 result = MAT4_ID();

    result.columns[0].x = right.x;
    result.columns[1].x = right.y;
    result.columns[2].x = right.z;
    
    result.columns[0].y = up.x;
    result.columns[1].y = up.y;
    result.columns[2].y = up.z;
    
    result.columns[0].z = forward.x;
    result.columns[1].z = forward.y;
    result.columns[2].z = forward.z;
    
    result.columns[3].x = -right.x   * position.x - right.y   * position.y - right.z   * position.z;
    result.columns[3].y = -up.x      * position.x - up.y      * position.y - up.z      * position.z;
    result.columns[3].z = -forward.x * position.x - forward.y * position.y - forward.z * position.z;
    
    return result;
}

mat4 make_projection_matrix_base(float l, float r, float b, float t, float n, float f) {
    mat4 result = {0};
    result.elements[0][0] = 2 * n / (r-l);
    result.elements[1][1] = 2 * n / (t-b);
    result.elements[2][0] = (r+l) / (r-l);
    result.elements[2][1] = (t+b) / (t-b);
    result.elements[2][2] = -(f+n) / (f-n);
    result.elements[2][3] = -1;
    result.elements[3][2] = -2 * f * n / (f-n);

    return result;
}

mat4 make_projection_matrix(float w, float h, float fov, float near_z, float far_z) {
    // http://www.songho.ca/opengl/gl_matrix.html#example1
    float aspect = w / h;

    float tangent = tanf(DEG_TO_RAD(fov/2.f));
    float height = near_z * tangent;
    float width = height * aspect;

    // this is not the fasted possible for our use case, since some values will always come out as 0
    // but having this general method also has documentation values
    return make_projection_matrix_base(-width, width, -height, height, near_z, far_z);
}

mat4 mat4_transpose(mat4 m) {
    mat4 result;

    result.elements[0][0] = m.elements[0][0];
    result.elements[0][1] = m.elements[1][0];
    result.elements[0][2] = m.elements[2][0];
    result.elements[0][3] = m.elements[3][0];
    
    result.elements[1][0] = m.elements[0][1];
    result.elements[1][1] = m.elements[1][1];
    result.elements[1][2] = m.elements[2][1];
    result.elements[1][3] = m.elements[3][1];
    
    result.elements[2][0] = m.elements[0][2];
    result.elements[2][1] = m.elements[1][2];
    result.elements[2][2] = m.elements[2][2];
    result.elements[2][3] = m.elements[3][2];
    
    result.elements[3][0] = m.elements[0][3];
    result.elements[3][1] = m.elements[1][3];
    result.elements[3][2] = m.elements[2][3];
    result.elements[3][3] = m.elements[3][3];

    return result;
}

mat4 mat4_inv(mat4 m) {
    // https://github.com/HandmadeMath/HandmadeMath/blob/master/HandmadeMath.h
    // @NOTE this a general purpose inverse, for many transform matricies there are more efficient ways to inverse them

    vec3 c01 = vec_cross(m.columns[0].xyz, m.columns[1].xyz);
    vec3 c23 = vec_cross(m.columns[2].xyz, m.columns[2].xyz);

    vec3 b10 = vec_sub(mul_vec3_f(m.columns[0].xyz, m.columns[1].w), mul_vec3_f(m.columns[1].xyz, m.columns[0].w));
    vec3 b32 = vec_sub(mul_vec3_f(m.columns[2].xyz, m.columns[3].w), mul_vec3_f(m.columns[3].xyz, m.columns[2].w));

    float inv_determinant = 1.f / (vec_dot(c01, b32) + vec_dot(c23, b10));
    c01 = vec_mul(c01, inv_determinant);
    c23 = vec_mul(c23, inv_determinant);
    b10 = vec_mul(b10, inv_determinant);
    b32 = vec_mul(b32, inv_determinant);

    mat4 result;
    result.columns[0] = (vec4){ .xyz=vec_add(vec_cross(m.columns[1].xyz, b32), vec_mul(c23, m.columns[1].w)), .w=-vec_dot(m.columns[1].xyz, c23)};
    result.columns[1] = (vec4){ .xyz=vec_sub(vec_cross(b32, m.columns[0].xyz), vec_mul(c23, m.columns[0].w)), .w=+vec_dot(m.columns[0].xyz, c23)};
    result.columns[2] = (vec4){ .xyz=vec_add(vec_cross(m.columns[3].xyz, b10), vec_mul(c01, m.columns[3].w)), .w=-vec_dot(m.columns[3].xyz, c01)};
    result.columns[3] = (vec4){ .xyz=vec_add(vec_cross(b10, m.columns[2].xyz), vec_mul(c01, m.columns[2].w)), .w=+vec_dot(m.columns[2].xyz, c01)};

    return mat4_transpose(result);
}

// @NOTE axis needs to be normalized
mat4 mat4_rotate(mat4 m, float rad, vec3 axis) {
    float  a = rad;
    float  c = cos(a);
    float  s = sin(a);

    // axis = normalize(axis);
    vec3 temp = vec_mul(axis, 1.f-c);

    // https://mathworld.wolfram.com/RodriguesRotationFormula.html
    // http://www.songho.ca/opengl/gl_matrix.html
    
    //rotation
    float r00 = c + temp.x * axis.x;
    float r01 = temp.x * axis.y + s * axis.z;
    float r02 = temp.x * axis.z - s * axis.y;

    float r10 = temp.y * axis.x - s * axis.z;
    float r11 = c + temp.y * axis.y;
    float r12 = temp.y * axis.z + s * axis.x;

    float r20 = temp.z * axis.x + s * axis.y;
    float r21 = temp.z * axis.y - s * axis.x;
    float r22 = c + temp.z * axis.z;

    // result
    float res00 = r00 * m.elements[0][0] + r10 * m.elements[0][1] + r20 * m.elements[0][2];
    float res01 = r01 * m.elements[0][0] + r11 * m.elements[0][1] + r21 * m.elements[0][2];
    float res02 = r02 * m.elements[0][0] + r12 * m.elements[0][1] + r22 * m.elements[0][2];

    float res10 = r00 * m.elements[1][0] + r10 * m.elements[1][1] + r20 * m.elements[1][2];
    float res11 = r01 * m.elements[1][0] + r11 * m.elements[1][1] + r21 * m.elements[1][2];
    float res12 = r02 * m.elements[1][0] + r12 * m.elements[1][1] + r22 * m.elements[1][2];

    float res20 = r00 * m.elements[2][0] + r10 * m.elements[2][1] + r20 * m.elements[2][2];
    float res21 = r01 * m.elements[2][0] + r11 * m.elements[2][1] + r21 * m.elements[2][2];
    float res22 = r02 * m.elements[2][0] + r12 * m.elements[2][1] + r22 * m.elements[2][2];

    float res30 = r00 * m.elements[3][0] + r10 * m.elements[3][1] + r20 * m.elements[3][2];
    float res31 = r01 * m.elements[3][0] + r11 * m.elements[3][1] + r21 * m.elements[3][2];
    float res32 = r02 * m.elements[3][0] + r12 * m.elements[3][1] + r22 * m.elements[3][2];

    return (mat4){.elements={
        {res00, res01, res02, m.elements[0][3]},
        {res10, res11, res12, m.elements[1][3]},
        {res20, res21, res22, m.elements[2][3]},
        {res30, res31, res32, m.elements[3][3]}
    }};
}

#define vec_transform(m, v) _Generic((v), vec3: vec3_transform, vec4: vec4_transform)(m, v)
#define vec_rotate(v, rad, axis) _Generic((v), vec3: vec3_rotate)(v, rad, axis)

vec3 vec3_transform(mat4 m, vec3 v) {
    float w = 1.;
    return (vec3) {
        .x = (v.x * m.elements[0][0] + v.y * m.elements[1][0] + v.z * m.elements[2][0] + w * m.elements[3][0]),
        .y = (v.x * m.elements[0][1] + v.y * m.elements[1][1] + v.z * m.elements[2][1] + w * m.elements[3][1]),
        .z = (v.x * m.elements[0][2] + v.y * m.elements[1][2] + v.z * m.elements[2][2] + w * m.elements[3][2]),
    };
}

vec4 vec4_transform(mat4 m, vec4 v) {
    return (vec4) {
        .x = (v.x * m.elements[0][0] + v.y * m.elements[1][0] + v.z * m.elements[2][0] + v.w * m.elements[3][0]),
        .y = (v.x * m.elements[0][1] + v.y * m.elements[1][1] + v.z * m.elements[2][1] + v.w * m.elements[3][1]),
        .z = (v.x * m.elements[0][2] + v.y * m.elements[1][2] + v.z * m.elements[2][2] + v.w * m.elements[3][2]),
        .w = (v.x * m.elements[0][3] + v.y * m.elements[1][3] + v.z * m.elements[2][3] + v.w * m.elements[3][3])
    };
} 

vec3 vec3_rotate(vec3 v, float rad, vec3 axis) {
    mat4 m = MAT4_ID();
    return vec_transform(mat4_rotate(m, rad, axis), v);
}


mat4 mat4_mul(mat4 left, mat4 right) {
    mat4 result;
    result.columns[0] = vec_transform(left, right.columns[0]);
    result.columns[1] = vec_transform(left, right.columns[1]);
    result.columns[2] = vec_transform(left, right.columns[2]);
    result.columns[3] = vec_transform(left, right.columns[3]);
    
    return result;
}

// @Note: up needs to be normalized
mat4 look_at(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec_norm(vec_sub(center, eye));
    
    vec3 s = vec_norm(vec_cross(f, up));
    vec3 u = vec_norm(vec_cross(s,  f)); // @Todo: is this even neccessary?
    
    mat4 result;
    
    result.columns[0] = vec4(             s.x,              u.x,            -f.x, 0);
    result.columns[1] = vec4(             s.y,              u.y,            -f.y, 0);
    result.columns[2] = vec4(             s.z,              u.z,            -f.z, 0);
    result.columns[3] = vec4(-vec_dot(s, eye), -vec_dot(u, eye), vec_dot(f, eye), 1);
    
    return result;
}



// ----- QUATERNION

typedef vec4 quat;

#define unit_quat() vec4(0, 0, 0, 1)

// @Note: b is rotated by a
// @Note: result is not normalized
static inline quat quat_mul_quat(quat a, quat b) {
    quat result;
    
    result.x =  b.elements[3] * +a.elements[0];
    result.y =  b.elements[2] * -a.elements[0];
    result.z =  b.elements[1] * +a.elements[0];
    result.w =  b.elements[0] * -a.elements[0];

    result.x += b.elements[2] * +a.elements[1];
    result.y += b.elements[3] * +a.elements[1];
    result.z += b.elements[0] * -a.elements[1];
    result.w += b.elements[1] * -a.elements[1];
    
    result.x += b.elements[1] * -a.elements[2];
    result.y += b.elements[0] * +a.elements[2];
    result.z += b.elements[3] * +a.elements[2];
    result.w += b.elements[2] * -a.elements[2];

    result.x += b.elements[0] * +a.elements[3];
    result.y += b.elements[1] * +a.elements[3];
    result.z += b.elements[2] * +a.elements[3];
    result.w += b.elements[3] * +a.elements[3];
    
    return result;
}

static inline mat4 quat_to_mat(quat q) {
    quat norm = vec_norm(q);
    
    float xx = norm.x * norm.x;
    float yy = norm.y * norm.y;
    float zz = norm.z * norm.z;
    float xy = norm.x * norm.y;
    float xz = norm.x * norm.z;
    float yz = norm.y * norm.z;
    float wx = norm.w * norm.x;
    float wy = norm.w * norm.y;
    float wz = norm.w * norm.z;

    mat4 result;
    result.elements[0][0] = 1.0f - 2.0f * (yy + zz);
    result.elements[0][1] = 2.0f * (xy + wz);
    result.elements[0][2] = 2.0f * (xz - wy);
    result.elements[0][3] = 0;
    
    result.elements[1][0] = 2.0f * (xy - wz);
    result.elements[1][1] = 1.0f - 2.0f * (xx + zz);
    result.elements[1][2] = 2.0f * (yz + wx);
    result.elements[1][3] = 0.0f;
    
    result.elements[2][0] = 2.0f * (xz + wy);
    result.elements[2][1] = 2.0f * (yz - wx);
    result.elements[2][2] = 1.0f - 2.0f * (xx + yy);
    result.elements[2][3] = 0.0f;

    result.elements[3][0] = 0.0f;
    result.elements[3][1] = 0.0f;
    result.elements[3][2] = 0.0f;
    result.elements[3][3] = 1.0f;
    
    return result;
}

static inline quat mat_to_quat(mat4 m) {
    float t;
    quat result;
    
    if (m.elements[2][2] < 0.0f) {
        if (m.elements[0][0] > m.elements[1][1]) {
            t = 1 + m.elements[0][0] - m.elements[1][1] - m.elements[2][2];
            result = (quat) {
                .x = t,
                .y = m.elements[0][1] + m.elements[1][0],
                .z = m.elements[2][0] + m.elements[0][2],
                .w = m.elements[1][2] - m.elements[2][1]
            };
        } else {
            t = 1 - m.elements[0][0] + m.elements[1][1] - m.elements[2][2];
            result = (quat) {
                m.elements[0][1] + m.elements[1][0],
                t,
                m.elements[1][2] + m.elements[2][1],
                m.elements[2][0] - m.elements[0][2]
            };
        }
    } else {
        if (m.elements[0][0] < -m.elements[1][1]) {
            t = 1 - m.elements[0][0] - m.elements[1][1] + m.elements[2][2];
            result = (quat) {
                m.elements[2][0] + m.elements[0][2],
                m.elements[1][2] + m.elements[2][1],
                t,
                m.elements[0][1] - m.elements[1][0]
            };
        } else {
            t = 1 + m.elements[0][0] + m.elements[1][1] + m.elements[2][2];
            result = (quat) {
                m.elements[1][2] - m.elements[2][1],
                m.elements[2][0] - m.elements[0][2],
                m.elements[0][1] - m.elements[1][0],
                t
            };
        }
    }
    
    result = vec_mul(result, 0.5f / (float)sqrt(t));
    
    return result;
}

// @Note: axis needs to be normalized
static inline quat quat_from_axis_angle(vec3 axis, float angle) {
    quat result;
    
    float s = sin(angle / 2.0f);
    
    result.xyz = vec_mul(axis, s);
    result.w = cos(angle / 2.0f);
    
    return vec_norm(result);
}

// @Info: this rotates u to v
quat quat_from_unit_vectors(vec3 u, vec3 v) {
    // source: https://www.xarg.org/proof/quaternion-from-two-vectors/
    float dot = vec_dot(u, v);
    
    if (dot > 0.99999) { // @Info: parallel vectors, return identity quat
        return vec4(0, 0, 0, 1);
    } else if (dot < -0.99999) { // @Info: u and v are facing each other; 180 degree turn
        // @Note: this rotates around the y axis
        return vec4(0, 1, 0, 0);
    }
    
    quat result;
    
    vec3 c = vec_cross(u, v);
    float w = 1.f + vec_dot(u, v);
    
    result.x = c.x;
    result.y = c.y; 
    result.z = c.z;
    result.w = w;
    
    return vec_norm(result);
}

static inline quat quat_invert(quat a) {
    return (quat){ -a.x, -a.y, -a.z, a.w };
}

static inline quat quat_nlerp(quat left, float t, quat right) {
    quat result = vec_mix(left, 1.0 - t, right, t);
    return vec_norm(result);
}

static inline vec3 quat_rotate_vec(quat q, vec3 v) {
    return vec_transform(quat_to_mat(q), v);
}

static inline quat quat_slerp(quat left, float t, quat right) {
    quat result;
    
    float cos_theta = vec_dot(left, right);
    
    // @Note: take the shorter path around the sphere
    if (cos_theta < 0.0f) {
        cos_theta = -cos_theta;
        right = (quat) { -right.x, -right.y, -right.z, -right.w };
    }
    
    if (cos_theta > 0.9995f) {
        result = quat_nlerp(left, t, right);
    } else {
        float angle = acos(cos_theta);
        float mix_left = sin((1.0 - t) * angle);
        float mix_right = sin(t * angle);
        
        result = vec_mix(left, mix_left, right, mix_right);
        result = vec_norm(result);
    }
    
    return result;
}

static inline vec3 vec3_slerp(vec3 a, vec3 b, float t) {
    // source: https://www.shadertoy.com/view/4sV3zt
    float dot = vec_dot(a, b);
    
    // @Note: not strickly necessary
    dot = CLAMP(dot, -1.0, 1.0);
    
    float theta = acos(dot) * t;
    vec3 relative_vec = vec_norm(vec_sub(b, vec_mul(a, dot)));
    
    return vec_add(vec_mul(a, (float)cos(theta)), 
                   vec_mul(relative_vec, (float)sin(theta)));
}

mat4 make_model_matrix(vec3 t, vec3 s, quat r) {
    mat4 result = MAT4_ID();
    
    mat4 scale = (mat4){ .columns = {
        {s.x,   0,   0, 0},
        {  0, s.y,   0, 0},
        {  0,   0, s.z, 0},
        {  0,   0,   0, 1}}
    };
    
    mat4 rotation = quat_to_mat(r);
    
    mat4 translation = { .columns = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {t.x, t.y, t.z, 1}}
    };
    
    result = mat4_mul(scale, result);
    result = mat4_mul(rotation, result);
    result = mat4_mul(translation, result);
    
    // result.columns[0] = (vec4){s.x,   0,   0, 0};
    // result.columns[1] = (vec4){  0, s.y,   0, 0};
    // result.columns[2] = (vec4){  0,   0, s.z, 0};
    // result.columns[3] = (vec4){t.x, t.y, t.z, 1};
    return result;
}


// ----- COLOR

#define rgb_invert(C) { 1 - (C).r, 1 - (C).g, 1 - (C).b, (C).a }

color rgb_to_hsv(color c) {
    float max = MAX(c.r, MAX(c.g, c.b));
    float min = MIN(c.r, MIN(c.g, c.b));
    float delta = max - min;

    float h;
    if (delta == 0) {
        h = 0;
    } else if (max == c.r) {
        h = 60 * (fmod((c.g-c.b) / delta, 6));
    } else if (max == c.g) {
        h = 60 * (((c.b-c.r) / delta) + 2);
    } else {
        h = 60 * (((c.r-c.g) / delta) + 4);
    }

    while (h < 0) { h += 360; }

    float s = (max == 0) ? 0 : (delta / max);
    float v = max;

    return (color){h, s, v, c.a};
}

color hsv_to_rgb(color c) {
    if (c.h >= 360) { c.h = 0; }
    c.h /= 60.f;
    
    float frac = c.h - (int)c.h;
    float p = c.v * (1.f - c.s);
    float q = c.v * (1.f - (c.s * frac));
    float t = c.v * (1.f - (c.s * (1.f - frac)));

    float r, g, b;
    switch ((int)c.h) {
        case 0: {
            r = c.v, g = t, b = p;
        } break;
        case 1: {
            r = q, g = c.v, b = p;
        } break;
        case 2: {
            r = p, g = c.v, b = t;
        } break;
        case 3: {
            r = p, g = q, b = c.v;
        } break;
        case 4: {
            r = t, g = p, b = c.v;
        } break;
        case 5:
        default: {
            r = c.v, g = p, b = q;
        } break;
    }

    return (color){r, g, b, c.a};
}

/*
double sin_approx(double x) {
    double x2 = x * x;
    double x3 = x2 * x;
    double x5 = x2 * x3;
    
    x = x - x3 / 6.0 + x5 / 120.0;
    return x;
}

double fast_sin(double x) {
    int k = floor(x * 2.0 / PI);
    double _x = x - k * PI * 0.5;
    
    switch (k % 4) {
        case 0: return sin_approx(_x);
        case 1: return sin_approx(-_x + PI);
        case 2: return -sin_approx(_x - PI); 
        case 3: return -sin_approx(-_x + 2.0 * PI);
    }
    
    return 0;
}
*/


float ease_out_cubic(float t) {
    float a = 1. - t;
    return 1. - (a * a * a);
}

float ease_out_squared(float t) {
    float a = 1. - t;
    return 1. - (a * a);
}

float ease_out_circ(float t) {
    float a = 1. - t;
    return sqrt(1. - (a * a));
}

float ease_in_out_sin(float t) {
    return - (cos(PI * t) - 1.f) / 2.f;
}

float ease_out_back(float t) {
    float c0 = 1.70158;
    float c1 = c0 + 1.;
    
    float b = t - 1.;
    
    return 1. + c1 * (b * b * b) + c0 * (b * b);
}
