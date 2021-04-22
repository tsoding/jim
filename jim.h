#ifndef JIM_H_
#define JIM_H_

#include <assert.h>
#include <string.h>

#ifndef JIM_STACK_CAPACITY
#define JIM_STACK_CAPACITY 1024
#endif // JIM_STACK_CAPACITY

typedef void* Jim_Sink;
typedef size_t (*Jim_Write)(const void *ptr, size_t size, size_t nmemb, Jim_Sink sink);

typedef enum {
    JIM_OK = 0,
    JIM_WRITE_ERROR,
    JIM_STACK_OVERFLOW,
    JIM_STACK_UNDERFLOW
} Jim_Error;

const char *jim_error_string(Jim_Error error);

typedef struct {
    Jim_Sink sink;
    Jim_Write write;
    Jim_Error error;
    size_t stack[JIM_STACK_CAPACITY];
    size_t stack_size;
} Jim;

void jim_begin(Jim *jim);
void jim_end(Jim *jim);

void jim_null(Jim *jim);
void jim_bool(Jim *jim, int boolean);
void jim_integer(Jim *jim, long long int x);
void jim_float(Jim *jim, double x);
void jim_string(Jim *jim, const char *str, const unsigned int *size);

void jim_array_begin(Jim *jim);
void jim_element_begin(Jim *jim);
void jim_element_end(Jim *jim);
void jim_array_end(Jim *jim);

void jim_object_begin(Jim *jim);
void jim_member_begin(Jim *jim);
void jim_member_key(Jim *jim, const char *str, const unsigned int *size);
void jim_member_value_begin(Jim *jim);
void jim_member_value_end(Jim *jim);
void jim_member_end(Jim *jim);
void jim_object_end(Jim *jim);

#endif // JIM_H_

#ifdef JIM_IMPLEMENTATION

static void jim_stack_push(Jim *jim)
{
    if (jim->error == JIM_OK) {
        if (jim->stack_size < JIM_STACK_CAPACITY) {
            jim->stack[jim->stack_size++] = 0;
        } else {
            jim->error = JIM_STACK_OVERFLOW;
        }
    }
}

static void jim_stack_pop(Jim *jim)
{
    if (jim->error == JIM_OK) {
        if (jim->stack_size > 0) {
            jim->stack_size--;
        } else {
            jim->error = JIM_STACK_UNDERFLOW;
        }
    }
}

static size_t *jim_stack_top(Jim *jim)
{
    if (jim->error == JIM_OK) {
        if (jim->stack_size > 0) {
            return &jim->stack[jim->stack_size - 1];
        } else {
            jim->error = JIM_STACK_UNDERFLOW;
        }
    }

    return NULL;
}

static void jim_write(Jim *jim, const char *buffer, size_t size)
{
    if (jim->error == JIM_OK) {
        if (jim->write(buffer, 1, size, jim->sink) < size) {
            jim->error = 1;
        }
    }
}

static void jim_write_cstr(Jim *jim, const char *cstr)
{
    if (jim->error == JIM_OK) {
        jim_write(jim, cstr, strlen(cstr));
    }
}

static int jim_get_utf8_char_len(unsigned char ch)
{
    if ((ch & 0x80) == 0) return 1;
    switch (ch & 0xf0) {
    case 0xf0:
        return 4;
    case 0xe0:
        return 3;
    default:
        return 2;
    }
}

const char *jim_error_string(Jim_Error error)
{
    switch (error) {
    case JIM_OK:
        return "There is no error. The developer of this software just had a case of \"Task failed successfully\" https://i.imgur.com/Bdb3rkq.jpg - Please contact the developer and tell them that they are very lazy for not checking errors properly.";
    case JIM_WRITE_ERROR:
        return "Write error";
    case JIM_STACK_OVERFLOW:
        return "Stack Overflow";
    case JIM_STACK_UNDERFLOW:
        return "Stack Underflow";
    default:
        return NULL;
    }
}

