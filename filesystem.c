#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "headers.h"

#define COMMAND_LENGTH 100

void PrintByteMaps(EXT_BYTE_MAPS *byteMaps);
int CheckCommand(char *commandStr, char *command, char *arg1, char *arg2);
void ReadSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock);
void PrintSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock); //for the info command.
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

int main()
{
   char command[COMMAND_LENGTH];
   char order[COMMAND_LENGTH];
   char argument1[COMMAND_LENGTH];
   char argument2[COMMAND_LENGTH];

   int i, j;
   unsigned long int m;
   EXT_SIMPLE_SUPERBLOCK superBlock;
   EXT_BYTE_MAPS byteMaps;
   EXT_INODE_BLOCK inodeBlock;
   EXT_DIRECTORY_ENTRY directory[MAX_FILES];
   EXT_DATA data[MAX_DATA_BLOCKS];
   EXT_DATA fileData[MAX_PARTITION_BLOCKS];
   int directoryEntry;
   int saveDataFlag;
   FILE *file;

   // Read the entire file at once
   // MORE CODE...

   file = fopen("partition.bin", "r+b");
   fread(&fileData, BLOCK_SIZE, MAX_PARTITION_BLOCKS, file);

   memcpy(&superBlock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], BLOCK_SIZE);
   memcpy(&byteMaps, (EXT_BYTE_MAPS *)&fileData[1], BLOCK_SIZE);
   memcpy(&inodeBlock, (EXT_INODE_BLOCK *)&fileData[2], BLOCK_SIZE);
   memcpy(&directory, (EXT_DIRECTORY_ENTRY *)&fileData[3], BLOCK_SIZE);
   memcpy(&data, (EXT_DATA *)&fileData[4], MAX_DATA_BLOCKS * BLOCK_SIZE);

   // Command processing loop
   for (;;)
   {
      do
      {
         printf(">> ");
         fflush(stdin);
         fgets(command, COMMAND_LENGTH, stdin);
      } while (CheckCommand(command, order, argument1, argument2) != 0);
      if (strcmp(order, "dir") == 0)
      {
         ListDirectory(&directory, &inodeBlock);
         continue;
      }
      // MORE CODE...
      // Write metadata in rename, remove, copy commands
      SaveInodesAndDirectory(&directory, &inodeBlock, file);
      SaveByteMaps(&byteMaps, file);
      SaveSuperBlock(&superBlock, file);
      if (saveDataFlag)
         SaveData(&data, file);
      saveDataFlag = 0;
      // If the command is exit, all metadata will have been written
      // missing data and close
      if (strcmp(order, "exit") == 0)
      {
         SaveData(&data, file);
         fclose(file);
         return 0;
      }
   }
}