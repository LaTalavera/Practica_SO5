#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "headers.h"

#define COMMAND_LENGTH 100

#ifndef TEST
int main()
{
   char command[COMMAND_LENGTH];
   char order[COMMAND_LENGTH];
   char argument1[COMMAND_LENGTH];
   char argument2[COMMAND_LENGTH];

   EXT_SIMPLE_SUPERBLOCK superBlock;
   EXT_BYTE_MAPS byteMaps;
   EXT_INODE_BLOCK inodeBlock;
   EXT_DIRECTORY_ENTRY directory[MAX_FILES];
   EXT_DATA data[MAX_DATA_BLOCKS];
   EXT_DATA fileData[MAX_PARTITION_BLOCKS];

   FILE *file = fopen("particion.bin", "r+b");
   if (file == NULL)
   {
      perror("Error opening file particion.bin");
      return 1;
   }
   fread(&fileData, BLOCK_SIZE, MAX_PARTITION_BLOCKS, file);

   memcpy(&superBlock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], BLOCK_SIZE);
   memcpy(&byteMaps, (EXT_BYTE_MAPS *)&fileData[1], BLOCK_SIZE);
   memcpy(&inodeBlock, (EXT_INODE_BLOCK *)&fileData[2], BLOCK_SIZE);
   memcpy(directory, (EXT_DIRECTORY_ENTRY *)&fileData[3], sizeof(EXT_DIRECTORY_ENTRY) * MAX_FILES);
   memcpy(&data, (EXT_DATA *)&fileData[4], MAX_DATA_BLOCKS * BLOCK_SIZE);

   // Command processing loop
   for (;;)
   {
      do
      {
         printf("\n>> ");
         fgets(command, COMMAND_LENGTH, stdin);
         char *newLine = strchr(command, '\n');
         if (newLine)
         {
            *newLine = '\0';
         }
         // clearInputBuffer(); // Clear the input buffer
      } while (CheckCommand(command, order, argument1, argument2) != 0);

      if (strcmp(order, "dir") == 0)
      {
         ListDirectory(directory, &inodeBlock);
         continue;
      }
      else if (strcmp(order, "info") == 0)
      {
         PrintSuperBlock(&superBlock);
         continue;
      }
      else if (strcmp(order, "bytemaps") == 0)
      {
         PrintByteMaps(&byteMaps);
         continue;
      }
      else if (strcmp(order, "create") == 0)
      {
         if (strlen(argument1) == 0 || strlen(argument2) == 0)
         {
            printf("Usage: create <file_name> <content>\n");
         }
         else
         {
            if (CreateFile(directory, &inodeBlock, &byteMaps, &superBlock, data, argument1, argument2) == 0)
            {
               SaveInodesAndDirectory(directory, &inodeBlock, file);
               SaveByteMaps(&byteMaps, file);
               SaveSuperBlock(&superBlock, file);
               SaveData(data, file); // Save data after creating the file
            }
         }
         continue;
      }

      else if (strcmp(order, "rename") == 0)
      {
         if (strlen(argument1) == 0 || strlen(argument2) == 0)
         {
            printf("Usage: rename <old_name> <new_name>\n");
         }
         else
         {
            if (RenameFile(directory, &inodeBlock, argument1, argument2) == 0)
            {
               SaveInodesAndDirectory(directory, &inodeBlock, file);
               SaveByteMaps(&byteMaps, file);
               SaveSuperBlock(&superBlock, file);
            }
         }
         continue;
      }
      else if (strcmp(order, "print") == 0)
      {
         if (strlen(argument1) == 0)
         {
            printf("Usage: print <file_name>\n");
         }
         else
         {
            PrintFile(directory, &inodeBlock, data, argument1);
         }
         continue;
      }
      else if (strcmp(order, "remove") == 0)
      {
         if (strlen(argument1) == 0)
         {
            printf("Usage: remove <file_name>\n");
         }
         else
         {
            if (DeleteFile(directory, &inodeBlock, &byteMaps, &superBlock, argument1) == 0)
            {
               SaveInodesAndDirectory(directory, &inodeBlock, file);
               SaveByteMaps(&byteMaps, file);
               SaveSuperBlock(&superBlock, file);
               SaveData(data, file); // Save data after deletion
            }
         }
         continue;
      }
      else if (strcmp(order, "copy") == 0)
      {
         if (strlen(argument1) == 0 || strlen(argument2) == 0)
         {
            printf("Usage: copy <source_file> <destination_file>\n");
         }
         else
         {
            if (CopyFile(directory, &inodeBlock, &byteMaps, &superBlock, data, argument1, argument2, file) == 0)
            {
               SaveInodesAndDirectory(directory, &inodeBlock, file);
               SaveByteMaps(&byteMaps, file);
               SaveSuperBlock(&superBlock, file);
               SaveData(data, file); // Save data after copying
            }
         }
         continue;
      }
      else if (strcmp(command, "debug") == 0)
      {
         DebugListAllDirectoryEntries(directory);
      }
      else if (strcmp(order, "help") == 0)
      {
         printf("\nAvailable Commands:\n");
         printf("  dir                       - List all files in the directory.\n");
         printf("  info                      - Display superblock information.\n");
         printf("  bytemaps                  - Display byte maps information.\n");
         printf("  create <name> <content>   - Create a new file.\n");
         printf("  rename <old> <new>        - Rename a file.\n");
         printf("  print <file>              - Display the content of a file.\n");
         printf("  remove <file>             - Delete a file.\n");
         printf("  copy <src> <dst>          - Copy a file.\n");
         printf("  clear                     - Clear the terminal screen.\n");
         printf("  debug                     - List all directory entries (debug mode).\n");
         printf("  exit                      - Save changes and exit the program.\n\n");
         continue;
      }
      else if (strcmp(order, "clear") == 0)
      {
         ClearScreen();
         continue;
      }
      else if (strcmp(order, "exit") == 0)
      {
         SaveInodesAndDirectory(directory, &inodeBlock, file);
         SaveByteMaps(&byteMaps, file);
         SaveSuperBlock(&superBlock, file);
         SaveData(data, file);
         fclose(file);
         return 0;
      }
      else
      {
         printf("Error: Invalid command '%s'. Please try again.\n", order);
         printf("Type 'help' to see the list of available commands.\n");
         continue;
      }
   }
}
#endif