void jim_begin(Jim *jim)
{
    (void) jim;
}

void jim_end(Jim *jim)
{
    (void) jim;
}

void jim_null(Jim *jim)
{
    if (jim->error == JIM_OK) {
        jim_write_cstr(jim, "null");
    }
}

void jim_bool(Jim *jim, int boolean)
{
    if (jim->error == JIM_OK) {
        if (boolean) {
            jim_write_cstr(jim, "true");
        } else {
            jim_write_cstr(jim, "false");
        }
    }
}

void jim_integer(Jim *jim, long long int x)
{
    assert(0 && "TODO: jim_integer is not implemented yet");
}

void jim_float(Jim *jim, double x)
{
    assert(0 && "TODO: jim_float is not implemented yet");
}

void jim_string(Jim *jim, const char *str, const unsigned int *size)
{
    if (jim->error == JIM_OK) {
        const char *hex_digits = "0123456789abcdef";
        const char *specials = "btnvfr";
        const char *p = str;
        size_t len = size ? *size : strlen(str);

        jim_write_cstr(jim, "\"");
        size_t cl;
        for (size_t i = 0; i < len; i++) {
            unsigned char ch = ((unsigned char *) p)[i];
            if (ch == '"' || ch == '\\') {
                jim_write(jim, "\\", 1);
                jim_write(jim, p + i, 1);
            } else if (ch >= '\b' && ch <= '\r') {
                jim_write(jim, "\\", 1);
                jim_write(jim, &specials[ch - '\b'], 1);
            } else if (0x20 <= ch && ch <= 0x7F) { // is printable
                jim_write(jim, p + i, 1);
            } else if ((cl = jim_get_utf8_char_len(ch)) == 1) {
                jim_write(jim, "\\u00", 4);
                jim_write(jim, &hex_digits[(ch >> 4) % 0xf], 1);
                jim_write(jim, &hex_digits[ch % 0xf], 1);
            } else {
                jim_write(jim, p + i, cl);
                i += cl - 1;
            }
        }

        jim_write_cstr(jim, "\"");
    }
}

void jim_array_begin(Jim *jim)
{
    if (jim->error == JIM_OK) {
        jim_write_cstr(jim, "[");
        jim_stack_push(jim);
    }
}

void jim_element_begin(Jim *jim)
{
    if (jim->error == JIM_OK) {
        size_t *top = jim_stack_top(jim);
        if (top && *top > 0) {
            jim_write_cstr(jim, ",");
        }
    }
}

void jim_element_end(Jim *jim)
{
    if (jim->error == JIM_OK) {
        size_t *top = jim_stack_top(jim);
        if (top) {
            *top += 1;
        }
    }
}

void jim_array_end(Jim *jim)
{
    if (jim->error == JIM_OK) {
        jim_write_cstr(jim, "]");
        jim_stack_pop(jim);
    }
}

void jim_object_begin(Jim *jim)
{
    if (jim->error == JIM_OK) {
        jim_write_cstr(jim, "{");
        jim_stack_push(jim);
    }
}

void jim_member_begin(Jim *jim)
{
    if (jim->error == JIM_OK) {
        size_t *top = jim_stack_top(jim);
        if (top && *top > 0) {
            jim_write_cstr(jim, ",");
        }
    }
}

void jim_member_key(Jim *jim, const char *str, const unsigned int *size)
{
    if (jim->error == JIM_OK) {
        jim_string(jim, str, size);
        jim_write_cstr(jim, ":");
    }
}

void jim_member_end(Jim *jim)
{
    if (jim->error == JIM_OK) {
        size_t *top = jim_stack_top(jim);
        if (top) {
            *top += 1;
        }
    }
}

void jim_object_end(Jim *jim)
{
    if (jim->error == JIM_OK) {
        jim_write_cstr(jim, "}");
        jim_stack_pop(jim);
    }
}

#endif // JIM_IMPLEMENTATION
