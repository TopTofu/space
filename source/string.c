#pragma once

typedef unsigned char bool;
#define true 1
#define false 0

typedef struct string {
    char* data;
    unsigned int length;
    unsigned int size;
} string;

// macro to pick the right macro depending on how many arguments we got
#define GET_ARGUMENT_COUNT_SPECIFIC_MACRO(_1, _2, NAME, ...) NAME

/*
Create a string from either another string or from a null-terminated char*. 
Optional second parameter is the length of the string.
If the first argument is a expression like 'some_string.data + 8', a length must also be given, since it can't be inferred.
Keep in mind that there is no copying of any string data, only the data pointer and potenially the length are copied over, so strings may overlap.
*/
#define string(...) GET_ARGUMENT_COUNT_SPECIFIC_MACRO(__VA_ARGS__,\
                                                      _string_with_length,\
                                                      _string_without_length)\
                                                      (__VA_ARGS__)
                                                      
#define _string_without_length(STR)         _string_with_length(STR, _string_length(STR))

#define _string_with_length(STR, LENGTH)    _Generic((STR), char*:       _string_from_c_str,\
                                                            const char*: _string_from_c_str,\
                                                            string:      _string_from_string)(STR, LENGTH)

#define _string_length(STR)  _Generic((STR), char*:      c_string_length,\
                                            const char*: c_string_length,\
                                            string:      string_length) (STR)

#define string_buffer(LENGTH) (string){ .data=(char[LENGTH]){0}, .length=0, .size=LENGTH }

#define string_is_valid(STR) ((STR).data != 0 && (STR).size != 0)
#define string_is_empty(STR) ((STR).length == 0)

unsigned int string_length(string str) { return str.length; }

string _string_from_string(string str, unsigned int length) {
    return (string){.data=str.data, .length=length, .size=length};
}

string _string_from_c_str(char* data, unsigned int length) {
    return (string){.data=data, .length=length, .size=length+1};
}

bool string_n_compare(string a, string b, unsigned int n) {
    if (a.length < n || b.length < n) { return false; }
    while(n--) {
        if (a.data[n] != b.data[n]) { return false; }
    }
    return true;
}


// ===============
// parsing utils
#define force_upper_case(C) ((C) & ~32)
#define force_lower_case(C) ((C) |  32)
#define is_white_space(C)   ((C) == ' ' || (C) == '\t')
#define is_alpha(C)         (((C) >= 'A' && (C) <= 'Z') || ((C) >= 'a' && (C) <= 'z'))
#define is_num(C)           ((C) >= '0' && (C) <= '9')
#define is_alphanum(C)      (is_alpha(C) || is_num(C))

bool char_can_be_number(char c, int base) {
    switch(base) {
        case 2: {
            return (c == '1') || (c == '0');
        } break;
        case 10: {
            return (c >= '0') && (c <= '9');
        } break;
        case 16: {
            bool is_digit      = (c >= '0') && (c <= '9');
            bool is_upper_case = (c >= 'A') && (c <= 'F');
            bool is_lower_case = (c >= 'a') && (c <= 'f');
            
            return is_digit || is_upper_case || is_lower_case;
        }
    }
    
    return false;
}

static inline void string_increment(string* str) {
    str->data++;
    str->length--;
    str->size--;
}

static inline void string_increment_n(string* str, int n) {
    str->data += n;
    str->length -= n;
    str->size -= n;
}

static inline void string_eat_white_spaces(string* str) {
    while (is_white_space(str->data[0])) { string_increment(str); }
}


bool string_compare(string a, string b) {
    if (a.length != b.length) { return false; }
    return string_n_compare(a, b, a.length);
}

bool string_begins_with(string a, string b) {
    if (a.length < b.length) { return false; }
    return string_n_compare(a, b, b.length);
}

unsigned int c_string_length(char* c_str) {
    unsigned int length = 0;
    while(c_str[length]) { length++; }
    return length;
}

void memory_copy(char* dest, char* src, unsigned int size) {
    while (size--) { *dest++ = *src++; }
}

bool string_n_copy(string dest, string src, unsigned int n) {
    if (dest.length < n || src.length < n) { return false; }
    memory_copy(dest.data, src.data, n);
    return true;
}

