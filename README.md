File-System

# File-System

Shell.cpp, Shell.h: It will process command from the command line

FileSys.cpp, FileSys.h: Provides the interface for file system command
- ls: list all content in a directory
- mkdir : create a subdirectory in the current directory
- cd : Change to specified directory. The directory must be a subdirectory in the current directory. (No paths or ".." are allowed.)
- home: Switch to the home directory.
- rmdir: Removes a subdirectory. The subdirectory must be empty.
- create : Creates an empty file of the filename in the current directory. An empty file consists of an inode and no data blocks.
- append : Appends the data to the file. Data should be appended in a manner to first fill the last data block as much as possible and then allocating new block(s) ONLY if more space is needed.
- stat : Displays stats for the given file or directory.
- cat : Display the contents of the file to the screen. Print a newline when completed.
- tail : Display the last N bytes of the file to the screen. Print a newline when completed. (If N >= file size, print the whole file just as with the cat command.) For example, if foo.txt contains “abcdef” then the command “tail foo.txt 3” would output “def”.
- rm : Remove a file from the directory, reclaim all of its blocks including its inode. Cannot remove directories.

BasicFileSys.cpp, BasicFileSys.h: A low-level interface that interacts with the disk.

Disk.cpp, Disk.h: Represents a "virtual" disk that is contained within a file.

Block.h: Four types of blocks used in the file system

- Superblock: There is only one superblock on the disk and that is always block 0. It
 contains a bitmap on what disk blocks are free. (The superblock is used by the Basic File
 System to implement get_free_block() and reclaim_block() - you shouldn't have to
 touch it, but be careful not to corrupt it by writing to it by mistake.)

- Directories: Represents a directory. The first field is a magic number which is used to
 distinguish between directories and inodes. The second field stores the number of files
 located in the directory. The remaining space is used to store the file entries. Each entry
 consists of a name and a block number (the directory block for directories and the inode
 block for data files). Unused entries are indicated by having a block number of 0 (we
 know that’s not a valid block because 0 is reserved for the superblock). Block 1 always
 contains the directory for the "home" directory.

- Inodes: Represents an index block for a data file. In this assignment, only direct index
 pointers are used. The first field is a magic number which is used to distinguish between
 directories and inodes. The second field is the size of the file (in bytes). The remaining
 space consists of an array of indices to data blocks of the file. Use 0 to represent unused
 pointer entries (note that files cannot access the superblock).

- Data blocks: Blocks currently used to store data in files.
Each of the four layers is implemented using a class. The class contains ("has-a") a single instance of the lower layer.
For instance, the file system class has an instance of the basic file system.

When appending to the file, it cannot include any space, or null termination characters

The user can quit the program when it's done.

A Makefile is provided, the it's needed to clear the disk, do make clean

To run the program: ./filesys

Run it with script: ./filesys -s

