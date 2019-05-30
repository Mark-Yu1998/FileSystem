// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount()
{
  bfs.mount();
  curr_dir = 1;
}

// unmounts the file system
void FileSys::unmount()
{
  bfs.unmount();
}

// make a directory
void FileSys::mkdir(const char *name)
{

  //Read in the current directory
  struct dirblock_t dir_block;
  bfs.read_block(curr_dir, (void *)&dir_block);

  //Check if directory is full
  if (dir_block.num_entries >= MAX_DIR_ENTRIES)
  {
    cout << "Directory is full" << endl;
    return;
  }

  //Check if directory already exist
  for (int i = 0; i < dir_block.num_entries; i++)
  {
    if (!strcmp(name, dir_block.dir_entries[i].name))
    {
      cout << "File exists" << endl;
      return;
    }
  }
  int length = strlen(name);

  //Check if file name is too long
  if (length > MAX_FNAME_SIZE)
  {
    cout << "File name is too long" << endl;
    return;
  }
  //Get free block
  short block = bfs.get_free_block();

  //Check if disk is full
  if (!block)
  {
    cout << "Disk is full" << endl;
    return;
  }

  //create new directory
  struct dirblock_t newDir;
  newDir.magic = DIR_MAGIC_NUM;
  newDir.num_entries = 0;

  unsigned int size = dir_block.num_entries;

  assignName_Block(length, dir_block, name, block);

  bfs.write_block(block, (void *)&newDir);
  dir_block.num_entries++;
  bfs.write_block(curr_dir, (void *)&dir_block);
}

// switch to a directory
void FileSys::cd(const char *name)
{
  struct dirblock_t current;
  bfs.read_block(curr_dir, (void *)&current);

  bool exist = false;
  //Targeted block number
  short tgt_block = -1;

  bool is_dir = false;
  for (int i = 0; i < current.num_entries; i++)
  {
    if (!strcmp(current.dir_entries[i].name, name))
    {
      exist = true;
      tgt_block = current.dir_entries[i].block_num;
      //exit the loop immediately once found
      i = current.num_entries;
    }
  }
  is_dir = is_directory(tgt_block);

  if (!exist)
  {
    cout << "File does not exist" << endl;
    return;
  }

  if (!is_dir)
  {
    cout << "File is not a directory" << endl;
    return;
  }

  curr_dir = tgt_block;
}

// switch to home directory
void FileSys ::home()
{
  curr_dir = 1;
}

// remove a directory
void FileSys::rmdir(const char *name)
{
  //Read in the current directory
  struct dirblock_t dir_block;
  bfs.read_block(curr_dir, (void *)&dir_block);

  short tgr_block = 0;
  int pos = 0;
  for (int i = 0; i < dir_block.num_entries; i++)
  {
    short block = dir_block.dir_entries[i].block_num;
    if (is_directory(block) && !strcmp(dir_block.dir_entries[i].name, name))
    {
      tgr_block = block;
      pos = i;
      i = MAX_DIR_ENTRIES;
    }
  }
  bool is_dir = is_directory(tgr_block);
  if (!tgr_block)
  {
    cout << "File does not exist" << endl;
    return;
  }

  if (!is_dir)
  {
    cout << "File is not a directory" << endl;
    return;
  }
  struct dirblock_t tgr_dir;

  //read the directory going to delete
  bfs.read_block(tgr_block, (void *)&tgr_dir);

  //if directory is not empty
  if (tgr_dir.num_entries)
  {
    cout << "Directory is not Empty" << endl;
    return;
  }

  int size = dir_block.num_entries;

  //earse the name of the deleted directory
  erase(dir_block.dir_entries[pos].name);

  //shift everything to replace the deleted one
  shift_blocks(pos, size, dir_block);

  //write back
  bfs.write_block(tgr_block, (void *)&tgr_dir);
  bfs.reclaim_block(tgr_block);
  dir_block.num_entries--;
  bfs.write_block(curr_dir, (void *)&dir_block);
}

// list the contents of current directory
void FileSys::ls()
{
  struct dirblock_t current;
  bfs.read_block(curr_dir, (void *)&current);

  //if the directory is empty
  if (!current.num_entries)
  {
    return;
  }

  for (int i = 0; i < current.num_entries; ++i)
  {

    short block = current.dir_entries[i].block_num;

    int length = strlen(current.dir_entries[i].name);
    cout << current.dir_entries[i].name;
    if (is_directory(block))
    {
      cout << "/";
    }

    cout << "   ";
  }
  cout << endl;
}

