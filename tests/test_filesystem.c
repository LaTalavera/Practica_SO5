#include "unity.h"
#include "headers.h"
#include <string.h>

void setUp(void)
{
    // This function is run before each test; can be used it to set up test data
}

void tearDown(void)
{
    // This function is run after each test; can be used it for cleanup (freeing memory, etc.)
}

void test_CheckCommand_ValidInput(void)
{
    char commandStr[] = "dir";
    char command[100], arg1[100], arg2[100];
    int result = CheckCommand(commandStr, command, arg1, arg2);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("dir", command);
    TEST_ASSERT_EQUAL_STRING("", arg1);
    TEST_ASSERT_EQUAL_STRING("", arg2);
}

void test_CheckCommand_WithArguments(void)
{
    char commandStr[] = "copy file1 file2";
    char command[100], arg1[100], arg2[100];
    int result = CheckCommand(commandStr, command, arg1, arg2);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("copy", command);
    TEST_ASSERT_EQUAL_STRING("file1", arg1);
    TEST_ASSERT_EQUAL_STRING("file2", arg2);
}

void test_FindFile_FileExists(void)
{
    EXT_DIRECTORY_ENTRY directory[MAX_FILES];
    EXT_INODE_BLOCK inodes;
    // Initialize directory with one file named "testfile"
    for (int i = 0; i < MAX_FILES; i++)
    {
        directory[i].inode = NULL_INODE;
        directory[i].file_name[0] = '\0';
    }
    directory[0].inode = 5;
    strncpy(directory[0].file_name, "testfile", FILE_NAME_LENGTH - 1);

    int result = FindFile(directory, &inodes, "testfile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "File should be found at index 0");
}

void test_FindFile_FileNotExists(void)
{
    EXT_DIRECTORY_ENTRY directory[MAX_FILES];
    EXT_INODE_BLOCK inodes;
    // No files in directory
    for (int i = 0; i < MAX_FILES; i++)
    {
        directory[i].inode = NULL_INODE;
        directory[i].file_name[0] = '\0';
    }

    int result = FindFile(directory, &inodes, "nonexistent");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, result, "Should return -1 if the file does not exist");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_CheckCommand_ValidInput);
    RUN_TEST(test_CheckCommand_WithArguments);
    RUN_TEST(test_FindFile_FileExists);
    RUN_TEST(test_FindFile_FileNotExists);
    return UNITY_END();
}
