#include <stdio.h>
#include <stdbool.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#define JIMP_IMPLEMENTATION
#include "jimp.h"

typedef struct {
    const char *name;
    long age;
    const char *location;
    long body_count;
} Person;

typedef struct {
    Person *items;
    size_t count;
    size_t capacity;
} People;

bool parse_person(Jimp *jimp, Person *p)
{
    if (!jimp_object_begin(jimp)) return false;
    while (jimp_object_member(jimp)) {
        if (strcmp(jimp->member, "name") == 0) {
            if (!jimp_string(jimp, &p->name))       return false;
        } else if (strcmp(jimp->member, "age") == 0) {
            if (!jimp_number(jimp, &p->age))        return false;
        } else if (strcmp(jimp->member, "location") == 0) {
            if (!jimp_string(jimp, &p->location))   return false;
        } else if (strcmp(jimp->member, "body_count") == 0) {
            if (!jimp_number(jimp, &p->body_count)) return false;
        } else {
            jimp_unknown_member(jimp);
            return false;
        }
    }
    return jimp_object_end(jimp);
}

bool parse_people(Jimp *jimp, People *ps)
{
    if (!jimp_array_begin(jimp)) return false;
    while (jimp_array_item(jimp)) {
        Person p = {0};
        if (!parse_person(jimp, &p)) return false;
        da_append(ps, p);
    }
    if (!jimp_array_end(jimp)) return false;

    return true;
}

void print_person(const Person *p)
{
    printf("name       = %s\n",  p->name);
    printf("age        = %ld\n", p->age);
    printf("location   = %s\n",  p->location);
    printf("body_count = %ld\n", p->body_count);
}

typedef struct {
    long *items;
    size_t count;
    size_t capacity;
} Numbers;

int main()
{
    // const char *file_path = "profile.json";
    // const char *file_path = "numbers.json";
    // const char *file_path = "profiles.json";
    // const char *file_path = "empty.json";
    // const char *file_path = "one.json";
    const char *file_path = "database.json";
    String_Builder sb = {0};
    if (!read_entire_file(file_path, &sb)) return 1;
    Jimp jimp = {
        .file_path = file_path,
    };
    static char string_store[1024];
    stb_c_lexer_init(&jimp.l, sb.items, sb.items + sb.count, string_store, sizeof(string_store));

    People ps = {0};
    Numbers xs = {0};
    if (!jimp_object_begin(&jimp)) return 1;
    while (jimp_object_member(&jimp)) {
        if (strcmp(jimp.member, "profile") == 0) {
            if (!parse_people(&jimp, &ps)) return 1;
        } else if (strcmp(jimp.member, "number") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                long x = 0;
                jimp_number(&jimp, &x);
                da_append(&xs, x);
            }
            if (!jimp_array_end(&jimp)) return 1;
        } else {
            jimp_unknown_member(&jimp);
            return 1;
        }
    }
    if (!jimp_object_end(&jimp)) return 1;

    da_foreach(Person, p, &ps) {
        print_person(p);
        printf("\n");
    }
    printf("------------------------------\n");
    da_foreach(long, x, &xs) {
        printf("%ld ", *x);
    }
    printf("\n");

    return 0;
}
