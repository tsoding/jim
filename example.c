#include <stdio.h>

#define JIM_IMPLEMENTATION
#include "./jim.h"

int main()
{
    Jim jim = {
        .sink = stdout,
        .write = fwrite,
    };

    jim_begin(&jim);
        jim_array_begin(&jim);
            jim_element_begin(&jim);
                jim_array_begin(&jim);
                    jim_element_begin(&jim);
                    jim_null(&jim);
                    jim_element_end(&jim);
                    for (int i = 0; i < 5; ++i) {
                        jim_element_begin(&jim);
                        jim_bool(&jim, i);
                        jim_element_end(&jim);
                    }
                jim_array_end(&jim);
            jim_element_end(&jim);

            jim_element_begin(&jim);
                jim_bool(&jim, 0);
            jim_element_end(&jim);

            jim_element_begin(&jim);
                jim_bool(&jim, 1);
            jim_element_end(&jim);

            jim_element_begin(&jim);
                size_t size = 5;
                jim_string(&jim, "\0\0\0\0\0", &size);
            jim_element_end(&jim);
            
            jim_element_begin(&jim);
                jim_object_begin(&jim);
                    jim_member_begin(&jim);
                        jim_member_key(&jim, "hello", NULL);
                        jim_bool(&jim, 0);
                    jim_member_end(&jim);
                jim_object_end(&jim);
            jim_element_end(&jim);
        jim_array_end(&jim);
    jim_end(&jim);

    if (jim.error != JIM_OK) {
        fprintf(stderr, "ERROR: could not serialize json properly: %s\n", jim_error_string(jim.error));
        return -1;
    }

    return 0;
}
