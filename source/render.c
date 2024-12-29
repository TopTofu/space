#pragma once


static u32 make_vao(vertex* vertices, int vertex_count, u32* indices, int index_count) {
    u32 result;
    
    glGenVertexArrays(1, &result);
    glBindVertexArray(result);
    
    u32 buffers[2]; // 0 - vbo, 1 - ebo
    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(2, buffers);

    return result;
}

mesh load_obj(char* file_path) {
    string source = read_file(string(file_path), &global->transient_arena);
    
    string comment_prefix   = string("#");
    string group_prefix     = string("g");
    string vertex_prefix    = string("v");
    string uv_prefix        = string("vt");
    string normal_prefix    = string("vn");
    string face_prefix      = string("f");
    string line_prefix      = string("l");

    int vertex_count = 0;
    int index_count = 0;
    
    string cursor = source;
    while (cursor.length) {
        string line = string_eat_line(&cursor);
        string prefix = string_eat_token(&line, ' ');

        if (string_compare(prefix, vertex_prefix)) { vertex_count++; }
        if (string_compare(prefix, face_prefix)) { index_count += 3; }
    }   
    
    vertex* vertices = push_transient(sizeof(vertices[0]) * vertex_count * 3);
    u32* indices     = push_transient(sizeof(indices[0]) * index_count);

    int vertex_i = 0;
    int index_i = 0;

    cursor = source;
    while (cursor.length) {
        string line = string_eat_line(&cursor);
        string prefix = string_eat_token(&line, ' ');
        if (string_compare(prefix, comment_prefix)) { }
        else if (string_compare(prefix, group_prefix)) { }
        else if (string_compare(prefix, vertex_prefix)) { 
            float x = string_eat_double(&line);
            float y = string_eat_double(&line);
            float z = string_eat_double(&line);
            
            vertices[vertex_i++].p = vec3(x, y, z);
        }
        
        else if (string_compare(prefix, uv_prefix)) { }
        else if (string_compare(prefix, normal_prefix)) { }
        else if (string_compare(prefix, face_prefix)) {
            while (line.length) {
                int a = string_eat_int(&line, 10) - 1; 
                indices[index_i++] = a;
                string_eat_token(&line, ' ');
            }
        }
        else if (string_compare(prefix, line_prefix)) { }
    }
    
    mesh result = { .primitive = GL_TRIANGLES,
                .index_count = index_count,
                .scale = vec3(1, 1, 1),
                .rotation = unit_quat() };
    
    result.vao = make_vao(vertices, vertex_count, indices, index_count);
    
    return result;
}

mesh make_line_mesh() {
    // @info: this mesh is for debugging. it should be rendered using the immediate_line shader.
    //        since the shader takes 2 uniform positions and draws the line using the geometry shader
    //        stage, this is just a single vertex dummy mesh to invoke that geometry shader.

    mesh result = { .primitive = GL_POINTS,
                    .index_count = 1,
                    .scale = vec3(1, 1, 1),
                    .rotation = unit_quat() };
    vertex vertices[1] = { 0 };
    
    u32 indices[1] = { 0 };
    
    result.vao = make_vao(vertices, 1, indices, 1);
    return result;
}

mesh make_quad_mesh() {
    mesh result = { .primitive = GL_TRIANGLES,
                    .index_count = 6,
                    .scale = vec3(1, 1, 1),
                    .rotation = unit_quat() };
    
    // vertex vertices[4] = {
    //     { .p = vec3(0, 0, 0), .uv = vec2(0, 0) },
    //     { .p = vec3(1, 0, 0), .uv = vec2(1, 0) },
    //     { .p = vec3(1, 1, 0), .uv = vec2(1, 1) },
    //     { .p = vec3(0, 1, 0), .uv = vec2(0, 1) }
    // };
    vertex vertices[4] = {
        { .p = vec3(-1, -1, 0), .uv = vec2(0, 0) },
        { .p = vec3( 1, -1, 0), .uv = vec2(1, 0) },
        { .p = vec3( 1,  1, 0), .uv = vec2(1, 1) },
        { .p = vec3(-1,  1, 0), .uv = vec2(0, 1) }
    };
    
    u32 indices[6] = { 0, 1, 2, 0, 2, 3 };
    
    result.vao = make_vao(vertices, 4, indices, 6);
    return result;
}

mesh make_cube_mesh() {
    mesh result = { .primitive = GL_TRIANGLES,
                    .index_count = 36,
                    .scale = vec3(1, 1, 1),
                    .rotation = unit_quat() };

    vertex vertices[8] = {
        { .p = vec3(-.5, -.5, -.5) },
        { .p = vec3(+.5, -.5, -.5) },
        { .p = vec3(+.5, +.5, -.5) },
        { .p = vec3(-.5, +.5, -.5) },
        { .p = vec3(-.5, -.5, +.5) },
        { .p = vec3(+.5, -.5, +.5) },
        { .p = vec3(+.5, +.5, +.5) },
        { .p = vec3(-.5, +.5, +.5) }
    };
    
    u32 indices[36] = {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        3, 2, 6, 3, 6, 7,
        5, 4, 7, 5, 7, 6,  
        4, 3, 7, 4, 0, 3,
        1, 4, 5, 1, 0, 4
    };
    
    result.vao = make_vao(vertices, 8, indices, 36);
    result.vao = make_vao(vertices, 8, indices, 36);
    return result;
}

