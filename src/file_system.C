/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define INODE_LIST_BLOCK 0
#define FREE_LIST_BLOCK  1

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem()
{
    Console::puts("In file system constructor.\n");

    inodes = new Inode[MAX_INODES];
    free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];

} // FileSystem::FileSystem


FileSystem::~FileSystem()
{
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */

    writeBlockToDisk(INODE_LIST_BLOCK, (unsigned char *)inodes);
    writeBlockToDisk(FREE_LIST_BLOCK, free_blocks);

    delete [] inodes;
    delete [] free_blocks;

} // FileSystem::~FileSystem


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

short FileSystem::GetFreeInode()
{
    for (int i = 0; i < MAX_INODES; i++)
    {
        if (inodes[i].isAllocated == false)
        {
            inodes[i].isAllocated = true;
            return i;
        }
    }

    return -1;

} // FileSystem::GetFreeInode


int FileSystem::GetFreeBlock()
{
    for (int i = 0; i < SimpleDisk::BLOCK_SIZE; i++)
    {
        if (free_blocks[i] == 'f')
        {
            free_blocks[i] = 'u';
            return i;
        }
    }

    return -1;

} // FileSystem::GetFreeBlock


bool FileSystem::Mount(SimpleDisk * _disk)
{
    Console::puts("mounting file system from disk\n");
    /* Here you read the inode list and the free list into memory */

    this->disk = _disk;

    if (disk == nullptr)
    {
        Console::puts("Failed to find disk! Cannot mount file system.\n");
        return false;
    }

    unsigned char *tempBuf;
    disk->read(INODE_LIST_BLOCK, tempBuf);
    inodes = (Inode *)tempBuf;

    disk->read(FREE_LIST_BLOCK, free_blocks);

    // If the Inode Block or the Free Block is marked free, then the File System is
    // not present or is corrupted. In either case, we cannot proceed.
    if (free_blocks[INODE_LIST_BLOCK] == 'f' || free_blocks[FREE_LIST_BLOCK] == 'f')
    {
        Console::puts("File System not present in disk! Failed to mount disk.\n");
        return false;
    }

    return true;

} // FileSystem::Mount


bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) // static!
{
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */


    // Initilaize and store empty the 'Inode' list.
    Inode *inodeBuf;
    inodeBuf = new Inode[MAX_INODES];
    for (int i = 0; i < MAX_INODES; i++)
    {
        inodeBuf[i] = {};
    }

    _disk->write(INODE_LIST_BLOCK, (unsigned char *)inodeBuf);

    // reclaim heap memory.
    delete [] inodeBuf;

    // Initialize and store the free block bitmap.
    unsigned char *freeBlockBuf;
    freeBlockBuf = new unsigned char[SimpleDisk::BLOCK_SIZE];

    // First two blocks are used.
    freeBlockBuf[INODE_LIST_BLOCK] = 'u';
    freeBlockBuf[FREE_LIST_BLOCK] = 'u';

    for (int i = 2; i < SimpleDisk::BLOCK_SIZE; i++)
    {
        // Mark free blocks.
        freeBlockBuf[i] = 'f';
    }

    _disk->write(FREE_LIST_BLOCK, freeBlockBuf);

    // reclaim heap memory.
    delete [] freeBlockBuf;

    return true;

} // FileSystem::Format


Inode * FileSystem::LookupFile(int _file_id)
{
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */

    for (int i = 0; i < MAX_INODES; i++)
    {
        if (inodes[i].id == _file_id)
        {
            return &inodes[i];
        }
    }

    return nullptr;

} // FileSystem::LookupFile


bool FileSystem::CreateFile(int _file_id)
{
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */

    // First check whether file already exists.
    if (LookupFile(_file_id))
    {
        Console::puts("File already exists!\n");
        return false;
    }

    // Get a block to hold the indices which in contain the
    // data blocks to store a file.
    int dataIndexBlock = GetFreeBlock();
    if (dataIndexBlock == -1)
    {
        Console::puts("Out of Free Blocks! Cannot create file...\n");
        return false;
    }

    int inodeIndex = GetFreeInode();
    if (inodeIndex == -1)
    {
        Console::puts("Out of inodes! Cannot create file...\n");
        return false;
    }

    inodes[inodeIndex].fs = this;
    inodes[inodeIndex].id = _file_id;
    inodes[inodeIndex].blockNo = dataIndexBlock;
    inodes[inodeIndex].size = 0;

    disk->write(INODE_LIST_BLOCK, (unsigned char *)inodes);
    disk->write(FREE_LIST_BLOCK, free_blocks);

    // Store the indices of all the data blocks used to store a file.
    unsigned char *dataIndirectBuffer;
    dataIndirectBuffer = new unsigned char[SimpleDisk::BLOCK_SIZE];
    for (int i = 0; i < SimpleDisk::BLOCK_SIZE; i++)
    {
        // Initially all indices are set to 0xFF.
        dataIndirectBuffer[i] = 255;
    }

    writeBlockToDisk(dataIndexBlock, (unsigned char *)dataIndirectBuffer);

    // reclaim heap memory.
    delete [] dataIndirectBuffer;

    return true;

} // FileSystem::CreateFile


bool FileSystem::DeleteFile(int _file_id)
{
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */

    Inode *inode = LookupFile(_file_id);
    if (inode == nullptr)
    {
        Console::puts("Failed to delete file! File does not exist.\n");
        return false;
    }

    free_blocks[inode->blockNo] = 'f';

    // Inode is free to be allocated to another block.
    inode->isAllocated = false;

    inode->blockNo = -1;
    inode->id = -1;

    writeBlockToDisk(INODE_LIST_BLOCK, (unsigned char *)inodes);
    writeBlockToDisk(FREE_LIST_BLOCK, free_blocks);

    return true;

} // FileSystem::DeleteFile


void FileSystem::readBlockFromDisk(int blockNo, unsigned char *buffer)
{
    disk->read(blockNo, buffer);

} // FileSystem::readBlockFromDisk


void FileSystem::writeBlockToDisk(int blockNo, unsigned char *buffer)
{
    disk->write(blockNo, buffer);

} // FileSystem::writeBlockToDisk

void FileSystem::writeInodeListToDisk(void)
{
    writeBlockToDisk(INODE_LIST_BLOCK, (unsigned char *)inodes);
}
