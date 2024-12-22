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
