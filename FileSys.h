// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#ifndef FILESYS_H
#define FILESYS_H

#include "BasicFileSys.h"

class FileSys
{

public:
  // mounts the file system
  void mount();

  // unmounts the file system
  void unmount();

  // make a directory
  void mkdir(const char *name);

  // switch to a directory
  void cd(const char *name);

  // switch to home directory
  void home();

  // remove a directory
  void rmdir(const char *name);

  // list the contents of current directory
  void ls();

  // create an empty data file
  void create(const char *name);

  // append data to a data file
  void append(const char *name, const char *data);

  // display the contents of a data file
  void cat(const char *name);

  // display the last N bytes of the file
  void tail(const char *name, unsigned int n);

  // delete a data file
  void rm(const char *name);

  // display stats about file or directory
  void stat(const char *name);

private:
  BasicFileSys bfs; // basic file system
  short curr_dir;   // current directory
  bool is_directory(short blockNum);
  void assignName_Block(int length, struct dirblock_t &dir_block, const char *name, short block);
  void erase(char *target);
  void shift_blocks(int pos, int size, struct dirblock_t &dir_block);
  // Additional private variables and Helper functions - if desired
};

#endif