static mesh make_thruster_mesh() {
    int n = 8;
    float theta = DEG_TO_RAD(360.0 / (float)n);
    
    int index_count = n * 12;
    int vertex_count = n * 3;
    
    vertex* vertices = push_transient(sizeof(vertices[0]) * vertex_count);
    u32* indices     = push_transient(sizeof(indices[0]) * index_count);
    
    int index_counter = 0;
    for (int i = 0; i < n; i++) {
        vertices[i        ].p = vec3(0.5 * cos(theta * i),  0.5, 0.5 * sin(theta * i));
        vertices[i + n    ].p = vec3(0.5 * cos(theta * i),  0.2, 0.5 * sin(theta * i));
        vertices[i + 2 * n].p = vec3(0.3 * cos(theta * i), -0.5, 0.3 * sin(theta * i));
    
        if (i < n - 1) {
            indices[index_counter++] = i;
            indices[index_counter++] = (i + n);
            indices[index_counter++] = (i + 1);
            
            indices[index_counter++] = (i + 1);
            indices[index_counter++] = (i + n);
            indices[index_counter++] = (i + n + 1);
            
            indices[index_counter++] = (i + n)    ;
            indices[index_counter++] = (i + 2 * n);
            indices[index_counter++] = (i + n + 1);
            
            indices[index_counter++] = (i + n + 1);
            indices[index_counter++] = (i + 2 * n);
            indices[index_counter++] = (i + 2 * n + 1);
        }        
    }
    
    // @Info: add the last 4 triangles by hand because indices do not cycle nicely
    indices[index_counter++] = 0;
    indices[index_counter++] = 2 * n - 1;
    indices[index_counter++] = n;
    
    indices[index_counter++] = 0;
    indices[index_counter++] = n - 1;
    indices[index_counter++] = 2 * n - 1;
    
    indices[index_counter++] = n;
    indices[index_counter++] = 2 * n - 1;
    indices[index_counter++] = 3 * n - 1;
    
    indices[index_counter++] = n;
    indices[index_counter++] = 3 * n - 1;
    indices[index_counter++] = 2 * n;
    
    mesh result = { .primitive = GL_TRIANGLES, 
                    .scale = vec3(1, 1, 1),
                    .rotation = unit_quat(),
                    .index_count = index_counter };
    
    result.vao = make_vao(vertices, vertex_count, indices, index_counter);
    return result;
}

mesh make_tank_mesh() {
    int n = 8;
    float theta = DEG_TO_RAD(360.0 / (float)n);
    
    int index_count  = n * 6 + 3 * n;
    int vertex_count = n * 2;
    
    vertex* vertices = push_transient(sizeof(vertices[0]) * vertex_count);
    u32* indices     = push_transient(sizeof(indices[0]) * index_count);
    
    int index_counter = 0;
    
    for (int i = 0; i < n; i++) {
        int c = 2 * i;
        
        vertices[c    ].p = vec3(-.5, 0.5 * cos(theta * i), 0.5 * sin(theta * i));
        vertices[c + 1].p = vec3(0.5, 0.5 * cos(theta * i), 0.5 * sin(theta * i));
        
        indices[index_counter++] = (c    ) % (2 * n);
        indices[index_counter++] = (c + 1) % (2 * n);
        indices[index_counter++] = (c + 3) % (2 * n);
        
        indices[index_counter++] = (c    ) % (2 * n);
        indices[index_counter++] = (c + 3) % (2 * n);
        indices[index_counter++] = (c + 2) % (2 * n);
    }
    
    for (int i = 1; i < n / 2; i++) {
        indices[index_counter++] = 0;
        indices[index_counter++] = i * 2 + 2;
        indices[index_counter++] = i * 2;
        
        indices[index_counter++] = 0;
        indices[index_counter++] = 2 * n - i * 2;
        indices[index_counter++] = 2 * n - (i * 2 + 2);
        
        indices[index_counter++] = 1;
        indices[index_counter++] = 1 + i * 2 + 2;
        indices[index_counter++] = 1 + i * 2;
        
        indices[index_counter++] = 1;
        indices[index_counter++] = 1 + 2 * n - i * 2;
        indices[index_counter++] = 1 + 2 * n - (i * 2 + 2);
    }
    
    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = index_counter 
    };
    
    result.vao = make_vao(vertices, vertex_count, indices, index_counter);
    return result;
}

