#include "unity.h"
#include "headers.h" 

void setUp(void) {
    // This function is run before each test; can be used it to set up test data
}

void tearDown(void) {
    // This function is run after each test; can be used it for cleanup (freeing memory, etc.)
}

void test_CheckCommand_ValidInput(void) {
    char commandStr[] = "dir";
    char command[100], arg1[100], arg2[100];
    int result = CheckCommand(commandStr, command, arg1, arg2);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("dir", command);
    TEST_ASSERT_EQUAL_STRING("", arg1);
    TEST_ASSERT_EQUAL_STRING("", arg2);
}

void test_CheckCommand_WithArguments(void) {
    char commandStr[] = "copy file1 file2";
    char command[100], arg1[100], arg2[100];
    int result = CheckCommand(commandStr, command, arg1, arg2);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("copy", command);
    TEST_ASSERT_EQUAL_STRING("file1", arg1);
    TEST_ASSERT_EQUAL_STRING("file2", arg2);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_CheckCommand_ValidInput);
    RUN_TEST(test_CheckCommand_WithArguments);
    return UNITY_END();
}