int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2)
{
   // Split the command into tokens
   char *token;
   char commandCopy[COMMAND_LENGTH];
   strncpy(commandCopy, commandStr, COMMAND_LENGTH);
   commandCopy[COMMAND_LENGTH - 1] = '\0';

   // Remove the newline character
   char *newline = strchr(commandCopy, '\n');
   if (newline)
      *newline = '\0';

   // Get the first token (command)
   token = strtok(commandCopy, " ");
   if (token == NULL)
      return -1;
   strcpy(command, token);

   // Get the second token (first argument)
   token = strtok(NULL, " ");
   if (token)
      strcpy(arg1, token);
   else
      arg1[0] = '\0';

   // Get the third token (second argument)
   token = strtok(NULL, " ");
   if (token)
      strcpy(arg2, token);
   else
      arg2[0] = '\0';

   return 0;
}

void PrintSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock)
{
   // Print superblock information
   printf("\nSuperblock Information:\n");
   printf("Total inodes: %u\n", superBlock->total_inodes);
   printf("Total blocks: %u\n", superBlock->total_blocks);
   printf("Free blocks: %u\n", superBlock->free_blocks);
   printf("Free inodes: %u\n", superBlock->free_inodes);
   printf("First data block: %u\n", superBlock->first_data_block);
   printf("Block size: %u bytes\n", superBlock->block_size);
}

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

int FindFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *name)
{
   int i;

   // Iterate through the directory to find the file
   for (i = 0; i < MAX_FILES; i++)
   {
      if (directory[i].inode != NULL_INODE && strcmp(directory[i].file_name, name) == 0)
      {
         return i; // Return the index of the file
      }
   }

   return -1; // File not found
}

int RenameFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *oldName, char *newName)
{
   // Verify that the file names are not null or empty
   if (oldName == NULL || newName == NULL || strlen(newName) == 0)
   {
      printf("Invalid file names.\n");
      return -1;
   }

   // Find the file with the old name
   int fileIndex = FindFile(directory, inodes, oldName);
   if (fileIndex == -1)
   {
      printf("File '%s' not found.\n", oldName);
      return -1;
   }

   // Check if a file with the new name already exists
   if (FindFile(directory, inodes, newName) != -1)
   {
      printf("A file with the name '%s' already exists.\n", newName);
      return -1;
   }

   // Rename the file
   strncpy(directory[fileIndex].file_name, newName, sizeof(directory[fileIndex].file_name) - 1);
   directory[fileIndex].file_name[sizeof(directory[fileIndex].file_name) - 1] = '\0'; // Ensure null termination
   printf("File renamed from '%s' to '%s'.\n", oldName, newName);

   return 0;
}

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

int DeleteFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps, EXT_SIMPLE_SUPERBLOCK *superBlock, char *name)
{
   int fileIndex = FindFile(directory, inodes, name);
   if (fileIndex == -1)
   {
      printf("File '%s' not found.\n", name);
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
         byteMaps->block_bytemap[blockNum] = 0; // Mark block as free
         inode->block_numbers[i] = NULL_BLOCK;  // Reset block number
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
int CopyFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps,
             EXT_SIMPLE_SUPERBLOCK *superBlock, EXT_DATA *data,
             char *sourceName, char *destName, FILE *file)
{
   // Validate input parameters
   if (directory == NULL || inodes == NULL || byteMaps == NULL ||
       superBlock == NULL || data == NULL ||
       sourceName == NULL || destName == NULL || file == NULL)
   {
      printf("Invalid input parameters.\n");
      return -1;
   }

   // Check if the destination file already exists
   if (FindFile(directory, inodes, destName) != -1)
   {
      printf("Destination file '%s' already exists.\n", destName);
      return -1;
   }

   // Find the source file
   int sourceIndex = FindFile(directory, inodes, sourceName);
   if (sourceIndex == -1)
   {
      printf("Source file '%s' not found.\n", sourceName);
      return -1;
   }

   EXT_DIRECTORY_ENTRY *sourceEntry = &directory[sourceIndex];
   EXT_SIMPLE_INODE *sourceInode = &inodes->inodes[sourceEntry->inode];

   // Find the first free inode
   int destInodeIndex = -1;
   for (int i = 0; i < MAX_INODES; i++)
   {
      if (byteMaps->inode_bytemap[i] == 0 && i > 2) // inodes 0,1 reserved, 2 directory
      {
         destInodeIndex = i;
         break;
      }
   }

   if (destInodeIndex == -1)
   {
      printf("No free inodes available.\n");
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

   unsigned char buffer[BLOCK_SIZE];

   // Copy data blocks from memory instead of disk
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS && sourceInode->block_numbers[i] != NULL_BLOCK; i++)
   {
      // Find the first free block
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
         printf("No free blocks available to copy data.\n");
         // Free allocated resources before exiting
         byteMaps->inode_bytemap[destInodeIndex] = 0;
         superBlock->free_inodes++;
         memset(destInode, 0, sizeof(EXT_SIMPLE_INODE));
         return -1;
      }

      // Mark the block as occupied
      byteMaps->block_bytemap[destBlockNum] = 1;
      superBlock->free_blocks--;

      // Assign the block number to the destination inode
      destInode->block_numbers[i] = destBlockNum;

      // Calculate the data index for the source block in memory
      int dataIndex = sourceInode->block_numbers[i] - FIRST_DATA_BLOCK;
      if (dataIndex < 0 || dataIndex >= MAX_DATA_BLOCKS)
      {
         printf("Invalid data index %d for block %d.\n", dataIndex, sourceInode->block_numbers[i]);
         // Rollback changes
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

      // Copy block from in-memory data
      memcpy(buffer, data[dataIndex].data, BLOCK_SIZE);

      // Write data to the destination block on disk
      long destOffset = destBlockNum * BLOCK_SIZE;
      if (fseek(file, destOffset, SEEK_SET) != 0)
      {
         printf("Error seeking to destination block %d.\n", destBlockNum);
         return -1;
      }

      size_t bytesWritten = fwrite(buffer, 1, BLOCK_SIZE, file);
      if (bytesWritten != BLOCK_SIZE)
      {
         printf("Error writing to destination block %d.\n", destBlockNum);
         return -1;
      }

      // Update the data array in memory for the destination block
      memcpy(data[destBlockNum - FIRST_DATA_BLOCK].data, buffer, BLOCK_SIZE);
   }

   // Find the first vacant directory entry
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
      printf("No free directory entries available.\n");
      // Free allocated inode and blocks
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

   // Create a new directory entry for the copied file
   strncpy(directory[destDirIndex].file_name, destName, FILE_NAME_LENGTH - 1);
   directory[destDirIndex].file_name[FILE_NAME_LENGTH - 1] = '\0'; // Ensure null termination
   directory[destDirIndex].inode = destInodeIndex;

   printf("File '%s' copied to '%s' successfully.\n", sourceName, destName);
   return 0;
}

void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock, FILE *file)
{
   fseek(file, BLOCK_SIZE * 0, SEEK_SET); // Block 0
   if (fwrite(superBlock, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, file) != 1)
   {
      perror("Error saving SuperBlock");
   }
   fflush(file);
}