static mesh make_slope_mesh() {
    float slope = 0.3;

    vertex vertices[10] = {
        // front
        { .p = vec3(-.5, -.5, -.5) },
        { .p = vec3(0.5, -.5, -.5) },
        { .p = vec3(0.5, 0.5, -.5) },
        { .p = vec3(0.5 - slope, 0.5, -.5) },
        { .p = vec3(-.5, -.5 + slope, -.5) },
    
        // back
        { .p = vec3(-.5, -.5, 0.5) },
        { .p = vec3(0.5, -.5, 0.5) },
        { .p = vec3(0.5, 0.5, 0.5) },
        { .p = vec3(0.5 - slope, 0.5, 0.5) },
        { .p = vec3(-.5, -.5 + slope, 0.5) },
    };
    
    u32 indices[] = {
        0, 1, 2, 0, 2, 3, 0, 3, 4, // front face
        5, 7, 6, 5, 8, 7, 5, 9, 8, // back face
        0, 6, 1, 0, 5, 6,          // bottom
        1, 6, 7, 1, 7, 2,          // right
        2, 7, 8, 2, 8, 3,          // top
        3, 8, 9, 3, 9, 4,          // slope
        0, 4, 9, 0, 9, 5           // left
    };
    
    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}

float wing_inset  = 0.2;
float wing_half_w = 0.1;  
static mesh make_wing_mesh() {
    vertex vertices[] = {
        // top
        { .p = vec3( -.5 + wing_inset, wing_half_w, -.5 ) },    
        { .p = vec3( 0.5 - wing_inset, wing_half_w, -.5 ) },    
        { .p = vec3( 0.5,              wing_half_w, 0.5 ) },    
        { .p = vec3( -.5,              wing_half_w, 0.5 ) },
        
        // bottom
        { .p = vec3( -.5 + wing_inset, -wing_half_w, -.5 ) },    
        { .p = vec3( 0.5 - wing_inset, -wing_half_w, -.5 ) },    
        { .p = vec3( 0.5,              -wing_half_w, 0.5 ) },    
        { .p = vec3( -.5,              -wing_half_w, 0.5 ) },    
    };
    
    u32 indices[] = {
        0, 4, 5, 0, 5, 1, // front
        1, 5, 6, 1, 6, 2, // right
        2, 6, 7, 2, 7, 3, // back
        3, 7, 4, 3, 4, 0, // left
        0, 1, 2, 0, 2, 3, // top
        4, 7, 6, 4, 6, 5, // bottom
    };

    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}

static mesh make_wing_tip_mesh() {
    vertex vertices[] = {
        // top
        { .p = vec3(   0,              wing_half_w, -.5 ) },    
        { .p = vec3( 0.5 - wing_inset, wing_half_w, 0.5 ) },    
        { .p = vec3( -.5 + wing_inset, wing_half_w, 0.5 ) },    
        
        // bottom
        { .p = vec3(   0,              -wing_half_w, -.5 ) },    
        { .p = vec3( 0.5 - wing_inset, -wing_half_w, 0.5 ) },    
        { .p = vec3( -.5 + wing_inset, -wing_half_w, 0.5 ) },    
    };
    
    u32 indices[] = {
        0, 1, 2,          // top
        0, 3, 4, 0, 4, 1, // right
        1, 4, 5, 1, 5, 2, // back
        0, 2, 5, 0, 5, 3, // left
        3, 5, 4,          // bottom
    };

    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}

static mesh make_connector_mesh() {
    float gap = 0.1;
    float size = 0.1;
    
    vec3 m0 = vec3(- gap - size, 0., 0.);
    vec3 m1 = vec3(  gap + size, 0., 0.);
    
    vertex vertices[] = {
        // rod 1
        // front
        { .p = vec3(m0.x - size / 2., m0.y - size / 2., -.5) },
        { .p = vec3(m0.x + size / 2., m0.y - size / 2., -.5) },
        { .p = vec3(m0.x + size / 2., m0.y + size / 2., -.5) },
        { .p = vec3(m0.x - size / 2., m0.y + size / 2., -.5) },
        // back
        { .p = vec3(m0.x - size / 2., m0.y - size / 2., 0.5) },
        { .p = vec3(m0.x + size / 2., m0.y - size / 2., 0.5) },
        { .p = vec3(m0.x + size / 2., m0.y + size / 2., 0.5) },
        { .p = vec3(m0.x - size / 2., m0.y + size / 2., 0.5) },
        
        // rod 2
        // front
        { .p = vec3(m1.x - size / 2., m1.y - size / 2., -.5) },
        { .p = vec3(m1.x + size / 2., m1.y - size / 2., -.5) },
        { .p = vec3(m1.x + size / 2., m1.y + size / 2., -.5) },
        { .p = vec3(m1.x - size / 2., m1.y + size / 2., -.5) },
        // back
        { .p = vec3(m1.x - size / 2., m1.y - size / 2., 0.5) },
        { .p = vec3(m1.x + size / 2., m1.y - size / 2., 0.5) },
        { .p = vec3(m1.x + size / 2., m1.y + size / 2., 0.5) },
        { .p = vec3(m1.x - size / 2., m1.y + size / 2., 0.5) },
    };
    
    u32 indices[] = {
        // rod 1
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2, 
        2, 6, 7, 2, 7, 3,
        0, 4, 5, 0, 5, 1,
        0, 3, 7, 0, 7, 4,
        4, 7, 6, 4, 6, 5,
    
        // rod 2        
        0 + 8, 1 + 8, 2 + 8, 0 + 8, 2 + 8, 3 + 8,
        1 + 8, 5 + 8, 6 + 8, 1 + 8, 6 + 8, 2 + 8, 
        2 + 8, 6 + 8, 7 + 8, 2 + 8, 7 + 8, 3 + 8,
        0 + 8, 4 + 8, 5 + 8, 0 + 8, 5 + 8, 1 + 8,
        0 + 8, 3 + 8, 7 + 8, 0 + 8, 7 + 8, 4 + 8,
        4 + 8, 7 + 8, 6 + 8, 4 + 8, 6 + 8, 5 + 8,
    };
    
    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}

