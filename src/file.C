/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MAX_BLOCKS 128

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id)
{
    Console::puts("Opening file.\n");

    this->fs = _fs;
    this->fileId = _id;
    this->Reset();

    // Check if Inode exists, else do not
    // proceed.
    this->inode = fs->LookupFile(fileId);
    if (inode == nullptr)
    {
        Console::puts("Failed to obtain inode for the file!\n");
        assert(false);
    }

    // Load the Index block.
    fs->readBlockFromDisk(inode->blockNo, dataIndexBlock);

} // File::File


File::~File()
{
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */

    fs->writeBlockToDisk(inode->blockNo, dataIndexBlock);
    fs->writeInodeListToDisk();

} // File::~File

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf)
{
    Console::puts("reading from file\n");

    // Pointers to current block and current index.
    unsigned int currBlock = (currPos / SimpleDisk::BLOCK_SIZE);
    unsigned int currIndex = (currPos % SimpleDisk::BLOCK_SIZE);

    // Ensure data to be read does not overshoot
    // the size of the file.
    unsigned int dataToRead;
    if (_n > (inode->size - currPos))
    {
        dataToRead = (inode->size - currPos);
    }
    else
    {
        dataToRead = _n;
    }

    unsigned int dataRead = 0;

    while (dataToRead > 0)
    {
        if ((dataIndexBlock[currBlock] == 255) || (currBlock >= MAX_BLOCKS))
        {
            Console::puts("No Data in file!\n");
            return dataRead;
        }

        fs->readBlockFromDisk(dataIndexBlock[currBlock], block_cache);

        // copy only the minimum of the remaining data in the block
        // or the remaining data left to read.
        unsigned int dataInBlock = SimpleDisk::BLOCK_SIZE - currIndex;
        unsigned int readCount = (dataInBlock <= dataToRead) ? dataInBlock : dataToRead;

        // copu data to source buffer.
        memcpy((_buf + dataRead), (block_cache + currIndex), readCount);

        // increment/ decrement temporary variables.
        dataRead += readCount;
        dataToRead -= readCount;

        // Traverse to next block.
        currIndex = 0;
        currBlock ++;

        // Increment current position after every Block read.
        currPos += readCount;
    }

    currPos += dataRead;

    return dataRead;

} // File::Read


int File::Write(unsigned int _n, const char *_buf)
{
    Console::puts("writing to file\n");

    unsigned int currBlock = (currPos / SimpleDisk::BLOCK_SIZE);
    unsigned int currIndex = (currPos % SimpleDisk::BLOCK_SIZE);

    unsigned int dataWritten = 0;
    unsigned int dataToWrite = _n;

    while (dataToWrite > 0)
    {
        if (currBlock >= MAX_BLOCKS)
        {
            Console::puts("Max file size reached! This file system only"
                          "supports files of size 64KB.\n");
            return dataWritten;
        }

        if (dataIndexBlock[currBlock] == 255)
        {
            int newDataBlock = fs->GetFreeBlock();
            if (newDataBlock == -1)
            {
                Console::puts("Memory full! Out of free blocks.\n");
                return dataWritten;
            }

            // Add new block to the index table.
            dataIndexBlock[currBlock] = newDataBlock;

            Console::puts("Allocated block: "); Console::puti(newDataBlock);
            Console::puts("\n");
        }

        // Read the current block from disk.
        fs->readBlockFromDisk(dataIndexBlock[currBlock], block_cache);

        // Similar to 'read'.
        unsigned int freeSizeInBlock = SimpleDisk::BLOCK_SIZE - currIndex;
        unsigned int writeCount = (freeSizeInBlock <= dataToWrite) ? freeSizeInBlock : dataToWrite;

        memcpy((block_cache + currIndex), (_buf + dataWritten), writeCount);

        // Write tht block back to disk.
        fs->writeBlockToDisk(dataIndexBlock[currBlock], block_cache);

        // Update temporary variables.
        dataWritten += writeCount;
        dataToWrite -= writeCount;
        currPos += dataWritten;

        // Traverse to the next block.
        currIndex = 0;
        currBlock ++;
    }

    inode->size = currPos;

    return dataWritten;

} // File::Write


void File::Reset()
{
    Console::puts("resetting file\n");

    currPos = 0;

} // File::Reset


bool File::EoF()
{
    Console::puts("checking for EoF\n");

    if (currPos >= SimpleDisk::BLOCK_SIZE)
    {
        return true;
    }

    return false;

} // File::EoF
