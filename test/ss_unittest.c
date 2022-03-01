#include "ss_unittest.h"

#include <memory.h>
#include <stdlib.h>
#include <string.h>

ss_bool_t ss_unittest_tryrun(int argc, char *argv[])
{
    int         i;
    const char  *unit;

    unit = SS_NULL;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0) {
            unit = argv[++i];
            break;
        }
    }

    if (!unit) {
        return SS_FALSE;
    }



    return SS_TRUE;
}

