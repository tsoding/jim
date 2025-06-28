// Prototype of an Immediate Deserialization idea.
#ifndef JIMP_H_
#define JIMP_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// TODO: move all diagnostics reporting outside of the library
//   So the user has more options on how to report things

typedef enum {
    JIMP_INVALID,
    JIMP_EOF,

    // Puncts
    JIMP_OCURLY,
    JIMP_CCURLY,
    JIMP_OBRACKET,
    JIMP_CBRACKET,
    JIMP_COMMA,
    JIMP_COLON,

    // Symbols
    JIMP_TRUE,
    JIMP_FALSE,
    JIMP_NULL,

    // Values
    JIMP_STRING,
    JIMP_NUMBER,
} Jimp_Token;

typedef struct {
    const char *file_path;
    const char *start;
    const char *end;
    const char *point;

    Jimp_Token token;
    const char *token_start;

    char *string;
    size_t string_count;
    size_t string_capacity;
    double number;

    const char *member;
    bool value_is_null;
} Jimp;

bool jimp_bool(Jimp *jimp, bool *boolean, bool defaultt);
bool jimp_number(Jimp *jimp, double *number, double defaultt);
bool jimp_string(Jimp *jimp, const char **string, const char*defaultt);
bool jimp_object_begin(Jimp *jimp);
bool jimp_object_member(Jimp *jimp);
bool jimp_object_end(Jimp *jimp);
void jimp_unknown_member(Jimp *jimp);
bool jimp_array_begin(Jimp *jimp);
bool jimp_array_item(Jimp *jimp);
bool jimp_array_end(Jimp *jimp);
void jimp_diagf(Jimp *jimp, const char *fmt, ...);

#endif // JIMP_H_

#ifdef JIMP_IMPLEMENTATION

static bool jimp__expect_token(Jimp *jimp, Jimp_Token token);
static bool jimp__get_and_expect_token(Jimp *jimp, Jimp_Token token);
static const char *jimp__token_kind(Jimp_Token token);
static bool jimp__get_token(Jimp *jimp);
static void jimp__skip_whitespaces(Jimp *jimp);
static void jimp__append_to_string(Jimp *jimp, char x);

static void jimp__append_to_string(Jimp *jimp, char x)
{
    if (jimp->string_count >= jimp->string_capacity) {
        if (jimp->string_capacity == 0) jimp->string_capacity = 1024;
        else jimp->string_capacity *= 2;
        jimp->string = realloc(jimp->string, jimp->string_capacity);
    }
    jimp->string[jimp->string_count++] = x;
}

static void jimp__skip_whitespaces(Jimp *jimp)
{
    while (jimp->point < jimp->end && isspace(*jimp->point)) {
        jimp->point += 1;
    }
}

static Jimp_Token jimp__puncts[256] = {
    ['{'] = JIMP_OCURLY,
    ['}'] = JIMP_CCURLY,
    ['['] = JIMP_OBRACKET,
    [']'] = JIMP_CBRACKET,
    [','] = JIMP_COMMA,
    [':'] = JIMP_COLON,
};

static struct {
    Jimp_Token token;
    const char *symbol;
} jimp__symbols[] = {
    { .token = JIMP_TRUE,  .symbol = "true"  },
    { .token = JIMP_FALSE, .symbol = "false" },
    { .token = JIMP_NULL,  .symbol = "null"  },
};
#define jimp__symbols_count (sizeof(jimp__symbols)/sizeof(jimp__symbols[0]))

