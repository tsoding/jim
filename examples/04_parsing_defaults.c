#include <stdio.h>
#include <stdbool.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../thirdparty/nob.h"
#define JIMP_IMPLEMENTATION
#include "../jimp.h"

typedef struct {
    const char *name;
    double age;
    const char *location;
    double body_count;
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
            if (!jimp_string(jimp, &p->name, strdup("Default Name"))) return false;
        } else if (strcmp(jimp->member, "age") == 0) {
            if (!jimp_number(jimp, &p->age, 18))          return false;
        } else if (strcmp(jimp->member, "location") == 0) {
            if (!jimp_string(jimp, &p->location, strdup("UNKNOWN")))   return false;
        } else if (strcmp(jimp->member, "body_count") == 0) {
            if (!jimp_number(jimp, &p->body_count, 69))   return false;
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
        Person p = {0}; // jimp_object_* uses this value as its default
        if (!parse_person(jimp, &p)) return false;
        da_append(ps, p);
    }
    if (!jimp_array_end(jimp)) return false;

    return true;
}

void print_person(const Person *p)
{
    printf("name       = %s\n",  p->name);
    printf("age        = %lf\n", p->age);
    printf("location   = %s\n",  p->location);
    printf("body_count = %lf\n", p->body_count);
}

typedef struct {
    long *items;
    size_t count;
    size_t capacity;
} Numbers;

int main()
{
    const char *file_path = "database_default.json";
    String_Builder sb = {0};
    if (!read_entire_file(file_path, &sb)) return 1;
    Jimp jimp = {
        .file_path = file_path,
        .start = sb.items,
        .end = sb.items + sb.count,
        .point = sb.items,
    };

    People ps = {0};
    Numbers xs = {0};
    Numbers xs_null = {0};
    if (!jimp_object_begin(&jimp)) return 1;
    while (jimp_object_member(&jimp)) {
        if (strcmp(jimp.member, "profile") == 0) {
            if (!parse_people(&jimp, &ps)) return 1;
        } else if (strcmp(jimp.member, "number") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                double x;
                if (!jimp_number(&jimp, &x, 0)) return 1;
                da_append(&xs, x);
            }
            if (!jimp_array_end(&jimp)) return 1;
        } else if (strcmp(jimp.member, "array_null") == 0) {
            // The default for array is an empty array 
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                double x;
                if (!jimp_number(&jimp, &x, 0)) return 1;
                da_append(&xs_null, x);
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
    printf("------------------------------\n");
    if(xs_null.count == 0) {
        printf("[]\n");
    } else {
        da_foreach(long, x, &xs_null) {
            printf("%ld ", *x);
        }
    }
    printf("\n");

    return 0;
}