static mesh make_quarter_tube_mesh() {
    int n = 5;
    float theta = DEG_TO_RAD(360.0 / ((n - 1) * 4.));

    int vertex_count = n * 2 + 2;
    int index_count = (n - 1) * 6 + (n - 1) * 3;
    
    int index_counter = 0;

    vertex* vertices = push_transient(sizeof(vertex) * vertex_count);
    u32* indices = push_transient(sizeof(u32) * index_count);

    for (int i = 0; i < n; i++) {
        vertices[i * 2    ].p = vec3(cos(theta * i) - .5, sin(theta * i) - 0.5, -.5);
        vertices[i * 2 + 1].p = vec3(cos(theta * i) - .5, sin(theta * i) - 0.5, 0.5);
        
        if (i < n - 1) {
            indices[index_counter++] = i * 2;
            indices[index_counter++] = i * 2 + 1;
            indices[index_counter++] = i * 2 + 3;
            
            indices[index_counter++] = i * 2;
            indices[index_counter++] = i * 2 + 3;
            indices[index_counter++] = i * 2 + 2;
            
            indices[index_count++] = n * 2;
            indices[index_count++] = i * 2;
            indices[index_count++] = i * 2 + 2;
            
            indices[index_count++] = n * 2 + 1;
            indices[index_count++] = i * 2 + 1;
            indices[index_count++] = i * 2 + 3;
        }
    }
    
    vertices[n * 2    ].p = vec3(-.5, -.5, -.5);
    vertices[n * 2 + 1].p = vec3(-.5, -.5, 0.5);

    
    
    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = index_count
    };
    
    result.vao = make_vao(vertices, vertex_count, indices, result.index_count);
    return result;
}

static mesh make_fin_mesh() {
    float width = 0.2;
    
    vertex vertices[] = {
        { .p = vec3(width * -.5, -.5,         -.5) },
        { .p = vec3(width * 0.5, -.5,         -.5) },
        { .p = vec3(width * 0.5, -.5 + width, -.5) },
        { .p = vec3(width * -.5, -.5 + width, -.5) },
        
        { .p = vec3(width * -.5, -.5, 0.5) },
        { .p = vec3(width * 0.5, -.5, 0.5) },
        { .p = vec3(width * 0.5, 0.5, 0.5) },
        { .p = vec3(width * -.5, 0.5, 0.5) },
    };
    
    u32 indices[] = {
        0, 4, 5, 0, 5, 1, // front
        1, 5, 6, 1, 6, 2, // right
        2, 6, 7, 2, 7, 3, // back
        3, 7, 4, 3, 4, 0, // left
        0, 1, 2, 0, 2, 3, // top
        4, 7, 6, 4, 6, 5, // bottom
    };


    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}

static mesh make_corner_mesh() {
    vertex vertices[] = {
        { .p = vec3(-.5, -.5, 0.5) },
        { .p = vec3(0.5, -.5, -.5) },
        { .p = vec3(0.5, 0.5, 0.5) },
        { .p = vec3(0.5, -.5, 0.5) },
    };
    
    u32 indices[] = {
        0, 1, 2,
        0, 3, 1,
        0, 2, 3,
        1, 3, 2
    };
    
    mesh result = { .primitive = GL_TRIANGLES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}

static mesh mesh_cube_frame_mesh() {
    vertex vertices[8] = {
        { .p = vec3(-.5, -.5, -.5) },
        { .p = vec3(+.5, -.5, -.5) },
        { .p = vec3(+.5, +.5, -.5) },
        { .p = vec3(-.5, +.5, -.5) },
        { .p = vec3(-.5, -.5, +.5) },
        { .p = vec3(+.5, -.5, +.5) },
        { .p = vec3(+.5, +.5, +.5) },
        { .p = vec3(-.5, +.5, +.5) }
    };
    
    u32 indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, 1, 5, 5, 4, 4, 0, 2, 6, 6, 5, 6, 7, 7, 3, 4, 7
    };
    
    mesh result = { .primitive = GL_LINES, 
        .scale = vec3(1, 1, 1),
        .rotation = unit_quat(),
        .index_count = array_count(indices)
    };
    
    result.vao = make_vao(vertices, array_count(vertices), indices, result.index_count);
    return result;
}


static inline void init_framebuffer(framebuffer_info* framebuffer) {
    *framebuffer = (framebuffer_info) { 0 };
    glGenFramebuffers(1, &framebuffer->id);
}

typedef struct {
    u32 min_filter, mag_filter;
    u32 wrap_t, wrap_s;
} add_attachment_args;

