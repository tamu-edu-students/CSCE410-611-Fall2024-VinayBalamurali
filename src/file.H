/*
     File        : file.H

     Author      : Riccardo Bettati
     Modified    : 2021/11/18

     Description : Simple File class with sequential read/write operations.
 
*/

#ifndef _FILE_H_
#define _FILE_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */ 
/*--------------------------------------------------------------------------*/

class Inode;
class FileSystem;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* class  F i l e   */
/*--------------------------------------------------------------------------*/

class File  {
    
private:
    /* -- your file data structures here ... */
    
    /* You will need a reference to the inode, maybe even a reference to the 
       file system. 
       You may also want a current position, which indicates which position in 
       the file you will read or write next. */
    
    unsigned char block_cache[SimpleDisk::BLOCK_SIZE];
    /* It will be helpful to have a cached copy of the block that you are reading
       from and writing to. In the base submission, files have only one block, which
       you can cache here. You read the data from disk to cache whenever you open the
       file, and you write the data to disk whenever you close the file. 
    */

public:

    File(FileSystem * _fs, int _id); 
    /* Constructor for the file handle. Set the ’current position’ to be at the 
       beginning of the file. */

    ~File();
    /* Closes the file. Deletes any data structures associated with the file handle. */
  
    int Read(unsigned int _n, char * _buf);
    /* Read _n characters from the file starting at the current position and
       copy them in _buf.  Return the number of characters read. 
       Do not read beyond the end of the file. */
    
    int Write(unsigned int _n, const char * _buf);
    /* Write _n characters to the file starting at the current position. If the write
       extends over the end of the file, extend the length of the file until all data is 
       written or until the maximum file size is reached. Do not write beyond the maximum
       length of the file.  
       Return the number of characters written. */
    
    void Reset();
    /* Set the ’current position’ to the beginning of the file. */
    
    bool EoF();
    /* Is the current position for the file at the end of the file? */

};

#endif
