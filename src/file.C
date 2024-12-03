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

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    Console::puts("FUNCTION NOT IMPLEMENTED\n");
    assert(false);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    Console::puts("FUNCTION NOT IMPLEMENTED\n");
    assert(false);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    Console::puts("FUNCTION NOT IMPLEMENTED\n");
    assert(false);
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    Console::puts("FUNCTION NOT IMPLEMENTED\n");
    assert(false);
}

void File::Reset() {
    Console::puts("resetting file\n");
    Console::puts("FUNCTION NOT IMPLEMENTED\n");
    assert(false);
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    Console::puts("FUNCTION NOT IMPLEMENTED\n");
    assert(false);
}