#define framebuffer_add_attachment(framebuffer, type, width, height, ...) \
    _framebuffer_add_attachment(framebuffer, type, width, height, (add_attachment_args) { \
    .min_filter = GL_NEAREST, .mag_filter = GL_NEAREST, .wrap_t = GL_REPEAT, .wrap_s = GL_REPEAT, __VA_ARGS__ })
    
static framebuffer_attachment* _framebuffer_add_attachment(framebuffer_info* framebuffer, u32 type, int width, int height, add_attachment_args args) {
    framebuffer_attachment* result = 0;
    for (int i = 0; i < FRAMEBUFFER_ATTACHMENT_MAX_COUNT; i++) {
        if (framebuffer->attachments[i].id == 0) {
            result = &framebuffer->attachments[i];
            break;
        }
    }
    
    if (!result) { return 0; }
    
    result->type = type;
    result->width = width;
    result->height = height;
    
    glGenTextures(1, &result->id);
    glBindTexture(GL_TEXTURE_2D, result->id);
    // @Note: these need to be set for the texture to work :)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, args.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, args.mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, args.wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, args.wrap_t);
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
    
    switch (type) {
        case GL_COLOR_ATTACHMENT: {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            
            result->color_attachment_id = GL_COLOR_ATTACHMENT0 + (framebuffer->color_attachment_count++);
            glFramebufferTexture2D(GL_FRAMEBUFFER, result->color_attachment_id, GL_TEXTURE_2D, result->id, 0);
        } break;
        case GL_DEPTH_ATTACHMENT: {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, result->id, 0);
        } break;
        // case GL_STENCIL_ATTACHMENT: {} break;
        // case GL_DEPTH_STENCIL_ATTACHMENT: {} break;
        
        default: {
            report("Unsupported attachment type (%i) to add!\n", type);
            glDeleteTextures(1, &result->id);
            *result = (framebuffer_attachment) { 0 };
            result = 0;
        } break;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return result;
}

static void framebuffer_resize_all_attachments(framebuffer_info* framebuffer, int new_width, int new_height) {
    for (int i = 0; i < FRAMEBUFFER_ATTACHMENT_MAX_COUNT; i++) {
        framebuffer_attachment* attachment = &framebuffer->attachments[i];
        if (!attachment->id) { continue; }

        glBindTexture(GL_TEXTURE_2D, attachment->id);

        switch (attachment->type) {
            case GL_COLOR_ATTACHMENT: {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_width, new_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            } break;
            case GL_DEPTH_ATTACHMENT: {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, new_width, new_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            } break;
            
            // case GL_STENCIL_ATTACHMENT: {} break;
            // case GL_DEPTH_STENCIL_ATTACHMENT: {} break;
            
            default: { report("Unsupported attachment type (%i) to resize!\n", attachment->type); } break;
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void init_renderer(game_state* state) {
    renderer_info* renderer = &state->renderer;
    int window_w = state->platform->window_width;
    int window_h = state->platform->window_height;
    
    glClearColor(0., 0., 0.1, 1.0);
    
    // glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_SCISSOR_TEST);
    
    renderer->projection_matrix = make_projection_matrix(
        window_w,  
        window_h,
        state->current_camera->fov,
        state->current_camera->near,
        state->current_camera->far
    );
    
    renderer->line_mesh = make_line_mesh();
    renderer->quad_mesh = make_quad_mesh();
    renderer->cube_mesh = make_cube_mesh();
    renderer->cube_frame_mesh = mesh_cube_frame_mesh();

    init_framebuffer(&renderer->scene_framebuffer);

    renderer->scene_texture = framebuffer_add_attachment(&renderer->scene_framebuffer, 
        GL_COLOR_ATTACHMENT, window_w, window_h, .wrap_s = GL_CLAMP_TO_EDGE, .wrap_t = GL_CLAMP_TO_EDGE);
    renderer->scene_per_object_depth_texture = framebuffer_add_attachment(&renderer->scene_framebuffer,
        GL_COLOR_ATTACHMENT, window_w, window_h, .wrap_s = GL_CLAMP_TO_EDGE, .wrap_t = GL_CLAMP_TO_EDGE);
    renderer->scene_object_color_texture = framebuffer_add_attachment(&renderer->scene_framebuffer, 
        GL_COLOR_ATTACHMENT, window_w, window_h, .wrap_s = GL_CLAMP_TO_EDGE, .wrap_t = GL_CLAMP_TO_EDGE);
    renderer->scene_depth_texture = framebuffer_add_attachment(&renderer->scene_framebuffer, 
        GL_DEPTH_ATTACHMENT, window_w, window_h, .wrap_s = GL_CLAMP_TO_EDGE, .wrap_t = GL_CLAMP_TO_EDGE);
}

static inline void scissor(int x, int y, int w, int h) {
    glScissor(x, global->platform->window_height - y - h, w, h);
}

static inline void scissor_reset() {
    glScissor(0, 0, global->platform->window_width, global->platform->window_height);
}

static char* get_opengl_error_string(int code) {
    switch(code) {
        case 0x0500: {
            return "GL_INVALID_ENUM";
        } break;
        case 0x0501: {
            return "GL_INVALID_VALUE";
        } break;
        case 0x0502: {
            return "GL_INVALID_OPERATION";
        } break;
        case 0x0503: {
            return "GL_STACK_OVERFLOW";
        } break;
        case 0x0504: {
            return "GL_STACK_UNDERFLOW";
        } break;
        case 0x0505: {
            return "GL_OUT_OF_MEMORY";
        } break;
        case 0x0506: {
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        } break;
        case 0x0507 : {
            return "GL_CONTEXT_LOST";
        } break;
        case 0x8031: {
            return "GL_TABLE_TOO_LARGE";
        } break;
    };
    
    return "UNKNOWN";
}

// @Info: this set of functions use glProgramUniform instead of glUniform, so we don't
//        have to call glUseProgram if we want to set a uniform, which is handy in some cases
bool shader_set_int(shader_info* shader, char* name, int i) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniform1i(shader->id, location, i);
    return location != -1;
}

bool shader_set_uint(shader_info* shader, char* name, u32 i) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniform1ui(shader->id, location, i);
    return location != -1;
}

bool shader_set_float(shader_info* shader, char* name, float f) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniform1f(shader->id, location, f);
    return location != -1;
}

