#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "headers.h"

#define COMMAND_LENGTH 100
// TODO check relation between all these functions and the exercise requirementes, to make sure we need them all
void PrintByteMaps(EXT_BYTE_MAPS *byteMaps);
int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2);
void ReadSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock);
void PrintSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock); // for the info command.
int FindFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *name);
void ListDirectory(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes);

int RenameFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *oldName, char *newName);
int PrintFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_DATA *data, char *name);
int DeleteFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps, EXT_SIMPLE_SUPERBLOCK *superBlock, char *name, FILE *file);
int CopyFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, EXT_BYTE_MAPS *byteMaps, EXT_SIMPLE_SUPERBLOCK *superBlock, EXT_DATA *data, char *sourceName, char *destName, FILE *file);

void SaveInodesAndDirectory(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, FILE *file);
void SaveByteMaps(EXT_BYTE_MAPS *byteMaps, FILE *file);
void SaveSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock, FILE *file);
void SaveData(EXT_DATA *data, FILE *file);

void clearInputBuffer()
{
   int c;
   while ((c = getchar()) != '\n' && c != EOF)
      ;
}

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
   memcpy(directory, (EXT_DIRECTORY_ENTRY *)&fileData[3], sizeof(EXT_DIRECTORY_ENTRY) * MAX_FILES); // TODO double check if this is correct or is ok as it comes in the example file
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
      else if (strcmp(order, "rename") == 0)
      {
         if (strlen(argument1) == 0 || strlen(argument2) == 0)
         {
            printf("Usage: rename <old_name> <new_name>\n");
         }
         else
         {
            RenameFile(directory, &inodeBlock, argument1, argument2);
         }
         continue;
      }
      else if (strcmp(order, "exit") == 0)
      {
         // TODO uncomment it out once Savedata is implemented
         //  SaveData(data, file);
         fclose(file);
         return 0;
      }
   }
}

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
   EXT_SIMPLE_INODE *inode;

   for (i = 0; i < MAX_FILES; i++)
   {
      // Skip empty entries and the special entry "."
      if (directory[i].inode == 0xFFFF || strcmp(directory[i].file_name, ".") == 0)
      {
         continue;
      }

      // Get the corresponding inode
      inode = &inodes->inodes[directory[i].inode];

      // Print file name, size, and inode
      printf("\n%-20s size:%-6u inode:%-2d blocks:",
             directory[i].file_name, // File name
             inode->file_size,       // File size
             directory[i].inode);    // Inode number

      // Print occupied blocks
      for (j = 0; j < MAX_INODE_BLOCK_NUMS; j++)
      {
         if (inode->block_numbers[j] != 0xFFFF) // Skip unassigned blocks
         {
            printf(" %u", inode->block_numbers[j]);
         }
      }

      printf("\n");
   }
}

int FindFile(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes, char *name)
{
    int i;

    // Iterate through the directory to find the file
    for (i = 0; i < MAX_FILES; i++)
    {
        if (directory[i].inode != 0xFFFF && strcmp(directory[i].file_name, name) == 0)
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
