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

/* -- (none) -- */

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

    this->inode = fs->LookupFile(fileId);
    if (inode == nullptr)
    {
        Console::puts("Failed to obtain inode for the file!\n");
        assert(false);
    }

    fs->readBlockFromDisk(inode->blockNo, block_cache);

} // File::File


File::~File()
{
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */

    fs->writeBlockToDisk(inode->blockNo, block_cache);

} // File::~File

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf)
{
    Console::puts("reading from file\n");

    int count;
    for (count = 0; count < _n; count++)
    {
        if (EoF())
        {
            break;
        }

        _buf[count] = block_cache[currPos];
        currPos++;
    }

    return count;

} // File::Read


int File::Write(unsigned int _n, const char *_buf)
{
    Console::puts("writing to file\n");

    int count;
    for (count  = 0; count < _n; count++)
    {
        if (EoF())
        {
            break;
        }

        block_cache[currPos] = _buf[count];
        currPos;
    }

    return count;

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
