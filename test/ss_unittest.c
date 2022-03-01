#include "ss_unittest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void ss_unittest_function();

int main(int argc, char *argv[])
{
    ss_unittest_function();
    printf("unittest all done.\n");
    return 0;
}

void _assert_true(const char *filename, int lineno, ss_bool_t actual, const char *expression)
{
    if (!actual) {
        printf("assert error at %s:%d\n", filename, lineno);
        printf("[%s] expected true, actually false.\n", expression);
        exit(1);
    }
}

void _assert_false(const char *filename, int lineno, ss_bool_t actual, const char *expression)
{
    if (actual) {
        printf("assert error at %s:%d\n", filename, lineno);
        printf("[%s] expected false, actually true.\n", expression);
        exit(1);
    }
}

void _assert_equal_string(const char *filename, int lineno, const char *expected, const char *actual, const char *expression)
{
    if (strcmp(expected, actual) != 0) {
        printf("assert error at %s:%d\n", filename, lineno);
        printf("[%s] expected \"%s\", actually \"%s\".\n",
            expression, expected, actual);
        exit(1);
    }
}

void _assert_equal_int32(const char *filename, int lineno, ss_int32_t expected, ss_int32_t actual, const char *expression)
{
    if (expected != actual) {
        printf("assert error at %s:%d\n", filename, lineno);
        printf("[%s] expected %d, actually %d.\n", expression, expected, actual);
        exit(1);
    }
}