static bool jimp__get_token(Jimp *jimp)
{
    jimp__skip_whitespaces(jimp);

    jimp->token_start = jimp->point;

    if (jimp->point >= jimp->end) {
        jimp->token = JIMP_EOF;
        return false;
    }

    jimp->token = jimp__puncts[(unsigned char)*jimp->point];
    if (jimp->token) {
        jimp->point += 1;
        return true;
    }

    for (size_t i = 0; i < jimp__symbols_count; ++i) {
        const char *symbol = jimp__symbols[i].symbol;
        if (*symbol == *jimp->point) {
            while (*symbol && jimp->point < jimp->end && *symbol++ == *jimp->point++) {}
            if (*symbol) {
                jimp->token = JIMP_INVALID;
                jimp_diagf(jimp, "ERROR: invalid symbol\n");
                return false;
            } else {
                jimp->token = jimp__symbols[i].token;
                return true;
            }
        }
    }

    char *endptr = NULL;
    jimp->number = strtod(jimp->point, &endptr); // TODO: this implies that jimp->end is a valid address and *jimp->end == 0
    if (jimp->point != endptr) {
        jimp->point = endptr;
        jimp->token = JIMP_NUMBER;
        return true;
    }

    if (*jimp->point == '"') {
        jimp->point++;
        jimp->string_count = 0;
        while (jimp->point < jimp->end) {
            // TODO: support all the JSON escape sequences defined in the spec
            // Yes, including those dumb suroggate pairs. Spec is spec.
            if (*jimp->point == '"') {
                jimp->point++;
                jimp__append_to_string(jimp, '\0');
                jimp->token = JIMP_STRING;
                return true;
            } else {
                char x = *jimp->point++;
                jimp__append_to_string(jimp, x);
            }
        }
        jimp->token = JIMP_INVALID;
        jimp_diagf(jimp, "ERROR: unfinished string\n");
        return false;
    }

    jimp->token = JIMP_INVALID;
    jimp_diagf(jimp, "ERROR: invalid token\n");
    return false;
}

