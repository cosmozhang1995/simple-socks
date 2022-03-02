#ifndef _SS_UNITTEST_H_
#define _SS_UNITTEST_H_

#include "common/ss_types.h"


#define SS_UNITTEST                                             \
void ss_unittest_function()


void _assert_true(const char *filename, int lineno, ss_bool_t actual, const char *expression);
void _assert_false(const char *filename, int lineno, ss_bool_t actual, const char *expression);
void _assert_equal_string(const char *filename, int lineno, const char *expected, const char *actual, const char *expression);
void _assert_equal_int32(const char *filename, int lineno, ss_int32_t expected, ss_int32_t actual, const char *expression);

#define ASSERT_TRUE(statement) _assert_true(__FILE__, __LINE__, (statement), "" # statement)
#define ASSERT_FALSE(statement) _assert_false(__FILE__, __LINE__, (statement), "" # statement)
#define ASSERT_EQUAL_STRING(expected, statement) _assert_equal_string(__FILE__, __LINE__, (expected), (statement), "" # statement)
#define ASSERT_EQUAL_INT32(expected, statement) _assert_equal_int32(__FILE__, __LINE__, (expected), (statement), "" # statement)


#endif // _SS_UNITTEST_H_