void SaveByteMaps(EXT_BYTE_MAPS *byteMaps, FILE *file)
{
   fseek(file, BLOCK_SIZE * 1, SEEK_SET); // Block 1
   if (fwrite(byteMaps, sizeof(EXT_BYTE_MAPS), 1, file) != 1)
   {
      perror("Error saving ByteMaps");
   }
   fflush(file);
}

void SaveInodesAndDirectory(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodeBlock, FILE *file)
{
   // Save InodeBlock to Block 2
   fseek(file, BLOCK_SIZE * 2, SEEK_SET); // Block 2
   if (fwrite(inodeBlock, sizeof(EXT_INODE_BLOCK), 1, file) != 1)
   {
      perror("Error saving InodeBlock");
   }

   // Save Directory to Block 3
   fseek(file, BLOCK_SIZE * 3, SEEK_SET); // Block 3
   if (fwrite(directory, sizeof(EXT_DIRECTORY_ENTRY) * MAX_FILES, 1, file) != 1)
   {
      perror("Error saving Directory");
   }
   fflush(file);
}

void SaveData(EXT_DATA *data, FILE *file)
{
   fseek(file, BLOCK_SIZE * FIRST_DATA_BLOCK, SEEK_SET); // Starting from Block 4 (512 * 4 = 2048)
   if (fwrite(data, sizeof(EXT_DATA), MAX_DATA_BLOCKS, file) != MAX_DATA_BLOCKS)
   {
      perror("Error saving Data blocks");
   }
   fflush(file);
}

// helper functions
void DebugListAllDirectoryEntries(EXT_DIRECTORY_ENTRY *directory)
{
   printf("Debug: Listing all directory entries:\n");
   printf("-------------------------------------------------\n");
   for (int i = 0; i < MAX_FILES; i++)
   {
      printf("Entry %d: ", i);
      if (directory[i].inode != NULL_INODE && strlen(directory[i].file_name) > 0)
      {
         printf("Occupied - File Name: %s, Inode: %u\n", directory[i].file_name, directory[i].inode);
      }
      else
      {
         printf("Free\n");
      }
   }
   printf("-------------------------------------------------\n");
}

void ClearScreen()
{
#ifdef _WIN32
   system("cls");
#else
   system("clear");
#endif
}

int CreateFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps,
               EXT_SIMPLE_SUPERBLOCK *superBlock, EXT_DATA *data, char *fileName, char *content)
{
   // Check if the file name already exists
   if (FindFile(directory, inodes, fileName) != -1)
   {
      printf("Error: File '%s' already exists.\n", fileName);
      return -1;
   }

   // Find a free inode
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
      printf("Error: No free inodes available.\n");
      return -1;
   }

   // Mark the inode as used
   byteMaps->inode_bytemap[inodeIndex] = 1;
   superBlock->free_inodes--;

   // Initialize the inode
   EXT_SIMPLE_INODE *inode = &inodes->inodes[inodeIndex];
   inode->file_size = strlen(content);
   for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
   {
      inode->block_numbers[i] = NULL_BLOCK;
   }

   // Copy the content into blocks
   int bytesRemaining = inode->file_size;
   int contentOffset = 0;

   for (int i = 0; i < MAX_INODE_BLOCK_NUMS && bytesRemaining > 0; i++)
   {
      // Find a free block
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
         printf("Error: No free blocks available.\n");
         return -1;
      }

      // Mark the block as used
      byteMaps->block_bytemap[blockNum] = 1;
      superBlock->free_blocks--;

      // Assign the block to the inode
      inode->block_numbers[i] = blockNum;

      // Copy content to the block
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
         directory[i].file_name[FILE_NAME_LENGTH - 1] = '\0'; // Ensure null termination
         directory[i].inode = inodeIndex;
         printf("File '%s' created successfully.\n", fileName);
         return 0;
      }
   }

   printf("Error: No free directory entries available.\n");
   return -1;
}
