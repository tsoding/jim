#include <stdio.h>
#include <stdbool.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../thirdparty/nob.h"
#define JIMP_IMPLEMENTATION
#include "../jimp.h"

typedef struct {
    const char *name;
    long long age;
    const char *location;
    double cash;
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
        if (strcmp(jimp->string, "name") == 0) {
            if (!jimp_string(jimp)) return false;
            p->name = strdup(jimp->string);
        } else if (strcmp(jimp->string, "age") == 0) {
            if (!jimp_number(jimp)) return false;
            p->age = jimp->integer;
        } else if (strcmp(jimp->string, "location") == 0) {
            if (!jimp_string(jimp)) return false;
            p->location = strdup(jimp->string);
        } else if (strcmp(jimp->string, "cash") == 0) {
            if (!jimp_number(jimp)) return false;
            p->cash = jimp->decimal;
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
    printf("name     = %s\n",     p->name);
    printf("age      = %lld\n",   p->age);
    printf("location = %s\n",     p->location);
    printf("cash     = $%.2lf\n", p->cash);
}

typedef struct {
    JimpNumberType type;
    long long integer;
    double decimal;
} Number;

typedef struct {
    Number *items;
    size_t count;
    size_t capacity;
} Numbers;

int main(void)
{
    const char *file_path = "database.json";

    String_Builder sb = {0};
    if (!read_entire_file(file_path, &sb)) return 1;

    Jimp jimp = {0};
    jimp_begin(&jimp, file_path, sb.items, sb.count);

    People ps = {0};
    Numbers xs = {0};
    if (!jimp_object_begin(&jimp)) return 1;
    while (jimp_object_member(&jimp)) {
        if (strcmp(jimp.string, "profile") == 0) {
            if (!parse_people(&jimp, &ps)) return 1;
        } else if (strcmp(jimp.string, "number") == 0) {
            if (!jimp_array_begin(&jimp)) return 1;
            while (jimp_array_item(&jimp)) {
                if (!jimp_number(&jimp)) return 1;
                da_append(&xs, {.type = jimp.number_type, .integer = jimp.integer, .decimal = jimp.decimal});
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
    da_foreach(JimpNumber, x, &xs) {
        if (x->type == JIMP_INTEGER)
            printf("%lld ", x->integer);
        else
            printf("%lf ",  x->decimal);
    }
    printf("\n");

    return 0;
}
