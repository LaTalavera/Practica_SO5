#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "headers.h"

#define COMMAND_LENGTH 100

// ---------------------------------------------------------------------------
// MAIN FUNCTION
// ---------------------------------------------------------------------------
#ifndef TEST
int main()
{
   // Buffers for user input
   char command[COMMAND_LENGTH];
   char order[COMMAND_LENGTH];
   char argument1[COMMAND_LENGTH];
   char argument2[COMMAND_LENGTH];

   // Structures holding filesystem metadata and data
   EXT_SIMPLE_SUPERBLOCK superBlock;
   EXT_BYTE_MAPS byteMaps;
   EXT_INODE_BLOCK inodeBlock;
   EXT_DIRECTORY_ENTRY directory[MAX_FILES];
   EXT_DATA data[MAX_DATA_BLOCKS];
   EXT_DATA fileData[MAX_PARTITION_BLOCKS];

   // 1) Open the "particion.bin" file (simulating a disk partition)
   FILE *file = fopen("particion.bin", "r+b");
   if (file == NULL)
   {
      perror("Error opening file particion.bin");
      return 1;
   }

   // 2) Read the entire partition into memory for quick operations
   fread(&fileData, BLOCK_SIZE, MAX_PARTITION_BLOCKS, file);

   // 3) Extract each structure from the loaded blocks in memory
   memcpy(&superBlock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], BLOCK_SIZE);
   memcpy(&byteMaps, (EXT_BYTE_MAPS *)&fileData[1], BLOCK_SIZE);
   memcpy(&inodeBlock, (EXT_INODE_BLOCK *)&fileData[2], BLOCK_SIZE);
   memcpy(directory, (EXT_DIRECTORY_ENTRY *)&fileData[3], sizeof(EXT_DIRECTORY_ENTRY) * MAX_FILES);
   memcpy(&data, (EXT_DATA *)&fileData[4], MAX_DATA_BLOCKS * BLOCK_SIZE);

   // 4) Main loop: read commands until user exits or EOF
   for (;;)
   {
      do
      {
         printf("\n>> ");
         if (!fgets(command, COMMAND_LENGTH, stdin))
         {
            // If stdin closes or an error occurs, exit gracefully
            break;
         }
         // Remove trailing newline, if any
         char *newLine = strchr(command, '\n');
         if (newLine)
         {
            *newLine = '\0';
         }
      } while (CheckCommand(command, order, argument1, argument2) != 0);

      // Process the user command in a dedicated function
      ProcessCommand(
          order,
          argument1,
          argument2,
          &superBlock,
          &byteMaps,
          &inodeBlock,
          directory,
          data,
          file);
   }

   fclose(file);
   return 0;
}
#endif // TEST

// ---------------------------------------------------------------------------
// COMMAND PARSING AND PROCESSING
// ---------------------------------------------------------------------------

/**
 * @brief Splits the user input into 'order', 'arg1', and 'arg2'.
 * @return 0 if successfully parses something, or -1 if it fails to parse tokens.
 */
int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2)
{
   char commandCopy[COMMAND_LENGTH];
   strncpy(commandCopy, commandStr, COMMAND_LENGTH);
   commandCopy[COMMAND_LENGTH - 1] = '\0';

   // Remove newline if present
   char *newline = strchr(commandCopy, '\n');
   if (newline)
      *newline = '\0';

   // Tokenize the main command
   char *token = strtok(commandCopy, " ");
   if (token == NULL)
      return -1;
   strcpy(command, token);

   // Tokenize arg1
   token = strtok(NULL, " ");
   if (token)
      strcpy(arg1, token);
   else
      arg1[0] = '\0';

   // Tokenize arg2
   token = strtok(NULL, " ");
   if (token)
      strcpy(arg2, token);
   else
      arg2[0] = '\0';

   return 0;
}

/**
 * @brief Dispatches the user command to the appropriate function. This approach
 *        simplifies main() and centralizes command-handling logic.
 */