bool string_copy(string* dest, string src) {
    if (dest->size < src.length) { return false; }
    memory_copy(dest->data, src.data, src.length);
    dest->length = src.length;
    return true;
}

char* to_c_str(string str, void* (*alloc)(size_t)) {
    char* result = alloc(str.length+1);
    memory_copy(result, str.data, str.length);
    result[str.length] = '\0';
    return result;
}

int string_count_occurence(string str, char x) {
    int count = 0;
    for (int i = 0; i < str.length; i++) { 
        if(str.data[i] == x) { count++; }
    }
    return count;
}

// does not include c
string string_eat_to_first(string* str, char c) {
    string result = { .data=str->data };
    int length = str->length;
    for (int i = 0; i < length; i++) {
        if (str->data[0] == c) {
            break;
        }
        
        string_increment(str);
        result.length++;    
    }
    
    return result;
}

static inline string string_peek_to_first(string str, char c) {
    return string_eat_to_first(&str, c);
}

// includes c
string string_after_first(string str, char c) {
    string result = {0};
    for (int i = 0; i < str.length; i++) {
        if (str.data[i] == c) {
            result.data = str.data + i;
            result.length = str.length - i;
            result.size = result.length;
            break;
        }
    }

    return result;
}

// includes c
string string_until_first(string str, char c) {
    string result = {0};
    for (int i = 0; i < str.length; i++) {
        if (str.data[i] == c) {
            result.data = str.data;
            result.length = i + 1; // include c
            result.size = result.length;
            break;
        }
    }
    return result;
}

string string_after_last(string str, char c) {
    string result = {0};
    for (int i = str.length - 1; i >= 0; i--) {
        if (str.data[i] == c) {
            result.data = str.data + i;
            result.length = str.length - i;
            result.size = result.length;
            break;
        }
    }

    return result;
}

string string_until_last(string str, char c) {
    string result = {0};
    for (int i = str.length - 1; i >= 0; i--) {
        if (str.data[i] == c) {
            result.data = str.data;
            result.length = i + 1; // include c
            result.size = result.length;
            break;
        }
    }

    return result;
}

string get_file_name_from_path(string path) {
    string result = {0};
    int start = path.length;
    while (path.data[start] != '/' && start > 0) { start--; }
    if (start > 0) { start++; } // remove the '/'

    int end = start;
    while (path.data[end] != '.' && end < path.length) { end++; }

    result.length = end - start;
    result.size = result.length;
    result.data = path.data + start;
    
    return result;
}

bool string_starts_with(string str, string start) {
    return string_n_compare(str, start, start.length);
}

bool string_next_line(string* str) { // @Todo: remove
    while (str->data) {
        if (str->data[0] == '\n') {
            str->data++;
            str->length--;
            return true;
        }
        str->data++;
        str->length--;
    }

    return false;
}

// ===============
// parsing
long long string_eat_int(string* str, int base) {
    long long result = 0;
    bool is_negative = false;
    
    string_eat_white_spaces(str);

    if (str->data[0] == '-') {
        is_negative = true; // @Todo just set the sign bit
        string_increment(str);
    } else if (str->data[0] == '+') {
        string_increment(str);
    }
    
    char c = str->data[0];
    while(char_can_be_number(c, base) && str->length > 0) {
        result *= base;
        
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else {
            digit = force_upper_case(c) - 55; // offset so that A -> 10, B->11, ...
        }
        
        result += digit;
        
        string_increment(str);
        c = str->data[0];
    }
    
    if (is_negative) { result = -result; }
    
    return result;
}

static inline long long string_peek_int(string str, int base) {
    return string_eat_int(&str, base);
}

double string_eat_double(string* str) {
    if (str->length == 0) { return 0; }
    char* end;
    
    // @Todo: This needs to go.
    double result = strtod(str->data, &end);
    
    str->length = str->length - (end - str->data);
    str->data = end;
    
    return result;
}

// @TODO string_eat_double()
static inline double string_peek_double(string str) {
    return string_eat_double(&str);
    // if (str.length == 0) { return 0; }
    // return atof(str.data);
}

string string_eat(string* str, int count) {
    string result = { .data=str->data, .length=count, .size=count };
    str->data += count;
    str->length -= count;
    
    return result;
}