bool shader_set_vec2(shader_info* shader, char* name, vec2 v) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniform2fv(shader->id, location, 1, (float*)&v.elements);
    return location != -1;
}
bool shader_set_vec3(shader_info* shader, char* name, vec3 v) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniform3fv(shader->id, location, 1, (float*)&v.elements);
    return location != -1;
}

bool shader_set_vec4(shader_info* shader, char* name, vec4 v) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniform4fv(shader->id, location, 1, (float*)&v.elements);
    return location != -1;
}

bool shader_set_mat4(shader_info* shader, char* name, mat4 m) {
    int location = glGetUniformLocation(shader->id, name);
    glProgramUniformMatrix4fv(shader->id, location, 1, GL_FALSE, (float*)&m.elements);
    return location != -1;
}

u32 compile_shader(string source, u32 type) {
    u32 result = glCreateShader(type);
    
    glShaderSource(result, 1, &source.data, &source.length);
    glCompileShader(result);
    
    int compiled;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        u8 info[1024];
        
        glGetShaderInfoLog(result, 1024, 0, info);
        report("Failed to compile shader: %s\n", info);

        return 0;
    }

    return result;
}

/*
    === load shader ===
*/
u32 load_shader(string source) {
    string vertex_marker = string("::vertex");
    string fragment_marker = string("::fragment");
    string geometry_marker = string("::geometry");
    
    string cursor = string_after_first(source, ':');
    
    u32 vertex_id = 0, fragment_id = 0, geometry_id = 0;
    
    while (cursor.length) {
        if (string_begins_with(cursor, vertex_marker)) {
            string_eat_line(&cursor);
            string vertex_source = string_eat_to_first(&cursor, ':');
            
            vertex_id = compile_shader(vertex_source, GL_VERTEX_SHADER);
        
        } else if (string_begins_with(cursor, fragment_marker)) {
            string_eat_line(&cursor);
            string fragment_source = string_eat_to_first(&cursor, ':');
            
            fragment_id = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
        
        } else if (string_begins_with(cursor, geometry_marker)) {
            string_eat_line(&cursor);
            string geometry_source = string_eat_to_first(&cursor, ':');
            
            geometry_id = compile_shader(geometry_source, GL_GEOMETRY_SHADER);
        
        } else {
            string_eat_line(&cursor);
        }
    }
    
    if (!vertex_id || !fragment_id) { return 0; }
    
    u32 result = glCreateProgram();
    
    glAttachShader(result, vertex_id);
    glAttachShader(result, fragment_id);
    if (geometry_id) { glAttachShader(result, geometry_id); }
    
    glLinkProgram(result);
    
    int linked;
    glGetProgramiv(result, GL_LINK_STATUS, &linked);
    
    if (!linked) {
        u8 info[1024];
        glGetProgramInfoLog(result, 1024, NULL, info);
        report("Failed to link shader: %s\n", info);
        
        result = 0;
    }
    
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
    
    return result;
}

void reload_shader(string path, void* _shader) {
    shader_info* shader = _shader;

    assert(string_compare(path, shader->path));
    string shader_source = read_file(shader->path, &global->transient_arena);
    
    u32 id = load_shader(shader_source);
    if (id) {
        glDeleteProgram(shader->id);
        shader->id = id;
        
        report("Reloaded shader \"%.*s\"\n", shader->name.length, shader->name.data);
        
        string msg = string_buffer(128);
        string_write(&msg, "Reloaded shader \"");
        string_write(&msg, shader->name);
        string_write(&msg, "\"");
        
        report_ingame(msg);
    } else {
        report("Failed to reload shader \"%.*s\"\n", shader->name.length, shader->name.data);
        
        string msg = string_buffer(128);
        string_write(&msg, "Failed to reload shader \"");
        string_write(&msg, shader->name);
        string_write(&msg, "\"");
        
        report_ingame(msg);
    }
}