void ProcessCommand(
    const char *order,
    const char *arg1,
    const char *arg2,
    EXT_SIMPLE_SUPERBLOCK *superBlock,
    EXT_BYTE_MAPS *byteMaps,
    EXT_INODE_BLOCK *inodeBlock,
    EXT_DIRECTORY_ENTRY *directory,
    EXT_DATA *data,
    FILE *file)
{
   if (strcmp(order, "dir") == 0)
   {
      ListDirectory(directory, inodeBlock);
   }
   else if (strcmp(order, "info") == 0)
   {
      PrintSuperBlock(superBlock);
   }
   else if (strcmp(order, "bytemaps") == 0)
   {
      PrintByteMaps(byteMaps);
   }
   else if (strcmp(order, "rename") == 0)
   {
      if (strlen(arg1) == 0 || strlen(arg2) == 0)
      {
         fprintf(stderr, "Usage: rename <old_name> <new_name>\n");
      }
      else
      {
         if (RenameFile(directory, inodeBlock, (char *)arg1, (char *)arg2) == 0)
         {
            SaveAllChanges(directory, inodeBlock, byteMaps, superBlock, data, file);
         }
      }
   }
   else if (strcmp(order, "print") == 0)
   {
      if (strlen(arg1) == 0)
      {
         fprintf(stderr, "Usage: print <file_name>\n");
      }
      else
      {
         PrintFile(directory, inodeBlock, data, (char *)arg1);
      }
   }
   else if (strcmp(order, "remove") == 0)
   {
      if (strlen(arg1) == 0)
      {
         fprintf(stderr, "Usage: remove <file_name>\n");
      }
      else
      {
         if (DeleteFile(directory, inodeBlock, byteMaps, superBlock, (char *)arg1) == 0)
         {
            SaveAllChanges(directory, inodeBlock, byteMaps, superBlock, data, file);
         }
      }
   }
   else if (strcmp(order, "copy") == 0)
   {
      if (strlen(arg1) == 0 || strlen(arg2) == 0)
      {
         fprintf(stderr, "Usage: copy <source_file> <destination_file>\n");
      }
      else
      {
         if (CopyFile(directory, inodeBlock, byteMaps, superBlock, data, (char *)arg1, (char *)arg2, file) == 0)
         {
            SaveAllChanges(directory, inodeBlock, byteMaps, superBlock, data, file);
         }
      }
   }
   else if (strcmp(order, "create") == 0)
   {
      // We interpret arg1 as filename, arg2 as "content" for simplicity
      // If you need multi-word content, you'd have to adjust the logic to gather
      // multiple tokens or rework the input parsing.
      if (strlen(arg1) == 0 || strlen(arg2) == 0)
      {
         fprintf(stderr, "Usage: create <file_name> <content>\n");
      }
      else
      {
         if (CreateFile(directory, inodeBlock, byteMaps, superBlock, data, (char *)arg1, (char *)arg2) == 0)
         {
            SaveAllChanges(directory, inodeBlock, byteMaps, superBlock, data, file);
         }
      }
   }
   else if (strcmp(order, "debug") == 0)
   {
      DebugListAllDirectoryEntries(directory);
   }
   else if (strcmp(order, "help") == 0)
   {
      printf("\nAvailable Commands:\n");
      printf("  dir                  - List all files in the directory.\n");
      printf("  info                 - Display superblock information.\n");
      printf("  bytemaps             - Display byte maps information.\n");
      printf("  rename <old> <new>   - Rename a file.\n");
      printf("  print <file>         - Display the content of a file.\n");
      printf("  remove <file>        - Delete a file.\n");
      printf("  copy <src> <dst>     - Copy a file.\n");
      printf("  create <file> <cont> - Create a new file with given content.\n");
      printf("  clear                - Clear the terminal screen.\n");
      printf("  debug                - List directory entries (debug mode).\n");
      printf("  exit                 - Save changes and exit the program.\n\n");
   }
   else if (strcmp(order, "clear") == 0)
   {
      ClearScreen();
   }
   else if (strcmp(order, "exit") == 0)
   {
      // Ensure all changes are saved before exiting
      SaveAllChanges(directory, inodeBlock, byteMaps, superBlock, data, file);
      fclose(file);
      exit(0);
   }
   else
   {
      fprintf(stderr, "Error: Invalid command '%s'. Please try again.\n", order);
      fprintf(stderr, "Type 'help' to see the list of available commands.\n");
   }
}

// ---------------------------------------------------------------------------
// SAVE/LOAD OPERATIONS
// ---------------------------------------------------------------------------

/**
 * @brief Saves all major filesystem structures (inodes, directory, bytemaps,
 *        superblock, data blocks) in a single call.
 */
