#pragma once

#include <common.h>

#define UNIT_TEST_MAX_COUNT 64

/*
    The return value is nullptr on success.
    Otherwise it is the error message.
*/
typedef const char* (*RunUnitTest)(void*);

struct UnitTest {
public:
    const char* name;
    RunUnitTest run;
    void* run_param;

public:
    static void add_test(UnitTest test);

    static bool run_all_tests();

};