void load_all_shaders(game_state* state, char* shader_dir) {
    u8* file_names = push_size(&state->transient_arena, 0);
    
    u64 bytes_used;
    int file_count = platform_find_all_files(shader_dir, "*.glsl", file_names, &bytes_used);
    
    push_size(&state->transient_arena, bytes_used);
    
    shader_catalog* catalog = &state->shaders;

    catalog->shaders = push_size(&state->permanent_arena, sizeof(shader_info) * file_count);
    catalog->count = file_count;
    
    string shader_path = string_buffer(256);
    string_write(&shader_path, shader_dir);
    
    int shader_dir_length = shader_path.length;

    int failed = 0;
    for (int i = 0; i < file_count; i++) {
        shader_info* shader = &catalog->shaders[i];
        
        string_write(&shader_path, file_names);
        
        shader->path = push_string(&state->permanent_arena, shader_path.length);
        string_copy(&shader->path, shader_path);
        shader->name = get_file_name_from_path(shader->path);
        
        string shader_source = read_file(shader->path, &state->transient_arena);
        shader->id = load_shader(shader_source);
        
        if (!shader->id) {
            failed++;
            report("Failed to load shader \"%*s\"\n", shader->name.length, shader->name.data);
        }
        
        shader_path.length = shader_dir_length;
        
        platform_add_file_watch(shader->path, reload_shader, shader);
        
        while(*file_names++) {}
    }

    report("%i/%i shaders loaded\n", catalog->count, file_count);
}

shader_info* get_shader(char* _name) {
    string name = { .data = _name, .length = c_string_length(_name), .size = name.length };
    for (int i = 0; i < global->shaders.count; i++) {
        shader_info* shader = &global->shaders.shaders[i];
        if (string_compare(name, shader->name)) {
            return shader;
        }
    }
    
    return 0;
}


