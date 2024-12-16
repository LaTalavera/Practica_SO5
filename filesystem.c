#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "headers.h"

#define COMMAND_LENGTH 100

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
   FILE *file;

   file = fopen("particion.bin", "r+b");
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
         printf(">> ");
         fgets(command, COMMAND_LENGTH, stdin);
         clearInputBuffer(); // Clear the input buffer
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
      if (strcmp(order, "exit") == 0)
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
   // Separar el comando en tokens
   char *token;
   char commandCopy[COMMAND_LENGTH];
   strncpy(commandCopy, commandStr, COMMAND_LENGTH);
   commandCopy[COMMAND_LENGTH - 1] = '\0';

   // Quitar el salto de línea
   char *newline = strchr(commandCopy, '\n');
   if (newline)
      *newline = '\0';

   // Obtener el primer token (comando)
   token = strtok(commandCopy, " ");
   if (token == NULL)
      return -1;
   strcpy(command, token);

   // Obtener el segundo token (primer argumento)
   token = strtok(NULL, " ");
   if (token)
      strcpy(arg1, token);
   else
      arg1[0] = '\0';

   // Obtener el tercer token (segundo argumento)
   token = strtok(NULL, " ");
   if (token)
      strcpy(arg2, token);
   else
      arg2[0] = '\0';

   return 0;
}

void PrintSuperBlock(EXT_SIMPLE_SUPERBLOCK *superBlock)
{
   // Imprimir información del superbloque
   printf("Informacion del Superbloque:\n");
   printf("Inodos totales: %u\n", superBlock->total_inodes);
   printf("Bloques totales: %u\n", superBlock->total_blocks);
   printf("Bloques libres: %u\n", superBlock->free_blocks);
   printf("Inodos libres: %u\n", superBlock->free_inodes);
   printf("Primer bloque de datos: %u\n", superBlock->first_data_block);
   printf("Tamanio de bloque: %u bytes\n", superBlock->block_size);
}

void PrintByteMaps(EXT_BYTE_MAPS *byteMaps)
{
   int i;

   printf("Informacion bytemaps:");

   // Mostrar el contenido del bytemap de inodos
   printf("\nInodos: ");
   for (i = 0; i < MAX_INODES; i++)
   {
      printf("%u ", byteMaps->inode_bytemap[i]);
   }
   printf("\n");
   // Mostrar el contenido del bytemap de bloques (primeros 25 elementos)
   printf("Bloques [0-25]: ");
   for (i = 0; i < 25; i++)
   {
      printf("%u ", byteMaps->block_bytemap[i]);
   }
   printf("\n");
   printf(">>");
}

void ListDirectory(EXT_DIRECTORY_ENTRY *directory, EXT_INODE_BLOCK *inodes)
{
   int i, j;
   EXT_SIMPLE_INODE *inode;

   for (i = 0; i < MAX_FILES; i++)
   {
      // Ignorar entradas vacías y la entrada especial "."
      if (directory[i].inode == 0xFFFF || strcmp(directory[i].file_name, ".") == 0)
      {
         continue;
      }

      // Obtener el inodo correspondiente
      inode = &inodes->inodes[directory[i].inode];

      // Imprimir nombre del fichero, tamaño e inodo
      printf("%-20s tamanio:%-6u inodo:%-2d bloques:",
             directory[i].file_name, // Nombre del fichero
             inode->file_size,       // Tamaño del fichero
             directory[i].inode);    // Número de inodo

      // Imprimir bloques ocupados
      for (j = 0; j < MAX_INODE_BLOCK_NUMS; j++)
      {
         if (inode->block_numbers[j] != 0xFFFF) // Ignorar bloques no asignados
         {
            printf(" %u", inode->block_numbers[j]);
         }
      }

      printf("\n");
   }
}