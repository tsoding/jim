# JIM

Immediate Mode JSON Serialization Library in C. Similar to [imgui](https://github.com/ocornut/imgui) but for generating JSON.

## Example

[`example.c`](./example.c):

```c
#include <stdio.h>

#define JIMPLEMENTATION
#include "./jim.h"

int main()
{
    Jim jim = {
        .sink = stdout,
        .write = (Jim_Write) fwrite,
    };

    jim_object_begin(&jim);
        jim_member_key(&jim, "null");
        jim_null(&jim);

        jim_member_key(&jim, "bool");
        jim_array_begin(&jim);
            jim_bool(&jim, 0);
            jim_bool(&jim, 1);
        jim_array_end(&jim);

        jim_member_key(&jim, "integers");
        jim_array_begin(&jim);
            for (int i = -3; i <= 3; ++i) {
                jim_integer(&jim, i);
            }
        jim_array_end(&jim);

        jim_member_key(&jim, "floats");
        jim_array_begin(&jim);
            jim_float(&jim, 0.0, 4);
            jim_float(&jim, -0.0, 4);
            jim_float(&jim, 3.1415, 4);
            jim_float(&jim, 2.71828, 5);
            jim_float(&jim, 1.6180, 4);
            jim_float(&jim, 0.0 / 0.0, 4);
            jim_float(&jim, 1.0 / 0.0, 4);
            jim_float(&jim, -1.0 / 0.0, 4);
        jim_array_end(&jim);

        jim_member_key(&jim, "string");
        jim_array_begin(&jim);
            jim_string(&jim, "Hello\tWorld\n");
            jim_string_sized(&jim, "\0\0\0\0", 4);
        jim_array_end(&jim);
    jim_object_end(&jim);

    if (jim.error != JIM_OK) {
        fprintf(stderr, "ERROR: could not serialize json properly: %s\n",
                jim_error_string(jim.error));
        return -1;
    }

    return 0;
}
```

### Output

```console
$ cc -o example example.c
$ ./example | jq .
{
  "null": null,
  "bool": [
    false,
    true
  ],
  "integers": [
    -3,
    -2,
    -1,
    0,
    1,
    2,
    3
  ],
  "floats": [
    3.1415,
    2.71828,
    1.618
  ],
  "string": [
    "Hello\tWorld\n",
    "\u0000\u0000\u0000\u0000"
  ]
}
```

## Testing

```console
$ make
$ ./test
```

The expected outputs of the test cases are stored in [./test_expected.h](./test_expected.h). To regenerate it just run:

```console
$ ./test record
```

## Notes

1. Does not depends on libc. Could be theoretically used in embedded, but I know nothing about embedded, so maybe not.
2. `jim_float()` is quite likely very stupid and imprecise
