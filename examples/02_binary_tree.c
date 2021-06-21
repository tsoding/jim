#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define JIM_IMPLEMENTATION
#include "../jim.h"

#include "fruits.h"

typedef struct Node Node;

struct Node {
    const char *value;
    Node *left;
    Node *right;
};

Node *generate_tree_of_fruits(size_t level_cur, size_t level_max)
{
    if (level_cur < level_max) {
        // Let It Leak! Let It Leak!
        // Let It Leak! Oh, Let It Leak!
        // Memory costs nothing!
        // Let It Leak!
        Node *node = malloc(sizeof(*node));
        node->value = fruits[rand() % fruits_count];
        node->left = generate_tree_of_fruits(level_cur + 1, level_max);
        node->right = generate_tree_of_fruits(level_cur + 1, level_max);
        return node;
    } else {
        return NULL;
    }
}

void print_node_as_json(Jim *jim, Node *node)
{
    if (node == NULL) {
        jim_null(jim);
    } else {
        jim_object_begin(jim);
        jim_member_key(jim, "value");
        jim_string(jim, node->value);

        jim_member_key(jim, "left");
        print_node_as_json(jim, node->left);

        jim_member_key(jim, "right");
        print_node_as_json(jim, node->right);
        jim_object_end(jim);
    }
}

int main()
{
    srand(time(0));
    Jim jim = {
        .sink = stdout,
        .write = (Jim_Write) fwrite,
    };
    print_node_as_json(&jim, generate_tree_of_fruits(0, 4));
    return 0;
}
