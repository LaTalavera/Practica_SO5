# Filesystem Simulator

- [Introduction](#introduction)
- [Features](#features)
- [Filesystem Structure](#filesystem-structure)
  - [Superblock](#superblock)
  - [Byte Maps](#byte-maps)
  - [Inodes](#inodes)
  - [Directory Entries](#directory-entries)
  - [Data Blocks](#data-blocks)
- [Internal Mechanics](#internal-mechanics)
  - [Initialization](#initialization)
  - [Command Handling](#command-handling)
  - [File Operations](#file-operations)
    - [Listing Directory (`dir`)](#listing-directory-dir)
    - [Displaying Superblock Information (`info`)](#displaying-superblock-information-info)
    - [Displaying Byte Maps (`bytemaps`)](#displaying-byte-maps-bytemaps)
    - [Renaming Files (`rename`)](#renaming-files-rename)
    - [Printing File Contents (`print`)](#printing-file-contents-print)
    - [Deleting Files (`remove`)](#deleting-files-remove)
    - [Copying Files (`copy`)](#copying-files-copy)
    - [Clearing the Terminal (`clear`)](#clearing-the-terminal-clear)
    - [Debugging (`debug`)](#debugging-debug)
- [Installation](#installation)
- [Usage](#usage)
  - [Running the Program](#running-the-program)
  - [Available Commands](#available-commands)
- [Example Session](#example-session)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Introduction

The **Filesystem Simulator** is a command-line application developed in C that emulates basic filesystem operations. It allows users to perform file management tasks such as listing directory contents, renaming files, copying files, deleting files, and more within a simulated environment. This project is designed for educational purposes, providing insights into filesystem structures and operations.

## Features

- **Directory Listing (`dir`):** View all files in the directory along with their sizes, inodes, and allocated blocks.
- **Superblock Information (`info`):** Display detailed information about the filesystem's superblock.
- **Byte Maps Display (`bytemaps`):** Show the status of inodes and data blocks.
- **File Creating (`create`):** Create a new file with specified content in the filesystem.
- **File Renaming (`rename`):** Rename existing files.
- **File Printing (`print`):** Display the contents of a specified file.
- **File Deletion (`remove`):** Delete files from the filesystem.
- **File Copying (`copy`):** Duplicate files within the filesystem.
- **Terminal Clearing (`clear`):** Clear the terminal screen for better readability.
- **Debugging (`debug`):** List all directory entries, useful for debugging purposes.
- **Help (`help`):** Display a list of all available commands and their descriptions.
- **Exit (`exit`):** Save all changes and gracefully exit the program.

## Filesystem Structure

The simulated filesystem is organized into fixed-size blocks, each being 512 bytes. Understanding the filesystem structure is crucial to comprehending how the simulator manages files and directories.

### Superblock

- **Location:** Block `0` (First 512 bytes of `particion.bin`)
- **Purpose:** Holds metadata about the filesystem.
- **Contents:**
  - `total_inodes`: Total number of inodes available in the filesystem.
  - `total_blocks`: Total number of blocks in the filesystem.
  - `free_blocks`: Number of free (unallocated) blocks.
  - `free_inodes`: Number of free (unallocated) inodes.
  - `first_data_block`: The starting block number where data blocks begin.
  - `block_size`: Size of each block in bytes (512 bytes).
  - `padding`: Reserved space to ensure the superblock occupies exactly one block.

### Byte Maps

- **Location:** Block `1` (Second 512 bytes of `particion.bin`)
- **Purpose:** Manages allocation status of inodes and data blocks using bitmaps.
- **Contents:**
  - `block_bytemap`: An array indicating the allocation status of each block (`1` for occupied, `0` for free).
  - `inode_bytemap`: An array indicating the allocation status of each inode (`1` for occupied, `0` for free).
  - `padding`: Reserved space to ensure the byte maps occupy exactly one block.

### Inodes

- **Location:** Block `2` (Third 512 bytes of `particion.bin`)
- **Purpose:** Represents individual files, storing metadata and block allocations.
- **Structure (`EXT_SIMPLE_INODE`):**
  - `file_size`: Size of the file in bytes.
  - `block_numbers`: Array holding block numbers where the file's data is stored (`NULL_BLOCK` indicates no block).
- **Inode Block (`EXT_INODE_BLOCK`):**
  - Contains an array of `EXT_SIMPLE_INODE` structures.
  - `padding`: Reserved space to ensure the inode block occupies exactly one block.

### Directory Entries

- **Location:** Block `3` (Fourth 512 bytes of `particion.bin`)
- **Purpose:** Maps filenames to their corresponding inodes.
- **Structure (`EXT_DIRECTORY_ENTRY`):**
  - `file_name`: Name of the file (up to 16 characters plus null terminator).
  - `inode`: Inode number associated with the file (`NULL_INODE` indicates an unused entry).

### Data Blocks

- **Location:** Blocks `4` to `99` (Bytes `2048` to `51199` of `particion.bin`)
- **Purpose:** Store the actual content of files.
- **Structure (`EXT_DATA`):**
  - `data`: Array of bytes representing the file's data (512 bytes per block).

## Internal Mechanics

Understanding how the filesystem simulator manipulates these structures internally is essential for grasping its functionality.

### Initialization

1. **Loading the Filesystem:**

   - Upon starting, the program opens `particion.bin` in read-write binary mode.
   - It reads all blocks into an in-memory array (`fileData`), ensuring quick access and manipulation.

2. **Parsing Structures:**

   - **Superblock:** Extracted from `fileData[0]`.
   - **Byte Maps:** Extracted from `fileData[1]`.
   - **Inode Block:** Extracted from `fileData[2]`.
   - **Directory Entries:** Extracted from `fileData[3]`.
   - **Data Blocks:** Extracted from `fileData[4]` onwards.

3. **In-Memory Representation:**
   - All structures are copied into dedicated in-memory variables for efficient access and modification during program execution.

### Command Handling

The program operates in a loop, continuously prompting the user for commands. It parses and executes commands based on user input, interacting with the in-memory filesystem structures and updating `particion.bin` accordingly.

### File Operations

Each file operation manipulates the filesystem structures to reflect changes and ensures data consistency by saving updated structures back to `particion.bin`.

#### Listing Directory (`dir`)

- **Function:** `ListDirectory`
- **Logic:**
  - Iterates through all directory entries.
  - For each occupied entry, retrieves the corresponding inode.
  - Displays file name, size, inode number, and allocated data blocks.

#### Displaying Superblock Information (`info`)

- **Function:** `PrintSuperBlock`
- **Logic:**
  - Reads and displays metadata from the superblock, such as total inodes, total blocks, free inodes, free blocks, etc.

#### Displaying Byte Maps (`bytemaps`)

- **Function:** `PrintByteMaps`
- **Logic:**
  - Displays the allocation status of inodes and blocks.
  - Helps in understanding which inodes and blocks are occupied or free.

#### Creating Files (`create`)

- **Function:** `CreateFile`
- **Logic:**
  - Verifies that the file does not already exist in the directory.
  - Allocates a free inode and initializes its metadata.
  - Divides the content into blocks and allocates free blocks to store the data.
  - Updates the directory with a new entry for the created file.
  - Saves all updated structures to ensure persistence.

#### Renaming Files (`rename`)

- **Function:** `RenameFile`
- **Logic:**
  - Locates the directory entry for the specified file.
  - Ensures no existing file has the new name to avoid conflicts.
  - Updates the `file_name` in the directory entry.
  - Saves the updated directory and inode structures back to `particion.bin`.

#### Printing File Contents (`print`)

- **Function:** `PrintFile`
- **Logic:**
  - Finds the directory entry for the specified file.
  - Retrieves the associated inode to determine file size and allocated blocks.
  - Iterates through allocated data blocks, reading content from the in-memory `data` array.
  - Concatenates and displays the file's content.

#### Deleting Files (`remove`)

- **Function:** `DeleteFile`
- **Logic:**
  - Locates the directory entry for the specified file.
  - Frees allocated data blocks by updating the block bytemap.
  - Frees the inode by updating the inode bytemap and resetting inode data.
  - Removes the directory entry by setting its inode to `NULL_INODE` and clearing the filename.
  - Updates the superblock to reflect the increased count of free inodes and blocks.
  - Saves all modified structures back to `particion.bin`.

#### Copying Files (`copy`)

- **Function:** `CopyFile`
- **Logic:**
  - Validates input parameters and checks if the destination file already exists.
  - Finds the source file's directory entry and associated inode.
  - Allocates a new inode for the destination file.
  - Allocates new data blocks for the destination file, copying data from the source's in-memory `data` array.
  - Updates byte maps to mark new inodes and blocks as occupied.
  - Creates a new directory entry for the copied file.
  - Saves all modified structures back to `particion.bin`.

#### Clearing the Terminal (`clear`)

- **Function:** `ClearScreen`
- **Logic:**
  - Uses the `system()` function to call the appropriate terminal command (`clear` for Unix-like systems or `cls` for Windows) to clear the terminal screen.

#### Debugging (`debug`)

- **Function:** `DebugListAllDirectoryEntries`
- **Logic:**
  - Iterates through all directory entries.
  - Displays whether each entry is occupied or free, along with file names and inode numbers for occupied entries.
  - Useful for verifying the internal state of the filesystem during development and debugging.

### Saving and Loading Structures

To maintain data integrity and ensure that changes persist across program executions, the simulator employs a set of `Save...` functions that write in-memory structures back to `particion.bin`.

#### SaveSuperBlock

- **Function:** `SaveSuperBlock`
- **Logic:**
  - Seeks to block `0` in `particion.bin`.
  - Writes the `superBlock` structure to the superblock area.
  - Flushes the output to ensure data is written immediately.

#### SaveByteMaps

- **Function:** `SaveByteMaps`
- **Logic:**
  - Seeks to block `1` in `particion.bin`.
  - Writes the `byteMaps` structure to the byte maps area.
  - Flushes the output to ensure data is written immediately.

#### SaveInodesAndDirectory

- **Function:** `SaveInodesAndDirectory`
- **Logic:**
  - Seeks to block `2` in `particion.bin`.
  - Writes the `inodeBlock` structure to the inode area.
  - Seeks to block `3` in `particion.bin`.
  - Writes the `directory` array to the directory area.
  - Flushes the output to ensure data is written immediately.

#### SaveData

- **Function:** `SaveData`
- **Logic:**
  - Seeks to block `4` (defined by `FIRST_DATA_BLOCK`) in `particion.bin`.
  - Writes the `data` array (containing all data blocks) to the data area.
  - Flushes the output to ensure data is written immediately.

#### Data Consistency

- **In-Memory vs. Disk:**
  - All operations are performed on in-memory structures for efficiency.
  - After any modification (e.g., copying a file), the corresponding `Save...` functions are called to persist changes to disk.
  - This ensures that the in-memory state and disk state remain synchronized.

- **Block-Based Offsets:**
  - All `Save...` functions calculate file offsets based on block numbers (`BLOCK_SIZE * block_number`) to maintain proper alignment within `particion.bin`.
  - This prevents data corruption and ensures that each structure occupies its designated block.

## Installation

### Prerequisites

- **C Compiler:** Ensure you have `gcc` or any compatible C compiler installed.

### Steps

```bash
git clone https://github.com/LaTalavera/Practica_SO5.git
cd Practica_SO5
gcc -o filesystem filesystem.c
./filesystem
```
### Available Commands

- **`dir`**: List all files in the directory.
- **`info`**: Display superblock information.
- **`bytemaps`**: Show inode and block byte maps.
- **`rename <old_name> <new_name>`**: Rename a file.
- **`print <file_name>`**: Display the contents of a file.
- **`remove <file_name>`**: Delete a file.
- **`copy <source_name> <dest_name>`**: Copy a file.
- **`clear`**: Clear the terminal screen.
- **`debug`**: List all directory entries for debugging.
- **`help`**: Show available commands.
- **`exit`**: Save changes and exit the program.

## Continuous Integration

The project utilizes GitHub Actions to automate the build, static analysis, and testing processes. This ensures code quality and reliability by automatically verifying that new changes do not introduce regressions or issues.

### GitHub Actions Workflow

The CI pipeline is defined in build.yml and consists of three primary jobs:

#### Build the Application

Environment: Ubuntu latest
Steps:
Checkout the code.
Install GCC and build-essential packages.
Compile the filesystem.c source file into an executable named filesystem.
Upload the compiled executable as a build artifact.

#### Run Static Analysis

Environment: Ubuntu latest
Dependencies: Depends on the successful completion of the build job.
Steps:
Checkout the code.
Install cppcheck for static code analysis.
Run cppcheck on headers.h and filesystem.c with comprehensive analysis flags.
Fail the job if any errors are detected.

#### Run Unit Tests

Environment: Ubuntu latest
Dependencies: Depends on the successful completion of the build job.
Steps:
Checkout the code.
Install GCC and Unity testing framework.
Compile the unit tests located in the tests directory using gcc, linking against filesystem.c and unity.c.
Execute the compiled unit tests.
Upload the test results as an artifact.


## Unit Testing
### Testing Framework
The project employs the Unity testing framework, a lightweight and straightforward framework for C, to facilitate unit testing. Unity provides a simple API for writing and running tests, ensuring that individual components of the filesystem simulator function as expected.

Running Unit Tests
To run the unit tests locally, follow these steps:

Navigate to the Project Directory
```bash
cd Practica_SO5
```
Compile the Unit Tests

Ensure that you have GCC installed. Then, compile the tests using the following command:
```bash
gcc -Itests -I. -DTEST -o test_filesystem tests/test_filesystem.c filesystem.c tests/unity.c -Wall -Werror
```
**Flags Explained:**

- ``-Itests -I.``: Include directories for header files.
- ``-DTEST``: Define the `TEST` macro, enabling test-specific code paths.
- ``-o test_filesystem``: Output executable named `test_filesystem`.
- ``-Wall -Werror``: Enable all warnings and treat them as errors for stricter code quality.

### Execute the Unit Tests

Run the compiled test executable:
```bash
./test_filesystem
```