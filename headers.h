#define BLOCK_SIZE 512
#define MAX_INODES 24
#define MAX_FILES 20
#define MAX_DATA_BLOCKS 96
#define FIRST_DATA_BLOCK 4
#define MAX_PARTITION_BLOCKS (MAX_DATA_BLOCKS + FIRST_DATA_BLOCK)
  // superblock + inode and block bytemaps + inodes + directory
#define MAX_INODE_BLOCK_NUMS 7
#define FILE_NAME_LENGTH 17
#define NULL_INODE 0xFFFF
#define NULL_BLOCK 0xFFFF

/* Superblock structure */
typedef struct {
  unsigned int total_inodes;          /* total inodes in the partition */
  unsigned int total_blocks;          /* total blocks in the partition */
  unsigned int free_blocks;           /* free blocks */
  unsigned int free_inodes;           /* free inodes */
  unsigned int first_data_block;      /* first data block */
  unsigned int block_size;            /* block size in bytes */
  unsigned char padding[BLOCK_SIZE - 6 * sizeof(unsigned int)]; /* padding with 0's */
} EXT_SIMPLE_SUPERBLOCK;

/* Bytemaps, fit in one block */
typedef struct {
  unsigned char block_bytemap[MAX_PARTITION_BLOCKS];
  unsigned char inode_bytemap[MAX_INODES];  /* inodes 0 and 1 reserved, inode 2 directory */
  unsigned char padding[BLOCK_SIZE - (MAX_PARTITION_BLOCKS + MAX_INODES) * sizeof(char)];
} EXT_BYTE_MAPS;

/* Inode */
typedef struct {
  unsigned int file_size;
  unsigned short int block_numbers[MAX_INODE_BLOCK_NUMS];
} EXT_SIMPLE_INODE;

/* List of inodes, fit in one block */
typedef struct {
  EXT_SIMPLE_INODE inodes[MAX_INODES];
  unsigned char padding[BLOCK_SIZE - MAX_INODES * sizeof(EXT_SIMPLE_INODE)];
} EXT_INODE_BLOCK;

/* Individual directory entry */
typedef struct {
  char file_name[FILE_NAME_LENGTH];
  unsigned short int inode;
} EXT_DIRECTORY_ENTRY;

/* Data block */
typedef struct {
  unsigned char data[BLOCK_SIZE]; 	
} EXT_DATA;