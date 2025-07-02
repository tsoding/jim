#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JIM_IMPLEMENTATION
#include "./jim.h"
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "./nob.h"

#include "./test_jim_expected.h"

void null_case(Jim *jim)
{
    jim_begin(jim);
    jim_array_begin(jim);
    jim_null(jim);
    jim_array_end(jim);
}

void bool_case(Jim *jim)
{
    jim_begin(jim);
    jim_array_begin(jim);
    jim_bool(jim, 0);
    jim_bool(jim, 1);
    jim_array_end(jim);
}

void integer_case(Jim *jim)
{
    jim_begin(jim);
    jim_array_begin(jim);
    for (int i = -10; i <= 10; ++i) {
        jim_integer(jim, i);
    }
    jim_array_end(jim);
}

void float_case(Jim *jim)
{
    jim_begin(jim);
    jim_array_begin(jim);
    jim_float(jim, 0.0, 4);
    jim_float(jim, -0.0, 4);
    jim_float(jim, 3.1415, 4);
    jim_float(jim, 2.71828, 5);
    jim_float(jim, 1.6180, 1);
    jim_float(jim, 0.0 / 0.0, 4);
    jim_float(jim, 1.0 / 0.0, 4);
    jim_float(jim, -1.0 / 0.0, 4);
    jim_array_end(jim);
}

void string_case(Jim *jim)
{
    jim_begin(jim);
    jim_array_begin(jim);
    jim_string(jim, "hello");
    jim_string(jim, "world");
    jim_string(jim, "\n\b\t");
    jim_string_sized(jim, "\0\0\0\0", 4);
    jim_array_end(jim);
}

void array_case(Jim *jim)
{
    jim_begin(jim);
    jim_array_begin(jim);

    for (int n = 1; n <= 5; ++n) {
        for (int i = 0; i < n; ++i) jim_array_begin(jim);
        for (int i = 0; i < n; ++i) jim_array_end(jim);
    }

    jim_array_end(jim);
}

void object_case_rec(Jim *jim, int level, int *counter)
{
    if (level < 3) {
        jim_object_begin(jim);
        jim_member_key(jim, "l");
        object_case_rec(jim, level + 1, counter);
        jim_member_key(jim, "r");
        object_case_rec(jim, level + 1, counter);
        jim_object_end(jim);
    } else {
        jim_integer(jim, (*counter)++);
    }
}

void object_case(Jim *jim)
{
    jim_begin(jim);
    int counter = 0;
    object_case_rec(jim, 0, &counter);
}

void pp_empty_array_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_array_begin(jim);
    jim_array_end(jim);
}

void pp_singleton_array_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_array_begin(jim);
    jim_integer(jim, 69);
    jim_array_end(jim);
}

void pp_array_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_array_begin(jim);
    jim_integer(jim, 69);
    jim_integer(jim, 420);
    jim_integer(jim, 1337);
    jim_integer(jim, 80085);
    jim_array_end(jim);
}

void pp_empty_object_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_object_begin(jim);
    jim_object_end(jim);
}

void pp_singleton_object_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_object_begin(jim);
    jim_member_key(jim, "foo");
    jim_integer(jim, 69);
    jim_object_end(jim);
}

void pp_object_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_object_begin(jim);
    jim_member_key(jim, "foo");
    jim_integer(jim, 69);
    jim_member_key(jim, "bar");
    jim_integer(jim, 420);
    jim_member_key(jim, "baz");
    jim_integer(jim, 1337);
    jim_object_end(jim);
}

void pp_nested_case(Jim *jim)
{
    jim_begin(jim);
    jim->pp = 4;
    jim_object_begin(jim);
        jim_member_key(jim, "integer");
        jim_integer(jim, 69);
        jim_member_key(jim, "empty_array");
        jim_array_begin(jim);
        jim_array_end(jim);
        jim_member_key(jim, "empty_object");
        jim_object_begin(jim);
        jim_object_end(jim);
        jim_member_key(jim, "array_of_integers");
        jim_array_begin(jim);
            jim_integer(jim, 69);
            jim_integer(jim, 420);
            jim_integer(jim, 1337);
            jim_integer(jim, 80085);
        jim_array_end(jim);
        jim_member_key(jim, "object_of_integers");
        jim_object_begin(jim);
            jim_member_key(jim, "foo");
            jim_integer(jim, 69);
            jim_member_key(jim, "bar");
            jim_integer(jim, 420);
            jim_member_key(jim, "baz");
            jim_integer(jim, 1337);
            jim_member_key(jim, "karabaz");
            jim_integer(jim, 80085);
        jim_object_end(jim);
    jim_object_end(jim);
}

typedef struct {
    const char *name;
    void (*run)(Jim *jim);
} Test_Case;

#define TEST_CASE(case_name) \
    { \
        .name = #case_name, \
        .run = case_name \
    }

const Test_Case test_cases[] = {
    TEST_CASE(null_case),
    TEST_CASE(bool_case),
    TEST_CASE(integer_case),
    TEST_CASE(float_case),
    TEST_CASE(string_case),
    TEST_CASE(array_case),
    TEST_CASE(object_case),
    TEST_CASE(pp_empty_array_case),
    TEST_CASE(pp_singleton_array_case),
    TEST_CASE(pp_array_case),
    TEST_CASE(pp_empty_object_case),
    TEST_CASE(pp_singleton_object_case),
    TEST_CASE(pp_object_case),
    TEST_CASE(pp_nested_case),
};

bool record(const char *header_path)
{
    Jim jim_stream = {0};
    Jim jim_buffer = {0};

    jim_write_cstr(&jim_stream, "const char *test_cases_expected[] = {\n");
    for (size_t i = 0; i < ARRAY_LEN(test_cases); ++i) {
        jim_buffer.sink_count = 0;
        test_cases[i].run(&jim_buffer);
        jim_write_cstr(&jim_stream, "    ");
        jim_string_sized(&jim_stream, jim_buffer.sink, jim_buffer.sink_count);
        jim_write_cstr(&jim_stream, ",\n");
    }
    jim_write_cstr(&jim_stream, "};\n");

    if (!write_entire_file(header_path, jim_stream.sink, jim_stream.sink_count)) return false;
    printf("Updated %s\n", header_path);
    return true;
}

void test(void)
{
    Jim jim_buffer = {0};


    assert(ARRAY_LEN(test_cases) == ARRAY_LEN(test_cases_expected) && "Run `record` command to update expected test cases");
    for (size_t i = 0; i < ARRAY_LEN(test_cases); ++i) {
        printf("%s ... ", test_cases[i].name);

        jim_buffer.sink_count = 0;
        test_cases[i].run(&jim_buffer);

        if (jim_buffer.sink_count != strlen(test_cases_expected[i])
                || memcmp(jim_buffer.sink, test_cases_expected[i], jim_buffer.sink_count) != 0) {
            printf("FAILED!\n");
            printf("Expected: %s\n", test_cases_expected[i]);
            printf("Actual:   ");
            fwrite(jim_buffer.sink, jim_buffer.sink_count, 1, stdout);
            printf("\n");
            exit(1);
        }

        printf("OK\n");
    }
}

int main(int argc, char **argv)
{
    if (argc >= 2) {
        if (strcmp(argv[1], "record") == 0) {
            if (!record("test_jim_expected.h")) return 1;
        } else {
            fprintf(stderr, "Usage: ./test [record]\n");
            fprintf(stderr, "ERROR: unknown subcommand %s.\n", argv[1]);
        }
    } else  {
        test();
    }
}