void SaveAllChanges(
    EXT_DIRECTORY_ENTRY *directory,
    EXT_INODE_BLOCK *inodeBlock,
    EXT_BYTE_MAPS *byteMaps,
    EXT_SIMPLE_SUPERBLOCK *superBlock,
    EXT_DATA *data,
    FILE *file)
{
   // 1. Save inodes and directory
   SaveInodesAndDirectory(directory, inodeBlock, file);
   // 2. Save byte maps
   SaveByteMaps(byteMaps, file);
   // 3. Save superblock
   SaveSuperBlock(superBlock, file);
   // 4. Save data blocks
   SaveData(data, file);
}

/**
 * @brief Writes the superblock to disk (block 0).
 */
void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock, FILE *file)
{
   fseek(file, BLOCK_SIZE * 0, SEEK_SET);
   if (fwrite(superBlock, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, file) != 1)
   {
      perror("Error saving SuperBlock");
   }
   fflush(file);
}

/**
 * @brief Writes the byte maps (block 1) to disk: block-bytemap and inode-bytemap.
 */
void SaveByteMaps(EXT_BYTE_MAPS *byteMaps, FILE *file)
{
   fseek(file, BLOCK_SIZE * 1, SEEK_SET);
   if (fwrite(byteMaps, sizeof(EXT_BYTE_MAPS), 1, file) != 1)
   {
      perror("Error saving ByteMaps");
   }
   fflush(file);
}

/**
 * @brief Writes the inode block (block 2) and directory (block 3) to disk.
 */
void SaveInodesAndDirectory(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodeBlock, FILE *file)
{
   // Inode block is at block 2
   fseek(file, BLOCK_SIZE * 2, SEEK_SET);
   if (fwrite(inodeBlock, sizeof(EXT_INODE_BLOCK), 1, file) != 1)
   {
      perror("Error saving InodeBlock");
   }

   // Directory is at block 3
   fseek(file, BLOCK_SIZE * 3, SEEK_SET);
   if (fwrite(directory, sizeof(EXT_DIRECTORY_ENTRY) * MAX_FILES, 1, file) != 1)
   {
      perror("Error saving Directory");
   }
   fflush(file);
}

/**
 * @brief Writes all data blocks (starting from block 4) to disk.
 */
void SaveData(EXT_DATA *data, FILE *file)
{
   fseek(file, BLOCK_SIZE * FIRST_DATA_BLOCK, SEEK_SET);
   if (fwrite(data, sizeof(EXT_DATA), MAX_DATA_BLOCKS, file) != MAX_DATA_BLOCKS)
   {
      perror("Error saving Data blocks");
   }
   fflush(file);
}


// ---------------------------------------------------------------------------
// FILESYSTEM AND COMMAND-RELATED FUNCTIONS
// ---------------------------------------------------------------------------

/**
 * @brief Lists all files in the directory along with their sizes, inodes, and allocated blocks.
 * 
 * Iterates through the directory entries, skipping empty ones and the special entry ".", 
 * retrieves corresponding inodes, and displays file information.
 * 
 * @param directory Pointer to the directory entries array.
 * @param inodes Pointer to the inode block structure.
 */
void ListDirectory(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes)
{
   int i, j;
   int fileCount = 0; // Counter for found files

   printf("List of files in the directory:\n");
   printf("-------------------------------------------------------\n");
   for (i = 0; i < MAX_FILES; i++)
   {
      // Skip empty entries and the special entry "."
      if (directory[i].inode == NULL_INODE || strcmp(directory[i].file_name, ".") == 0)
      {
         continue;
      }

      // Get the corresponding inode
      EXT_SIMPLE_INODE *inode = &inodes->inodes[directory[i].inode];

      // Print file name, size, and inode
      printf("\n%-20s size:%-6u inode:%-2d blocks:",
             directory[i].file_name, // File name
             inode->file_size,       // File size
             directory[i].inode);    // Inode number

      // Print occupied blocks
      for (j = 0; j < MAX_INODE_BLOCK_NUMS; j++)
      {
         if (inode->block_numbers[j] != NULL_BLOCK) // Skip unassigned blocks
         {
            printf(" %u", inode->block_numbers[j]);
         }
      }

      fileCount++; // Increment the file counter
   }
   printf("\n");

   // Check if no files were found
   if (fileCount == 0)
   {
      printf("No files in the directory.\n");
   }
}