static inline string string_peek(string str, int count) {
    return string_eat(&str, count);
}

// result will not include the '\n'
string string_eat_line(string* str) {
    string result = { .data=str->data };
    
    int max_length = str->length;
    
    while (str->data[0] != '\n' && result.length < max_length) {
        string_increment(str);
        result.length++;
    }
    
    if (str->data[0] == '\n') {
        string_increment(str);
    }
    
    result.size = result.length;
    
    return result;
}

static inline string string_peek_line(string str) {
    return string_eat_line(&str);
}

string string_eat_token(string* str, char delimit) {
    // skipping white spaces may cause issues with empty tokens when the delimiter is a white space
    if (!is_white_space(delimit)) {
        string_eat_white_spaces(str);
    }

    string result = { .data=str->data };
    int length = str->length;
    
    for (int i = 0; i < length; i++) {
        if (str->data[0] == delimit) {
            string_increment(str); // remove delimiter
            break;
        }

        string_increment(str);
        result.length++;
    }

    result.size = result.length;
    
    return result;
}

static inline string string_peek_token(string str, char delimit) {
    return string_eat_token(&str, delimit);
}


// ===== binary file parsing utils =====
static inline unsigned long long string_read(string* str, int count) {
    unsigned long long result;
    memory_copy((char*)&result, str->data, count);
    string_increment_n(str, count); 
    return result; 
}


// ===============
// string write
typedef struct {
    int base;
    int prec;       // decimal place count
    unsigned int minimum_digit_count;
} string_write_args;

#define string_write(S, X, ...) _Generic((X), \
                                  char:             string_write_char,\
                                  unsigned char:    string_write_char,\
                                  int:              string_write_int,\
                                  unsigned int:     string_write_int,\
                                  long long:        string_write_int,\
                                  unsigned long long: string_write_int,\
                                  void*:            string_write_pointer,\
                                  char*:            string_write_c_string,\
                                  unsigned char*:   string_write_c_string,\
                                  float:            string_write_double,\
                                  double:           string_write_double,\
                                  string:           string_write_string)\
        (S, X, (string_write_args){.base = 10, .prec = 5, .minimum_digit_count = 0, __VA_ARGS__})

const char* decimals = "0123456789ABCDEF";
int string_write_int(string* s, long long x, string_write_args args) {
    if (args.base < 2 || args.base > 16) return 0;
    
    bool is_negative = (x < 0);
    x = ABS(x);
    
    float log_x = (x == 0) ? 0 : log(x);
    int digit_count = FLOOR(log_x / log(args.base)) + 1;
    
    int padding_zero_count = MAX(0, (int)args.minimum_digit_count - digit_count);

    digit_count += padding_zero_count;

    if (is_negative) { digit_count++; }
    
    // check for sufficient space
    if (s->size - s->length < digit_count) return 0;
    
    int index = digit_count;
    char* buffer = s->data + s->length;
    
    for (int i = 0; i < padding_zero_count; i++) {
        buffer[i] = '0';
    }
    
    if (x == 0) { 
        buffer[--index] = decimals[0];
    } else {
        while(x) {
            int d = x % args.base;
            x = x / args.base;
            
            buffer[--index] = decimals[d];
        }
    }
    
    if (is_negative) { buffer[0] = '-'; }
    
    s->length += digit_count;
    
    return digit_count;
}

int string_write_pointer(string* s, void* p, string_write_args args) {
    string temp = { s->data, s->length + 2, s->size - 2 };
    int written = string_write_int(&temp, (unsigned long long)p, 
            (string_write_args) { .base = 16 });

    if (!written) { return 0; }
    s->data[s->length] = '0';
    s->data[s->length+1] = 'x';
    
    s->length = temp.length;
    
    return written + 2;    
}

int string_write_c_string(string* s, char* c, string_write_args args) {
    int length = c_string_length(c);
    if (s->size - s->length < length) { return 0; }
    
    memory_copy(s->data + s->length, c, length);
    s->length += length;
    
    return length;
}

int string_write_char(string* s, char c, string_write_args args) {
    if (s->size - s->length < 1) { return 0; }
    
    s->data[s->length++] = c;
    
    return 1;
}