/*
    === load texture ===
*/
u32 load_texture(char* path, int* w, int* h) {
    u32 result = 0;
    
    stbi_set_flip_vertically_on_load(1);
    int channels;
    u8* data = stbi_load(path, w, h, &channels, 0);
    
    if (!data) {
        stbi_image_free(data);
        printf("Failed to load image file \"%s\"\n", path);
        return result;
    }
    
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    if (channels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *w, *h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else if (channels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *w, *h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
        printf("Unknown number of color channels (%i) in image file %s\n", channels, path);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return result;
}

void reload_texture(string path, void* _texture) { 
    texture_info* texture = _texture;
    assert(string_compare(path, texture->path));
    
    platform_sleep(100);
    
    int w, h;
    u32 id = load_texture(path.data, &w, &h);
    
    if (id) {
        glDeleteTextures(1, &texture->id);
        
        texture->id = id;
        texture->w = w;
        texture->h = h;
        
        report("Reloaded texture \"%.*s\"\n", texture->name.length, texture->name.data);
    } else {
        report("Failed to reload texture \"%.*s\"\n", texture->name.length, texture->name.data);
    }
}

void load_all_textures(game_state* state, char* texture_dir) {
    u8* file_names = push_size(&state->transient_arena, 0);
    
    u64 bytes_used;
    int file_count = platform_find_all_files(texture_dir, "*.png", file_names, &bytes_used);
    
    push_size(&state->transient_arena, bytes_used);
    
    texture_catalog* catalog = &state->textures;
    catalog->textures = push_size(&state->permanent_arena, sizeof(texture_info) * file_count);
    catalog->count = file_count;
    
    string texture_path = string_buffer(256);
    string_write(&texture_path, texture_dir);
    
    int texture_dir_length = texture_path.length;
    
    int failed = 0;
    for (int i = 0; i < file_count; i++) {
        texture_info* texture = &catalog->textures[i];
        
        string_write(&texture_path, file_names);
        
        // @Note: paths are null-terminated for stbi.h
        texture->path = push_string(&state->permanent_arena, texture_path.length + 1);
        string_copy(&texture->path, texture_path);
        texture->path.data[texture_path.length] = 0;
        texture->name = get_file_name_from_path(texture->path);
        
        texture->id = load_texture(texture->path.data, &texture->w, &texture->h);
        
        if (!texture->id) {
            failed++;
            printf("Failed to load texture \"%*s\"\n", texture->name.length, texture->name.data);
        }
    
        platform_add_file_watch(texture->path, reload_texture, texture);
        
        texture_path.length = texture_dir_length;
        
        while(*file_names++) {}
    }
    
    printf("%i/%i textures loaded\n", catalog->count - failed, catalog->count);
}

texture_info* get_texture(char* _name) {
    string name = { .data = _name, .length = c_string_length(_name), .size = name.length };
    for (int i = 0; i < global->textures.count; i++) {
        texture_info* texture = &global->textures.textures[i];
        if (string_compare(name, texture->name)) {
            return texture;
        }
    }
    
    return 0;
}

typedef struct {
    u32 target;
} bind_texture_args;
#define shader_bind_texture(shader, texture, name, index, ...)\
    _shader_bind_texture(shader, (texture)->id, name, index,\
        (bind_texture_args){ .target = GL_TEXTURE_2D, __VA_ARGS__ })
void _shader_bind_texture(shader_info* shader, u32 id, char* name, u32 index, bind_texture_args args) {
    int loc = glGetUniformLocation(shader->id, name);
    
    glUniform1i(loc, index);
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(args.target, id);
}

/*
    === render ui ===
*/
static void ui_quad(int x, int y, int w, int h, color c) {
    float window_w = global->platform->window_width;
    float window_h = global->platform->window_height;
    
    vec2 scale = vec2(w / window_w, h / window_h);
    
    // @Info: (_x, _y) is the center of the quad    
    float _x = x + w / 2.0;
    float _y = y + h / 2.0;
    
    vec2 offset = screen_to_ndc(vec2(_x, _y));
    
    shader_info* shader = get_shader("ui_quad");
    glUseProgram(shader->id);
    
    shader_set_uniform(shader, "offset", offset);
    shader_set_uniform(shader, "scale", scale);
    shader_set_uniform(shader, "color", c);
    
    glBindVertexArray(global->renderer.quad_mesh.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

typedef struct {
    quat rotation;
    shader_info* shader;
} ui_quad_textured_args;
#define ui_quad_textured(x, y, w, h, tex, ...) \
    _ui_quad_textured(x, y, w, h, tex, (ui_quad_textured_args) { .shader = 0, .rotation = unit_quat(), __VA_ARGS__ })

static void _ui_quad_textured(int x, int y, int w, int h, u32 texture_id, ui_quad_textured_args args) {
    float window_w = global->platform->window_width;
    float window_h = global->platform->window_height;
    
    vec2 scale = vec2(w / window_w, h / window_h);
    
    // @Info: (_x, _y) is the center of the quad    
    float _x = x + w / 2.0;
    float _y = y + h / 2.0;
    
    vec2 offset = screen_to_ndc(vec2(_x, _y));
    
    if (!args.shader) { args.shader = get_shader("ui_quad_textured"); }
    glUseProgram(args.shader->id);
    
    shader_set_uniform(args.shader, "offset", offset);
    shader_set_uniform(args.shader, "scale", scale);
    shader_set_uniform(args.shader, "rotation", quat_to_mat(args.rotation));
    _shader_bind_texture(args.shader, texture_id, "tex", 0, (bind_texture_args) { .target = GL_TEXTURE_2D });
    
    glBindVertexArray(global->renderer.quad_mesh.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
}


/*
    === render mesh ===
*/
typedef struct {
    vec3 translation;
    vec3 scale_v;
    quat rotation;
    float scale;
    color color;
    float normal_factor;
} render_mesh_args;
#define render_mesh_basic(mesh, ...) _render_mesh_basic(mesh, (render_mesh_args) {\
    .translation = vec3(0, 0, 0), .scale_v = vec3(1, 1, 1), .rotation = unit_quat(), .scale = 1.,\
    .color = (color)RGB(57, 255, 20), .normal_factor = 1.,\
    __VA_ARGS__ })

static void _render_mesh_basic(mesh m, render_mesh_args args) {
    shader_info* shader = get_shader("game_object");
    glUseProgram(shader->id);
    
    vec3 translation = vec_add(args.translation, m.translation);
    
    mat4 model = make_model_matrix(translation, 
                                   vec_mul(vec_mul(args.scale_v, args.scale), m.scale), 
                                   quat_mul_quat(args.rotation, m.rotation));
    mat4 view = global->current_camera->view_matrix;
    mat4 proj = global->renderer.projection_matrix;
    
    shader_set_uniform(shader, "model", model);
    shader_set_uniform(shader, "view", view);
    shader_set_uniform(shader, "projection", proj);
    shader_set_uniform(shader, "color", args.color);
    shader_set_uniform(shader, "normal_factor", args.normal_factor);
    
    shader_set_uniform(shader, "translation", translation);
    
    glBindVertexArray(m.vao);
    glDrawElements(m.primitive, m.index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

static void render_mesh(mesh m, shader_info* shader) {
    glUseProgram(shader->id);
    
    mat4 model = make_model_matrix(m.translation, m.scale, m.rotation);
    mat4 view = global->current_camera->view_matrix;
    mat4 proj = global->renderer.projection_matrix;
    
    shader_set_uniform(shader, "model", model);
    shader_set_uniform(shader, "view", view);
    shader_set_uniform(shader, "projection", proj);
    
    glBindVertexArray(m.vao);
    glDrawElements(m.primitive, m.index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

static void debug_render_quad(vec3 p0, vec3 p1, color c) {
    shader_info* shader = get_shader("debug_quad");
    glUseProgram(shader->id);
    
    mesh m = global->renderer.line_mesh;

    mat4 view = global->current_camera->view_matrix;
    mat4 proj = global->renderer.projection_matrix;

    shader_set_uniform(shader, "view", view);
    shader_set_uniform(shader, "projection", proj);
    shader_set_uniform(shader, "color", c);
    
    shader_set_uniform(shader, "p0", p0);
    shader_set_uniform(shader, "p1", p1);
    
    glBindVertexArray(m.vao);
    glDrawElements(m.primitive, m.index_count, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