void jimp_diagf(Jimp *jimp, const char *fmt, ...)
{
    long line_number = 0;
    const char *line_start = jimp->start;
    const char *point = jimp->start;
    while (point < jimp->token_start) {
        char x = *point++;
        if (x == '\n') {
            line_start = point;
            line_number += 1;
        }
    }

    fprintf(stderr, "%s:%ld:%ld: ", jimp->file_path, line_number + 1, point - line_start + 1);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

static const char *jimp__token_kind(Jimp_Token token)
{
    switch (token) {
    case JIMP_EOF:      return "end of input";
    case JIMP_INVALID:  return "invalid";
    case JIMP_OCURLY:   return "{";
    case JIMP_CCURLY:   return "}";
    case JIMP_OBRACKET: return "[";
    case JIMP_CBRACKET: return "]";
    case JIMP_COMMA:    return ",";
    case JIMP_COLON:    return ":";
    case JIMP_TRUE:     return "true";
    case JIMP_FALSE:    return "false";
    case JIMP_NULL:     return "null";
    case JIMP_STRING:   return "string";
    case JIMP_NUMBER:   return "number";
    }
    assert(0 && "unreachable");
    return NULL;
}

bool jimp_array_begin(Jimp *jimp)
{
    if (!jimp__get_token(jimp)) return false;
    if(jimp->token == JIMP_NULL) {
        jimp->value_is_null = true;
        return true;
    } else if(jimp->token != JIMP_OBRACKET) {
        jimp_diagf(jimp, "ERROR: expected `%s` or `%s`, but got `%s`\n", jimp__token_kind(JIMP_OBRACKET), jimp__token_kind(JIMP_NULL), jimp__token_kind(jimp->token));
        return false;
    }
    return true;
}

bool jimp_array_end(Jimp *jimp)
{
    if(jimp->value_is_null) {
        jimp->value_is_null = false;
        return true;
    }
    return jimp__get_and_expect_token(jimp, JIMP_CBRACKET);
}

bool jimp_array_item(Jimp *jimp)
{
    if(jimp->value_is_null) return false;
    const char *point = jimp->point;
    if (!jimp__get_token(jimp)) return false;
    if (jimp->token == JIMP_COMMA) return true;
    if (jimp->token == JIMP_CBRACKET) {
        jimp->point = point;
        return false;
    }
    jimp->point = point;
    return true;
}

void jimp_unknown_member(Jimp *jimp)
{
    jimp_diagf(jimp, "ERROR: unexpected object member `%s`\n", jimp->member);
}

bool jimp_object_begin(Jimp *jimp)
{
    if (!jimp__get_token(jimp)) return false;
    if(jimp->token == JIMP_NULL) {
        jimp->value_is_null = true;
    } else if(jimp->token != JIMP_OCURLY) {
        jimp_diagf(jimp, "ERROR: expected `%s` or `%s`, but got `%s`\n", jimp__token_kind(JIMP_OCURLY), jimp__token_kind(JIMP_NULL), jimp__token_kind(jimp->token));
        return false;
    }
    return true;
}

bool jimp_object_member(Jimp *jimp)
{
    if(jimp->value_is_null) return false;
    const char *point = jimp->point;
    if (!jimp__get_token(jimp)) return false;
    if (jimp->token == JIMP_COMMA) {
        if (!jimp__get_and_expect_token(jimp, JIMP_STRING)) return false;
        jimp->member = strdup(jimp->string); // TODO: memory leak
        if (!jimp__get_and_expect_token(jimp, JIMP_COLON)) return false;
        return true;
    }
    if (jimp->token == JIMP_CCURLY) {
        jimp->point = point;
        return false;
    }
    if (!jimp__expect_token(jimp, JIMP_STRING)) return false;
    jimp->member = strdup(jimp->string); // TODO: memory leak
    if (!jimp__get_and_expect_token(jimp, JIMP_COLON)) return false;
    return true;
}

bool jimp_object_end(Jimp *jimp)
{
    if(jimp->value_is_null) {
        jimp->value_is_null = false;
        return true;
    }
    return jimp__get_and_expect_token(jimp, JIMP_CCURLY);
}

bool jimp_string(Jimp *jimp, const char **string, const char*defaultt)
{
    if (!jimp__get_token(jimp)) return false;
    if(jimp->token == JIMP_NULL) {
        *string = defaultt;
    } else if(jimp->token == JIMP_STRING) {
        *string = strdup(jimp->string);
    } else {
        jimp_diagf(jimp, "ERROR: expected `%s` or `%s`, but got `%s`\n", jimp__token_kind(JIMP_STRING), jimp__token_kind(JIMP_NULL), jimp__token_kind(jimp->token));
        return false;
    }
    return true;
}

bool jimp_bool(Jimp *jimp, bool *boolean, bool defaultt)
{
    if (!jimp__get_token(jimp)) return false;
    if(jimp->token == JIMP_NULL) {
        *boolean = defaultt;
    } else if (jimp->token == JIMP_TRUE) {
        *boolean = true;
    } else if (jimp->token == JIMP_FALSE) {
        *boolean = false;
    } else {
        jimp_diagf(jimp, "ERROR: expected boolean or `%s`, but got `%s`\n", jimp__token_kind(JIMP_NULL), jimp__token_kind(jimp->token));
        return false;
    }
    return true;
}

bool jimp_number(Jimp *jimp, double *number, double defaultt)
{
    if (!jimp__get_token(jimp)) return false;
    if(jimp->token == JIMP_NULL) {
        *number = defaultt;
    } else if(jimp->token == JIMP_NUMBER) {
        *number = jimp->number;
    } else {
        jimp_diagf(jimp, "ERROR: expected `%s` or `%s`, but got `%s`\n", jimp__token_kind(JIMP_NUMBER), jimp__token_kind(JIMP_NULL), jimp__token_kind(jimp->token));
        return false;
    }
    return true;
}

static bool jimp__get_and_expect_token(Jimp *jimp, Jimp_Token token)
{
    if (!jimp__get_token(jimp)) return false;
    return jimp__expect_token(jimp, token);
}

static bool jimp__expect_token(Jimp *jimp, Jimp_Token token)
{
    if (jimp->token != token) {
        jimp_diagf(jimp, "ERROR: expected %s, but got %s\n", jimp__token_kind(token), jimp__token_kind(jimp->token));
        return false;
    }
    return true;
}

#endif // JIMP_IMPLEMENTATION
