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

void test_SaveSuperBlock(void)
{
    // Initialize a test SuperBlock with known values
    EXT_SIMPLE_SUPERBLOCK testSuperBlock = {
        .total_inodes = 24,
        .total_blocks = 100,
        .free_blocks = 90,
        .free_inodes = 23,
        .first_data_block = 4,
        .block_size = BLOCK_SIZE};

    // Create a temporary file
    FILE *tempFile = fopen("temp_partition.bin", "wb+");
    TEST_ASSERT_NOT_NULL_MESSAGE(tempFile, "Failed to create temporary partition file.");

    // Call the function to test
    SaveSuperBlock(&testSuperBlock, tempFile);

    // Ensure data is written
    fflush(tempFile);

    // Reset file pointer to the beginning
    fseek(tempFile, 0, SEEK_SET);

    // Read back the SuperBlock
    EXT_SIMPLE_SUPERBLOCK readSuperBlock;
    size_t readCount = fread(&readSuperBlock, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, tempFile);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, readCount, "Failed to read SuperBlock from temporary file.");

    // Compare the written and read SuperBlocks
    TEST_ASSERT_EQUAL_UINT_MESSAGE(testSuperBlock.total_inodes, readSuperBlock.total_inodes, "total_inodes mismatch.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(testSuperBlock.total_blocks, readSuperBlock.total_blocks, "total_blocks mismatch.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(testSuperBlock.free_blocks, readSuperBlock.free_blocks, "free_blocks mismatch.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(testSuperBlock.free_inodes, readSuperBlock.free_inodes, "free_inodes mismatch.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(testSuperBlock.first_data_block, readSuperBlock.first_data_block, "first_data_block mismatch.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(testSuperBlock.block_size, readSuperBlock.block_size, "block_size mismatch.");

    // Clean up: Close and remove the temporary file
    fclose(tempFile);
    remove("temp_partition.bin");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_CheckCommand_ValidInput);
    RUN_TEST(test_CheckCommand_WithArguments);
    RUN_TEST(test_FindFile_FileExists);
    RUN_TEST(test_FindFile_FileNotExists);
    RUN_TEST(test_SaveSuperBlock); // New test added here
    return UNITY_END();
}
