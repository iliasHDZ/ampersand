#include "test.hpp"
#include "logger.hpp"

static UnitTest unit_tests[UNIT_TEST_MAX_COUNT];

static u32 unit_tests_index = 0;

static bool unit_tests_full = false;

void UnitTest::add_test(UnitTest test) {
    if (unit_tests_index >= UNIT_TEST_MAX_COUNT) {
        Log::ERR() << "Max unit test count reached! Please increase UNIT_TEST_MAX_COUNT in kernel/src/test.cpp!\n\n";
        unit_tests_full = true;
        return;
    }

    unit_tests[unit_tests_index++] = test;
}

bool UnitTest::run_all_tests() {
    if (unit_tests_full) {
        Log::ERR() << "Cannot run unit tests! Not all unit test have been included since max count has been reached!\n";
        return false;
    }

    Log::INFO() << "Running unit tests: \n";

    u32 failed    = 0;
    u32 succeeded = 0;

    for (u32 i = 0; i < unit_tests_index; i++) {
        UnitTest* test = &unit_tests[i];

        const char* err = test->run(test->run_param);

        if (err)
            failed++;
        else
            succeeded++;

        Log::INFO() << Out::dec() << '[' << (i + 1) << '/' << unit_tests_index << "] " << test->name << ": " << (err == nullptr ? "Success" : err) << '\n';   
    }

    Log::INFO() << Out::dec() << "Ran " << unit_tests_index << " tests\n";
    Log::INFO() << "Tests succeeded: " << succeeded << '\n';
    Log::INFO() << "Tests failed:    " << failed << '\n';

    unit_tests_index = 0;

    return true;
}