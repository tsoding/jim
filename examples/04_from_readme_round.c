#include <stdio.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../thirdparty/nob.h"
#define JIM_IMPLEMENTATION
#include "../jim.h"
#define JIMP_IMPLEMENTATION
#include "../jimp.h"

const char* JSON_STR =
"{"
// TODO: Uncomment when null is implemented
//"  \"null\": null,"
  "\"bool\":["
    "false,"
    "true"
  "],"
  "\"integers\":["
    "-3,"
    "-2,"
    "-1,"
    "0,"
    "1,"
    "2,"
    "3"
  "],"
  "\"floats\":["
    "3.1415,"
    "2.71828,"
    "1.618"
  "],"
  "\"string\":["
    "\"Hello\\tWorld\\n\","
    "\"\\u0000\\u0000\\u0000\\u0000\""
  "]"
"}";

typedef struct {
    bool *items;
    size_t count;
    size_t capacity;
} Bools;

typedef struct {
    long long *items;
    size_t count;
    size_t capacity;
} Integers;

typedef struct {
    double *items;
    size_t count;
    size_t capacity;
} Floats;

typedef struct {
    const char* *items;
    size_t count;
    size_t capacity;
} Strings;

typedef struct {
    // TODO: Re-add null when null is implemented
    Bools bools;
    Integers integers;
    Floats floats;
    Strings strings;
} JSON;

size_t sbwrite(const char *ptr, size_t size, size_t nmemb, String_Builder *sb) {
    assert(size == 1);

    sb_append_buf(sb, ptr, nmemb);
    return nmemb;
}

int main()
{
    JSON json = {0};

    Jimp jimp = {0};
    jimp_begin(&jimp, "JSON_STR", sb.items, sb.count);

    if (!jimp_object_begin(&jimp)) return 1;
    while (jimp_object_member(&jimp)) {
        if (strcmp(jimp.member, "null") == 0) {
            return 1; // TODO: Handle when null is implemented
        } else if (strcmp(jimp.member, "bool") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                bool x = false;
                if (!jimp_bool(&jimp, &x)) return 1;
                da_append(&json.bools, x);
            }
            if (!jimp_array_end(&jimp)) return 1;
        } else if (strcmp(jimp.member, "integers") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                long long x = 0;
                if (!jimp_int(&jimp, &x)) return 1;
                da_append(&json.integers, x);
            }
            if (!jimp_array_end(&jimp)) return 1;
        } else if (strcmp(jimp.member, "floats") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                double x = 0;
                if (!jimp_float(&jimp, &x)) return 1;
                da_append(&json.floats, x);
            }
            if (!jimp_array_end(&jimp)) return 1;
        } else if (strcmp(jimp.member, "string") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                const char* x = NULL;
                if (!jimp_string(&jimp, &x)) return 1;
                da_append(&json.strings, x);
            }
            if (!jimp_array_end(&jimp)) return 1;
        } else {
            jimp_unknown_member(&jimp);
            return 1;
        }
    }
    if (!jimp_object_end(&jimp)) return 1;


    Jim jim = {0};
    jim_begin(&jim);

    jim_object_begin(&jim);
        // TODO: Redo when null is implemented
        //jim_member_key(&jim, "null");
        //jim_null(&jim);

        jim_member_key(&jim, "bool");
        jim_array_begin(&jim);
            da_foreach(bool, b, &json.bools) {
                jim_bool(&jim, *b);
            }
        jim_array_end(&jim);

        jim_member_key(&jim, "integers");
        jim_array_begin(&jim);
            da_foreach(long long, i, &json.integers) {
                jim_integer(&jim, *i);
            }
        jim_array_end(&jim);

        jim_member_key(&jim, "floats");
        jim_array_begin(&jim);
            da_foreach(double, f, &json.floats) {
                jim_float(&jim, *f, 5);
            }
        jim_array_end(&jim);

        jim_member_key(&jim, "string");
        jim_array_begin(&jim);
            da_foreach(const char*, s, &json.strings) {
                jim_string(&jim, *s);
            }
        jim_array_end(&jim);
    jim_object_end(&jim);

    if (jim.error != JIM_OK) {
        fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
                jim_error_string(jim.error));
        return -1;
    }

    printf("Initial:    %s\n", JSON_STR);
    printf("Round Trip: %.*s\n", (int)jim.sink_count, jim.sink);

    // TODO: Compare the two and print a roundtrip error if they differ

    return 0;
}