// create an empty data file
void FileSys::create(const char *name)
{
  struct dirblock_t current;
  bfs.read_block(curr_dir, (void *)&current);

  if (current.num_entries >= MAX_DIR_ENTRIES)
  {
    cout << "Directory is Full" << endl;
    return;
  }

  int length = strlen(name);

  if (length > MAX_FNAME_SIZE)
  {
    cout << "File name too long!" << endl;
    return;
  }

  for (int i = 0; i < current.num_entries; i++)
  {
    if (!strcmp(name, current.dir_entries[i].name))
    {
      cout << "File exists" << endl;
      return;
    }
  }

  struct inode_t newFile;
  newFile.magic = INODE_MAGIC_NUM;
  newFile.size = 0;

  short block = bfs.get_free_block();

  if (!block)
  {
    cout << "Disk is Full" << endl;
    return;
  }

  unsigned int size = current.num_entries;

  for (int i = 0; i < MAX_DATA_BLOCKS; i++)
  {
    newFile.blocks[i] = 0;
  }

  assignName_Block(length, current, name, block);

  bfs.write_block(block, (void *)&newFile);
  current.num_entries++;
  bfs.write_block(curr_dir, (void *)&current);
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
  struct dirblock_t dirBlock;
  bfs.read_block(curr_dir, (void *)&dirBlock);
  int pos = -1;
  unsigned int size = dirBlock.num_entries;

  for (int i = 0; i < size; i++)
  {
    if (!strcmp(dirBlock.dir_entries[i].name, name))
    {
      pos = i;
      i = size;
    }
  }

  if (pos == -1)
  {
    cout << "File Does Not Exist" << endl;
    return;
  }

  short tgrBlock = dirBlock.dir_entries[pos].block_num;
  bool isDir = is_directory(tgrBlock);

  if (isDir)
  {
    cout << "File is a directory" << endl;
    return;
  }

  struct inode_t file;
  bfs.read_block(tgrBlock, &file);

  if (file.size >= MAX_DATA_BLOCKS * BLOCK_SIZE)
  {
    cout << "File is Full" << endl;
    return;
  }

  int dataSize = strlen(data);

  if (file.size + dataSize > MAX_FILE_SIZE)
  {
    cout << "Append exceeds maximum file size" << endl;
    return;
  }

  int blockNeeded = dataSize / BLOCK_SIZE;
  int location = file.size / BLOCK_SIZE;
  int start = file.size % BLOCK_SIZE;
  int dataPtr = 0;

  struct datablock_t data_block;
  //If all the old blocks are filled
  if (!start)
  {
    //Put it in a new block
    short block = bfs.get_free_block();
    file.blocks[location] = block;
  }
  else
  {
    bfs.read_block(file.blocks[location], &data_block);
  }

  for (int i = start; i < BLOCK_SIZE && dataPtr < dataSize; i++)
  {
    data_block.data[i] = data[dataPtr];
    dataPtr++;
    file.size++;
  }

  bfs.write_block(file.blocks[location], &data_block);

  while (dataPtr < dataSize)
  {
    short block = bfs.get_free_block();
    location++;
    file.blocks[location] = block;
    for (int i = 0; i < BLOCK_SIZE && dataPtr < dataSize; i++)
    {
      data_block.data[i] = data[dataPtr];
      file.size++;
      dataPtr++;
    }
    bfs.write_block(file.blocks[location], &data_block);
  }

  bfs.write_block(dirBlock.dir_entries[pos].block_num, &file);
  bfs.write_block(curr_dir, &dirBlock);
}

// display the contents of a data file
void FileSys::cat(const char *name)
{
  struct dirblock_t dirBlock;
  bfs.read_block(curr_dir, &dirBlock);

  int pos = -1;
  for (int i = 0; i < dirBlock.num_entries; i++)
  {
    if (!strcmp(dirBlock.dir_entries[i].name, name))
    {
      pos = i;
      i = MAX_DIR_ENTRIES;
    }
  }

  if (pos == -1)
  {
    cout << "File does not exist" << endl;
    return;
  }

  bool isDir = is_directory(dirBlock.dir_entries[pos].block_num);

  if (isDir)
  {
    cout << "File is a directory" << endl;
    return;
  }
  struct inode_t file;
  bfs.read_block(dirBlock.dir_entries[pos].block_num, &file);

  int blocksToRead = file.size / BLOCK_SIZE + (file.size % BLOCK_SIZE ? 1 : 0);
  int count = 0;
  int index = 0;
  struct datablock_t datablock;
  while (index < blocksToRead)
  {
    bfs.read_block(file.blocks[index], &datablock);

    for (int i = 0; i < BLOCK_SIZE && count < file.size; i++)
    {
      cout << datablock.data[i];
      count++;
    }
    index++;
  }
  cout << endl;
}

