/*
     File        : nonblocking_disk.H

     Author      : 

     Date        : 
     Description : 

*/

#ifndef _NONBLOCKING_DISK_H_
#define _NONBLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

class schedulerQueue;
class Thread;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* N o n B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class NonBlockingDisk : public SimpleDisk {

private:
   // To implement fitler lock.
   int *level;
   int *victim;
   schedulerQueue *ioBlockedQueue = nullptr;

   // Methods to acuire and release locks.
   // This is done using the Filter Lock algorithm.
   void acquireLock(void);
   void releaseLock(void);

   // Internal method to implement FilterLock.
   bool checkIfEqualOrGreater(int currentThread, int index);

public:
   NonBlockingDisk(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a NonBlockingDisk device with the given size connected to the 
      MASTER or DEPENDENT slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */

   /* DISK OPERATIONS */

   virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */

   // Override base class method.
   virtual void wait_until_ready(void) override;

   // Method to check if a thread is ready tp be added back to the
   // Ready Queue.
   bool isThreadReady(void);

   // Method to pick a thread from the blocked queue, to be added
   // back to the ready queue.
   Thread* scheduleBlockedThread(void);

};

#endif
