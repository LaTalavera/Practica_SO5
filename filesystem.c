#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "headers.h"

#define COMMAND_LENGTH 100

// Command processing loop
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

      // 1) Open the "particion.bin" file simulating a disk partition
      FILE *file = fopen("particion.bin", "r+b");
      if (file == NULL)
      {
         perror("Error opening file particion.bin");
         return 1;
      }

      // 2) Read the entire partition into memory
      fread(&fileData, BLOCK_SIZE, MAX_PARTITION_BLOCKS, file);

      // 3) Extract each structure (superblock, bytemaps, inodes, directory, data)
      //    from the loaded blocks in memory.
      memcpy(&superBlock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], BLOCK_SIZE);
      memcpy(&byteMaps, (EXT_BYTE_MAPS *)&fileData[1], BLOCK_SIZE);
      memcpy(&inodeBlock, (EXT_INODE_BLOCK *)&fileData[2], BLOCK_SIZE);
      memcpy(directory, (EXT_DIRECTORY_ENTRY *)&fileData[3], sizeof(EXT_DIRECTORY_ENTRY) * MAX_FILES);
      memcpy(&data, (EXT_DATA *)&fileData[4], MAX_DATA_BLOCKS * BLOCK_SIZE);

      // 4) Main loop: read commands until user exits
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
#endif

   // ---------------------------------------------------------------------------
   // COMMAND PARSING AND PROCESSING
   // ---------------------------------------------------------------------------

   /**
    * @brief Splits the user input into 'order', 'arg1', and 'arg2'.
    * @return 0 if successfully parses something, or -1 on failure.
    */
   int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2)
   {
      char commandCopy[COMMAND_LENGTH];
      strncpy(commandCopy, commandStr, COMMAND_LENGTH);
      commandCopy[COMMAND_LENGTH - 1] = '\0';

      // Remove any newline
      char *newline = strchr(commandCopy, '\n');
      if (newline)
         *newline = '\0';

      // Extract main command token
      char *token = strtok(commandCopy, " ");
      if (token == NULL)
         return -1;
      strcpy(command, token);

      // Extract first argument
      token = strtok(NULL, " ");
      if (token)
         strcpy(arg1, token);
      else
         arg1[0] = '\0';

      // Extract second argument
      token = strtok(NULL, " ");
      if (token)
         strcpy(arg2, token);
      else
         arg2[0] = '\0';

      return 0;
   }

   /**
    * @brief Dispatches the user command to the appropriate function. This keeps
    *        'main()' clean and maintainable.
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
      // Simple command dispatcher
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
      else if (strcmp(order, "debug") == 0)
      {
         DebugListAllDirectoryEntries(directory);
      }
      else if (strcmp(order, "help") == 0)
      {
         // Show usage
         printf("\nAvailable Commands:\n");
         printf("  dir                  - List all files in the directory.\n");
         printf("  info                 - Display superblock information.\n");
         printf("  bytemaps             - Display byte maps information.\n");
         printf("  rename <old> <new>   - Rename a file.\n");
         printf("  print <file>         - Display the content of a file.\n");
         printf("  remove <file>        - Delete a file.\n");
         printf("  copy <src> <dst>     - Copy a file.\n");
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
         // Save changes, then exit
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
       EXT_DIRECTORY_ENTRY * directory,
       EXT_INODE_BLOCK * inodeBlock,
       EXT_BYTE_MAPS * byteMaps,
       EXT_SIMPLE_SUPERBLOCK * superBlock,
       EXT_DATA * data,
       FILE * file)
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
   void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK * superBlock, FILE * file)
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
   void SaveByteMaps(EXT_BYTE_MAPS * byteMaps, FILE * file)
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
   void SaveInodesAndDirectory(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodeBlock, FILE * file)
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
   void SaveData(EXT_DATA * data, FILE * file)
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
   // (Renaming, Deleting, Copying, Printing, Listing, etc.)
   // ---------------------------------------------------------------------------

   /**
    * @brief Renames a file from oldName to newName if it exists and newName is not taken.
    * @return 0 on success, -1 on failure.
    */
   int RenameFile(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodes, char *oldName, char *newName)
   {
      if (oldName == NULL || newName == NULL || strlen(newName) == 0)
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
    * @brief Deletes a file (frees data blocks, inode, directory entry).
    * @return 0 on success, -1 on failure.
    */
   int DeleteFile(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodes, EXT_BYTE_MAPS * byteMaps,
                  EXT_SIMPLE_SUPERBLOCK * superBlock, char *name)
   {
      int fileIndex = FindFile(directory, inodes, name);
      if (fileIndex == -1)
      {
         fprintf(stderr, "File '%s' not found.\n", name);
         return -1;
      }

      int inodeIndex = directory[fileIndex].inode;
      EXT_SIMPLE_INODE *inode = &inodes->inodes[inodeIndex];

      // Free data blocks used by this file
      for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
      {
         if (inode->block_numbers[i] != NULL_BLOCK)
         {
            int blockNum = inode->block_numbers[i];
            byteMaps->block_bytemap[blockNum] = 0; // Mark block as free
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
    * @brief Copies an existing source file to a new destination file, allocating new
    *        blocks and inode for the destination. Writes data to 'particion.bin'.
    * @return 0 on success, -1 on failure.
    */
   int CopyFile(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodes, EXT_BYTE_MAPS * byteMaps,
                EXT_SIMPLE_SUPERBLOCK * superBlock, EXT_DATA * data,
                char *sourceName, char *destName, FILE *file)
   {
      // Basic parameter checks
      if (!directory || !inodes || !byteMaps || !superBlock || !data ||
          !sourceName || !destName || !file)
      {
         fprintf(stderr, "Invalid input parameters.\n");
         return -1;
      }

      // Check if the destination already exists
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

      // Find a free inode for the destination
      int destInodeIndex = -1;
      for (int i = 0; i < MAX_INODES; i++)
      {
         // skip reserved inodes: 0,1 and 2 is for directory in many simplified FS
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

      // Initialize destination inode
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
         // Find a free block for the destination
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

         // Mark and update free blocks
         byteMaps->block_bytemap[destBlockNum] = 1;
         superBlock->free_blocks--;
         destInode->block_numbers[i] = destBlockNum;

         // Calculate source data index
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

      // Find a free directory entry for the new file
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

      // Create the new directory entry
      strncpy(directory[destDirIndex].file_name, destName, FILE_NAME_LENGTH - 1);
      directory[destDirIndex].file_name[FILE_NAME_LENGTH - 1] = '\0';
      directory[destDirIndex].inode = destInodeIndex;

      printf("File '%s' copied to '%s' successfully.\n", sourceName, destName);
      return 0;
   }

   /**
    * @brief Prints the contents of a file if it exists, reading from the in-memory data array.
    * @return 0 if file is empty or success, -1 on error.
    */
   int PrintFile(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodes, EXT_DATA * data, char *name)
   {
      int fileIndex = FindFile(directory, inodes, name);
      if (fileIndex == -1)
      {
         fprintf(stderr, "File '%s' not found.\n", name);
         return -1;
      }

      EXT_SIMPLE_INODE *inode = &inodes->inodes[directory[fileIndex].inode];
      if (inode->file_size == 0)
      {
         printf("File '%s' is empty.\n", name);
         return 0;
      }

      unsigned char *buffer = malloc(inode->file_size + 1);
      if (!buffer)
      {
         perror("Memory allocation failed");
         return -1;
      }

      memset(buffer, 0, inode->file_size + 1);
      size_t bytesCopied = 0;

      for (int i = 0; i < MAX_INODE_BLOCK_NUMS; i++)
      {
         if (inode->block_numbers[i] == NULL_BLOCK)
            continue;

         int blockNumber = inode->block_numbers[i];
         if (blockNumber < FIRST_DATA_BLOCK || blockNumber >= (FIRST_DATA_BLOCK + MAX_DATA_BLOCKS))
         {
            fprintf(stderr, "Error: Invalid block number %d for file '%s'.\n", blockNumber, name);
            continue;
         }

         int dataIndex = blockNumber - FIRST_DATA_BLOCK;
         if (dataIndex >= MAX_DATA_BLOCKS)
         {
            fprintf(stderr, "Error: Data index %d out of bounds for block %d.\n", dataIndex, blockNumber);
            continue;
         }

         EXT_DATA *block = &data[dataIndex];
         size_t bytesToCopy = (inode->file_size - bytesCopied) < BLOCK_SIZE
                                  ? (inode->file_size - bytesCopied)
                                  : BLOCK_SIZE;

         memcpy(buffer + bytesCopied, block->data, bytesToCopy);
         bytesCopied += bytesToCopy;

         if (bytesCopied >= inode->file_size)
            break;
      }

      buffer[inode->file_size] = '\0';
      printf("Content of file '%s':\n%s\n", name, buffer);
      free(buffer);
      return 0;
   }

   /**
    * @brief Lists all files in the directory along with their sizes, inodes, and occupied blocks.
    */
   void ListDirectory(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodes)
   {
      int fileCount = 0;
      printf("List of files in the directory:\n");
      printf("-------------------------------------------------------\n");

      for (int i = 0; i < MAX_FILES; i++)
      {
         if (directory[i].inode == NULL_INODE || strcmp(directory[i].file_name, ".") == 0)
            continue;

         EXT_SIMPLE_INODE *inode = &inodes->inodes[directory[i].inode];
         printf("\n%-20s size:%-6u inode:%-2d blocks:",
                directory[i].file_name,
                inode->file_size,
                directory[i].inode);

         // Print the occupied blocks for this file
         for (int j = 0; j < MAX_INODE_BLOCK_NUMS; j++)
         {
            if (inode->block_numbers[j] != NULL_BLOCK)
            {
               printf(" %u", inode->block_numbers[j]);
            }
         }
         fileCount++;
      }
      printf("\n");

      if (fileCount == 0)
      {
         printf("No files in the directory.\n");
      }
   }

   /**
    * @brief Prints Superblock information (inodes, blocks, free inodes, free blocks, etc.).
    */
   void PrintSuperBlock(EXT_SIMPLE_SUPERBLOCK * superBlock)
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
    * @brief Prints the byte maps of inodes and blocks to help debug the allocation status.
    */
   void PrintByteMaps(EXT_BYTE_MAPS * byteMaps)
   {
      printf("\nByte maps information:");
      printf("\nInodes: ");
      for (int i = 0; i < MAX_INODES; i++)
      {
         printf("%u ", byteMaps->inode_bytemap[i]);
      }
      printf("\nBlocks [0-25]: ");
      for (int i = 0; i < 25; i++)
      {
         printf("%u ", byteMaps->block_bytemap[i]);
      }
      printf("\n");
   }

   /**
    * @brief Finds a file by name and returns its index in the directory, or -1 if not found.
    */
   int FindFile(EXT_DIRECTORY_ENTRY * directory, EXT_INODE_BLOCK * inodes, char *name)
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
   void DebugListAllDirectoryEntries(EXT_DIRECTORY_ENTRY * directory)
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
    * @brief Clears the screen using system() calls. This is platform-dependent and generally
    *        not recommended in production, but acceptable for an academic simulator.
    */
   void ClearScreen()
   {
#ifdef _WIN32
      system("cls");
#else
      system("clear");
#endif
   }