/**
 * @brief Prints the metadata stored in the superblock.
 * 
 * Displays details such as the total number of inodes, total blocks, free blocks, 
 * free inodes, the starting data block, and the block size.
 * 
 * @param superBlock Pointer to the superblock structure.
 */
void PrintSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock)
{
   printf("\nSuperblock Information:\n");
   printf("Total inodes: %u\n", superBlock->total_inodes);
   printf("Total blocks: %u\n", superBlock->total_blocks);
   printf("Free blocks: %u\n", superBlock->free_blocks);
   printf("Free inodes: %u\n", superBlock->free_inodes);
   printf("First data block: %u\n", superBlock->first_data_block);
   printf("Block size: %u bytes\n", superBlock->block_size);
}

/**
 * @brief Displays the allocation status of inodes and data blocks.
 * 
 * Prints the inode bitmap and the first 25 entries of the block bitmap, 
 * showing which resources are currently allocated or free.
 * 
 * @param byteMaps Pointer to the structure containing the byte maps.
 */
void PrintByteMaps(EXT_BYTE_MAPS *byteMaps)
{
   int i;

   printf("\nByte maps information:");

   // Display the contents of the inode bytemap
   printf("\nInodes: ");
   for (i = 0; i < MAX_INODES; i++)
   {
      printf("%u ", byteMaps->inode_bytemap[i]);
   }
   printf("\n");
   // Display the contents of the block bytemap (first 25 elements)
   printf("Blocks [0-25]: ");
   for (i = 0; i < 25; i++)
   {
      printf("%u ", byteMaps->block_bytemap[i]);
   }
   printf("\n");
}

/**
 * @brief Displays the content of a specified file.
 * 
 * Finds the file in the directory, retrieves its associated inode, and reads
 * its data blocks to display the content of the file.
 * 
 * @param directory Pointer to the directory entries array.
 * @param inodes Pointer to the inode block structure.
 * @param data Pointer to the data blocks array.
 * @param name Name of the file to be printed.
 * @return 0 on success, -1 if the file is not found or an error occurs.
 */
int PrintFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_DATA *data, char *name)
{
   // Find the file using FindFile
   int fileIndex = FindFile(directory, inodes, name);

   if (fileIndex == -1)
   {
      printf("File '%s' not found.\n", name);
      return -1;
   }

   // Get the inode of the file
   EXT_SIMPLE_INODE *inode = &inodes->inodes[directory[fileIndex].inode];

   // Check if the file size is valid
   if (inode->file_size == 0)
   {
      printf("File '%s' is empty.\n", name);
      return 0;
   }

   // Allocate buffer to hold file content
   unsigned char *buffer = malloc(inode->file_size + 1); // +1 for null terminator
   if (!buffer)
   {
      perror("Memory allocation failed");
      return -1;
   }

   size_t bytesCopied = 0;
   memset(buffer, 0, inode->file_size + 1); // Initialize buffer

   for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
   {
      if (inode->block_numbers[i] == NULL_BLOCK)
      {
         continue;
      }

      int blockNumber = inode->block_numbers[i];

      // Ensure blockNumber is within valid range
      if (blockNumber < FIRST_DATA_BLOCK || blockNumber >= (FIRST_DATA_BLOCK + MAX_DATA_BLOCKS))
      {
         printf("Error: Invalid block number %d for file '%s'.\n", blockNumber, name);
         continue;
      }

      // Map block number to data array index
      int dataIndex = blockNumber - FIRST_DATA_BLOCK;

      // Boundary check
      if (dataIndex >= MAX_DATA_BLOCKS)
      {
         printf("Error: Data index %d out of bounds for block %d.\n", dataIndex, blockNumber);
         continue;
      }

      EXT_DATA *block = &data[dataIndex];

      // Determine how many bytes to copy from this block
      size_t bytesToCopy = (inode->file_size - bytesCopied) < BLOCK_SIZE ? (inode->file_size - bytesCopied) : BLOCK_SIZE;
      memcpy(buffer + bytesCopied, block->data, bytesToCopy);
      bytesCopied += bytesToCopy;

      if (bytesCopied >= inode->file_size)
      {
         break;
      }
   }

   buffer[inode->file_size] = '\0'; // Ensure null termination

   printf("Content of file '%s':\n%s\n", name, buffer);

   free(buffer);
   return 0;
}