// display the last N bytes of the file
void FileSys::tail(const char *name, unsigned int n)
{
  struct dirblock_t dirblock;
  bfs.read_block(curr_dir, &dirblock);

  int pos = -1;
  for (int i = 0; i < dirblock.num_entries; i++)
  {
    if (!strcmp(dirblock.dir_entries[i].name, name))
    {
      pos = i;
      i = MAX_DIR_ENTRIES;
    }
  }

  if (pos == -1)
  {
    cout << "File does not exist" << endl;
    return;
  }

  if (is_directory(dirblock.dir_entries[pos].block_num))
  {
    cout << "File is a directory" << endl;
    return;
  }

  struct inode_t file;
  bfs.read_block(dirblock.dir_entries[pos].block_num, &file);

  if (file.size <= n)
  {
    cat(name);
    return;
  }

  int indexBlock = (file.size - n) / BLOCK_SIZE;
  int start = (file.size - n) % BLOCK_SIZE;
  int count = 0;
  struct datablock_t datablock;
  bfs.read_block(file.blocks[indexBlock], &datablock);

  for (int i = start; i < BLOCK_SIZE && count < n; i++)
  {
    cout << datablock.data[i];
    count++;
  }

  while (count < n)
  {
    indexBlock++;
    bfs.read_block(file.blocks[indexBlock], &datablock);
    for (int i = 0; i < BLOCK_SIZE && count < n; i++)
    {
      cout << datablock.data[i];
      count++;
    }
  }
  cout << endl;
}

// delete a data file
void FileSys::rm(const char *name)
{
  struct dirblock_t dir_block;
  bfs.read_block(curr_dir, (void *)&dir_block);

  int size = dir_block.num_entries;
  int pos = 0;
  bool found = false;
  for (int i = 0; i < size; i++)
  {
    if (!strcmp(name, dir_block.dir_entries[i].name))
    {
      found = true;
      pos = i;
      i = size;
    }
  }

  if (!found)
  {
    cout << "File Does Not Exist" << endl;
    return;
  }
  int block = dir_block.dir_entries[pos].block_num;

  if (is_directory(block))
  {
    cout << "File is a directory" << endl;
    return;
  }

  //delete i-node
  struct inode_t file;
  bfs.read_block(block, (void *)&file);

  int length = strlen(name);
  int blockToDelete = file.size / BLOCK_SIZE + (file.size % BLOCK_SIZE ? 1 : 0);

  for (int i = 0; i < blockToDelete; i++)
  {
    //reclaim all the datablock
    bfs.reclaim_block(file.blocks[i]);
  }

  erase(dir_block.dir_entries[pos].name);

  shift_blocks(pos, size, dir_block);
  file.size = 0;

  bfs.write_block(block, (void *)&file);
  bfs.reclaim_block(block);
  dir_block.num_entries--;
  bfs.write_block(curr_dir, (void *)&dir_block);
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
  struct dirblock_t dirblock;
  bfs.read_block(curr_dir, &dirblock);

  int pos = -1;
  for (int i = 0; i < dirblock.num_entries; i++)
  {
    if (!strcmp(dirblock.dir_entries[i].name, name))
    {
      pos = i;
      //Immediately exit once the file is found
      i = MAX_DIR_ENTRIES;
    }
  }

  if (pos == -1)
  {
    cout << "File does not exisit" << endl;
    return;
  }

  bool isDir = is_directory(dirblock.dir_entries[pos].block_num);

  if (isDir)
  {
    cout << "Directory name: " << name << "/" << endl;
    cout << "Directory Block: " << dirblock.dir_entries[pos].block_num << endl;
    return;
  }
  else
  {
    cout << "Inode block: " << dirblock.dir_entries[pos].block_num << endl;
    struct inode_t file;
    bfs.read_block(dirblock.dir_entries[pos].block_num, &file);
    cout << "Bytes in file: " << file.size << endl;
    cout << "Number of blocks: " << (file.size / BLOCK_SIZE + (file.size % BLOCK_SIZE ? 1 : 0)) << endl;
    cout << "First block: " << file.blocks[0] << endl;
    return;
  }
}

// HELPER FUNCTIONS (optional)

//Verify if a the block is a directory
bool FileSys ::is_directory(short blockNum)
{

  if (blockNum == -1)
  {
    return false;
  }
  struct dirblock_t current;
  bfs.read_block(blockNum, (void *)&current);

  return current.magic == DIR_MAGIC_NUM;
}

//assign name to the blcoks
void FileSys ::assignName_Block(int length, struct dirblock_t &dir_block, const char *name, short block)
{

  unsigned int index = dir_block.num_entries;
  erase(dir_block.dir_entries[index].name);
  strncpy(dir_block.dir_entries[index].name, name, length);
  dir_block.dir_entries[index].block_num = block;
  dir_block.dir_entries[index].name[length] = '\0';
}

void FileSys ::erase(char *target)
{
  char replace[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  strncpy(target, replace, MAX_FNAME_SIZE);
}

//Shift blocks one to the left
void FileSys ::shift_blocks(int pos, int size, struct dirblock_t &dir_block)
{
  for (int i = pos; i < size - 1; i++)
  {
    dir_block.dir_entries[i].block_num = dir_block.dir_entries[i + 1].block_num;
    erase(dir_block.dir_entries[i].name);
    strncpy(dir_block.dir_entries[i].name, dir_block.dir_entries[i + 1].name,
            strlen(dir_block.dir_entries[i + 1].name));
  }
}