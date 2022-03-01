#ifndef _SS_UNITTEST_H_
#define _SS_UNITTEST_H_

typedef void (*ss_unittest_function_t)();

extern ss_unittest_function_t ss_unittest_function;

#define SS_UNITTEST(func) \
    ss_unittest_function_t ss_unittest_function = func;

#endif // _SS_UNITTEST_H_