int string_write_double(string* s, double x, string_write_args args) {
    // @Todo check for sufficient space
    int ipart = (int)x;
    double fpart = ABS(x - ipart);
    
    // print the '-' instead of in string_write_int() to cover -0
    if (x < 0) {
        s->data[s->length++] = '-';
    }
    
    int written = string_write_int(s, ABS(ipart), args);
    
    if (!written) { return 0; }

    if (args.prec) {
        string_write_char(s, '.', args);
        written += args.prec;
        
        fpart *= pow(10, args.prec); // @Todo get rid of this pow call
        
        if (fpart != 0) { args.minimum_digit_count = args.prec; }
        
        string_write_int(s, fpart, args);
    }

    return written;
}

int string_write_string(string* s, string append, string_write_args args) {
    if (s->size - s->length < append.length) { return 0; }
    
    memory_copy(s->data + s->length, append.data, append.length);
    s->length += append.length;
    
    return append.length;
}

int string_write_bool(string* s, bool b) {
    if (b) {
        return string_write(s, string("true"));
    } else {
        return string_write(s, string("false"));
    }
}

static inline void string_clear(string* s) {
    s->length = 0;
}

// ===============
// quick way to get an invalid string
#define empty_string() empty_string
string empty_string = { 0 };

// ===============
// string print
#define string_print(STR, ...) _string_print(STR, #STR, (print_string_args){.new_line=true, __VA_ARGS__}) 

typedef struct {
    bool new_line;
} print_string_args;

void _string_print(string str, char* name, print_string_args args) {
    printf("%s = \"%.*s\" [%i (size = %i)]", name, str.length, str.data, str.length, str.size);
    if (args.new_line) {
        printf("\n");
    }
}


// ===============
// string builder
typedef struct string_builder_node {
    char* data;
    unsigned int length;
    struct string_builder_node* next;
} string_builder_node;

typedef struct {
    string_builder_node* front;
    string_builder_node* end;
    unsigned int node_count;
    unsigned int total_length;
    
    void* (*allocator)(size_t);
    void  (*free)(void*);
} string_builder;

string_builder make_string_builder(void* (*allocator)(size_t), void (*free)(void*)) {
    string_builder result;
    
    result.front = result.end = 0;
    result.node_count = 0;
    result.total_length = 0;
    
    result.allocator = allocator;
    result.free = free;
    
    return result;
}

#define string_builder_append(BUF, STR)       _string_builder_append(BUF, string(STR))
#define string_builder_append_front(BUF, STR) _string_builder_append_front(BUF, string(STR))

void _string_builder_append(string_builder* builder, string str) {
    string_builder_node* node = builder->allocator(sizeof(string_builder_node));
    
    node->data = str.data;
    node->length = str.length;
    node->next = 0;
    
    if (!builder->end) {
        builder->end = builder->front = node;
    } else {
        builder->end->next = node;
        builder->end = node;
    }
    
    builder->node_count++;
    builder->total_length += str.length;
}

void _string_builder_append_front(string_builder* builder, string str) {
    string_builder_node* node = builder->allocator(sizeof(string_builder_node));
    
    node->data = str.data;
    node->length = str.length;
    node->next = builder->front;
    
    builder->front = node;
    
    builder->node_count++;
    builder->total_length += str.length;
}

// @Note: this also clears
string string_builder_flush(string_builder* builder) {
    string result;
    result.data = builder->allocator(builder->total_length + 1);
    
    string_builder_node* node = builder->front;
    unsigned int current_length = 0;
    while (node) {
        memory_copy(result.data + current_length, node->data, node->length);
        current_length += node->length;
        
        string_builder_node* temp = node;
        node = node->next;
        builder->free(temp);
    }
    
    result.data[current_length] = '\0';
    result.length = builder->total_length;
    result.size = builder->total_length;
    
    *builder = make_string_builder(builder->allocator, builder->free);
    
    return result;
}

void string_builder_clear(string_builder* builder) {
    string_builder_node* node = builder->front;
    while (node) {
        string_builder_node* temp = node;
        node = node->next;
        builder->free(temp);
    }
    
    *builder = make_string_builder(builder->allocator, builder->free);
}