/**
 * @brief Renames a file from oldName to newName if it exists and newName is not taken.
 * @return 0 on success, -1 on failure.
 */
int RenameFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *oldName, char *newName)
{
   if (!oldName || !newName || strlen(newName) == 0)
   {
      fprintf(stderr, "Invalid file names.\n");
      return -1;
   }

   int fileIndex = FindFile(directory, inodes, oldName);
   if (fileIndex == -1)
   {
      fprintf(stderr, "File '%s' not found.\n", oldName);
      return -1;
   }

   if (FindFile(directory, inodes, newName) != -1)
   {
      fprintf(stderr, "A file with the name '%s' already exists.\n", newName);
      return -1;
   }

   strncpy(directory[fileIndex].file_name, newName, sizeof(directory[fileIndex].file_name) - 1);
   directory[fileIndex].file_name[sizeof(directory[fileIndex].file_name) - 1] = '\0';
   printf("File renamed from '%s' to '%s'.\n", oldName, newName);
   return 0;
}

/**
 * @brief Deletes a file by freeing its data blocks, its inode, and removing its directory entry.
 * @return 0 on success, -1 on failure.
 */
int DeleteFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes,
               EXT_BYTE_MAPS *byteMaps, EXT_SIMPLE_SUPERBLOCK *superBlock, char *name)
{
   int fileIndex = FindFile(directory, inodes, name);
   if (fileIndex == -1)
   {
      fprintf(stderr, "File '%s' not found.\n", name);
      return -1;
   }

   int inodeIndex = directory[fileIndex].inode;
   EXT_SIMPLE_INODE *inode = &inodes->inodes[inodeIndex];

   // Free data blocks
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
   {
      if (inode->block_numbers[i] != NULL_BLOCK)
      {
         int blockNum = inode->block_numbers[i];
         byteMaps->block_bytemap[blockNum] = 0;
         inode->block_numbers[i] = NULL_BLOCK;
         superBlock->free_blocks++;
      }
   }

   // Free inode
   byteMaps->inode_bytemap[inodeIndex] = 0;
   memset(inode, 0, sizeof(EXT_SIMPLE_INODE));
   superBlock->free_inodes++;

   // Remove directory entry
   directory[fileIndex].inode = NULL_INODE;
   memset(directory[fileIndex].file_name, 0, sizeof(directory[fileIndex].file_name));

   printf("File '%s' deleted successfully.\n", name);
   return 0;
}

/**
 * @brief Copies an existing source file to a new destination file by allocating
 *        new blocks and a new inode for the destination. Writes data to 'particion.bin'.
 * @return 0 on success, -1 on failure.
 */
int CopyFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps,
             EXT_SIMPLE_SUPERBLOCK *superBlock, EXT_DATA *data,
             char *sourceName, char *destName, FILE *file)
{
   if (!directory || !inodes || !byteMaps || !superBlock || !data ||
       !sourceName || !destName || !file)
   {
      fprintf(stderr, "Invalid input parameters.\n");
      return -1;
   }

   // Check if destination already exists
   if (FindFile(directory, inodes, destName) != -1)
   {
      fprintf(stderr, "Destination file '%s' already exists.\n", destName);
      return -1;
   }

   // Locate source file
   int sourceIndex = FindFile(directory, inodes, sourceName);
   if (sourceIndex == -1)
   {
      fprintf(stderr, "Source file '%s' not found.\n", sourceName);
      return -1;
   }

   EXT_DIRECTORY_ENTRY *sourceEntry = &directory[sourceIndex];
   EXT_SIMPLE_INODE *sourceInode = &inodes->inodes[sourceEntry->inode];

   // Find a free inode for the new file
   int destInodeIndex = -1;
   for (int i = 0; i < MAX_INODES; i++)
   {
      // Usually inodes 0,1,2 are reserved in many simplified FS examples
      if (byteMaps->inode_bytemap[i] == 0 && i > 2)
      {
         destInodeIndex = i;
         break;
      }
   }
   if (destInodeIndex == -1)
   {
      fprintf(stderr, "No free inodes available.\n");
      return -1;
   }

   // Mark the inode as occupied
   byteMaps->inode_bytemap[destInodeIndex] = 1;
   superBlock->free_inodes--;

   // Initialize the destination inode
   EXT_SIMPLE_INODE *destInode = &inodes->inodes[destInodeIndex];
   destInode->file_size = sourceInode->file_size;
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
   {
      destInode->block_numbers[i] = NULL_BLOCK;
   }

   // Copy data blocks
   unsigned char buffer[BLOCK_SIZE];
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS && sourceInode->block_numbers[i] != NULL_BLOCK; i++)
   {
      // Find a free block
      int destBlockNum = -1;
      for (int j = FIRST_DATA_BLOCK; j < MAX_PARTITION_BLOCKS; j++)
      {
         if (byteMaps->block_bytemap[j] == 0)
         {
            destBlockNum = j;
            break;
         }
      }
      if (destBlockNum == -1)
      {
         fprintf(stderr, "No free blocks available to copy data.\n");
         // rollback the inode
         byteMaps->inode_bytemap[destInodeIndex] = 0;
         superBlock->free_inodes++;
         memset(destInode, 0, sizeof(EXT_SIMPLE_INODE));
         return -1;
      }

      byteMaps->block_bytemap[destBlockNum] = 1;
      superBlock->free_blocks--;
      destInode->block_numbers[i] = destBlockNum;

      // Calculate the source data index
      int dataIndex = sourceInode->block_numbers[i] - FIRST_DATA_BLOCK;
      if (dataIndex < 0 || dataIndex >= MAX_DATA_BLOCKS)
      {
         fprintf(stderr, "Invalid data index %d for block %d.\n", dataIndex, sourceInode->block_numbers[i]);
         // rollback
         for (int k = 0; k < i; k++)
         {
            if (destInode->block_numbers[k] != NULL_BLOCK)
            {
               byteMaps->block_bytemap[destInode->block_numbers[k]] = 0;
               superBlock->free_blocks++;
               destInode->block_numbers[k] = NULL_BLOCK;
            }
         }
         byteMaps->inode_bytemap[destInodeIndex] = 0;
         superBlock->free_inodes++;
         memset(destInode, 0, sizeof(EXT_SIMPLE_INODE));
         return -1;
      }

      // Copy from in-memory data array
      memcpy(buffer, data[dataIndex].data, BLOCK_SIZE);

      // Write the copied data to the new block in "particion.bin"
      long destOffset = destBlockNum * BLOCK_SIZE;
      if (fseek(file, destOffset, SEEK_SET) != 0)
      {
         fprintf(stderr, "Error seeking to destination block %d.\n", destBlockNum);
         return -1;
      }

      size_t bytesWritten = fwrite(buffer, 1, BLOCK_SIZE, file);
      if (bytesWritten != BLOCK_SIZE)
      {
         fprintf(stderr, "Error writing to destination block %d.\n", destBlockNum);
         return -1;
      }

      // Update the 'data' array in memory
      memcpy(data[destBlockNum - FIRST_DATA_BLOCK].data, buffer, BLOCK_SIZE);
   }

   // Find a free directory entry
   int destDirIndex = -1;
   for (int i = 0; i < MAX_FILES; i++)
   {
      if (directory[i].inode == NULL_INODE)
      {
         destDirIndex = i;
         break;
      }
   }
   if (destDirIndex == -1)
   {
      fprintf(stderr, "No free directory entries available.\n");
      // rollback inode and blocks
      for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
      {
         if (destInode->block_numbers[i] != NULL_BLOCK)
         {
            byteMaps->block_bytemap[destInode->block_numbers[i]] = 0;
            superBlock->free_blocks++;
            destInode->block_numbers[i] = NULL_BLOCK;
         }
      }
      byteMaps->inode_bytemap[destInodeIndex] = 0;
      superBlock->free_inodes++;
      memset(destInode, 0, sizeof(EXT_SIMPLE_INODE));
      return -1;
   }

   // Create the directory entry
   strncpy(directory[destDirIndex].file_name, destName, FILE_NAME_LENGTH - 1);
   directory[destDirIndex].file_name[FILE_NAME_LENGTH - 1] = '\0';
   directory[destDirIndex].inode = destInodeIndex;

   printf("File '%s' copied to '%s' successfully.\n", sourceName, destName);
   return 0;
}

/**
 * @brief Creates a new file with the specified name and content, allocating
 *        an inode and the required data blocks.

 */
int CreateFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps,
               EXT_SIMPLE_SUPERBLOCK *superBlock, EXT_DATA *data,
               char *fileName, char *content)
{
   // Check if file already exists
   if (FindFile(directory, inodes, fileName) != -1)
   {
      fprintf(stderr, "Error: File '%s' already exists.\n", fileName);
      return -1;
   }

   // Locate a free inode
   int inodeIndex = -1;
   for (int i = 0; i < MAX_INODES; i++)
   {
      if (byteMaps->inode_bytemap[i] == 0)
      {
         inodeIndex = i;
         break;
      }
   }
   if (inodeIndex == -1)
   {
      fprintf(stderr, "Error: No free inodes available.\n");
      return -1;
   }

   // Mark inode as used
   byteMaps->inode_bytemap[inodeIndex] = 1;
   superBlock->free_inodes--;

   // Initialize the inode
   EXT_SIMPLE_INODE *inode = &inodes->inodes[inodeIndex];
   inode->file_size = strlen(content);
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
   {
      inode->block_numbers[i] = NULL_BLOCK;
   }

   int bytesRemaining = inode->file_size;
   int contentOffset = 0;

   // Assign data blocks for this new file
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS && bytesRemaining > 0; i++)
   {
      // Find free block
      int blockNum = -1;
      for (int j = FIRST_DATA_BLOCK; j < MAX_PARTITION_BLOCKS; j++)
      {
         if (byteMaps->block_bytemap[j] == 0)
         {
            blockNum = j;
            break;
         }
      }
      if (blockNum == -1)
      {
         fprintf(stderr, "Error: No free blocks available to create file.\n");
         return -1;
      }

      // Mark block as used
      byteMaps->block_bytemap[blockNum] = 1;
      superBlock->free_blocks--;

      // Assign block to inode
      inode->block_numbers[i] = blockNum;

      // Copy content into this block
      int dataIndex = blockNum - FIRST_DATA_BLOCK;
      int bytesToCopy = (bytesRemaining < BLOCK_SIZE) ? bytesRemaining : BLOCK_SIZE;
      memcpy(data[dataIndex].data, content + contentOffset, bytesToCopy);
      contentOffset += bytesToCopy;
      bytesRemaining -= bytesToCopy;
   }

   // Create a directory entry
   for (int i = 0; i < MAX_FILES; i++)
   {
      if (directory[i].inode == NULL_INODE)
      {
         strncpy(directory[i].file_name, fileName, FILE_NAME_LENGTH - 1);
         directory[i].file_name[FILE_NAME_LENGTH - 1] = '\0';
         directory[i].inode = inodeIndex;
         printf("File '%s' created successfully.\n", fileName);
         return 0;
      }
   }

   fprintf(stderr, "Error: No free directory entries available.\n");
   return -1;
}

// ---------------------------------------------------------------------------
// DEBUG AND UTILITY FUNCTIONS
// ---------------------------------------------------------------------------

/**
 * @brief Finds a file by name and returns its index in the directory, or -1 if not found.
 */
int FindFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *name)
{
   for (int i = 0; i < MAX_FILES; i++)
   {
      if (directory[i].inode != NULL_INODE &&
          strcmp(directory[i].file_name, name) == 0)
      {
         return i;
      }
   }
   return -1;
}

/**
 * @brief Lists all directory entries (occupied or free) for debugging purposes.
 */
void DebugListAllDirectoryEntries(EXT_DIRECTORY_ENTRY *directory)
{
   printf("Debug: Listing all directory entries:\n");
   printf("-------------------------------------------------\n");
   for (int i = 0; i < MAX_FILES; i++)
   {
      printf("Entry %d: ", i);
      if (directory[i].inode != NULL_INODE && strlen(directory[i].file_name) > 0)
      {
         printf("Occupied - File Name: %s, Inode: %u\n",
                directory[i].file_name, directory[i].inode);
      }
      else
      {
         printf("Free\n");
      }
   }
   printf("-------------------------------------------------\n");
}

/**
 * @brief Clears the screen using a system() call. This is OS-dependent and
 *        generally discouraged in production environments but acceptable for
 *        academic simulations.
 */
void ClearScreen()
{
#ifdef _WIN32
   system("cls");
#else
   system("clear");
#endif
}