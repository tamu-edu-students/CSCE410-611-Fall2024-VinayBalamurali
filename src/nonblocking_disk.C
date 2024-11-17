/*
     File        : nonblocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "scheduler.H"
#include "thread.H"
#include "nonblocking_disk.H"

extern Scheduler *SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

NonBlockingDisk::NonBlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size)
{
    ioBlockedQueue = new schedulerQueue();

    Console::puts("Constructed object of non blocking disk.\n");
} // NonBlockingDisk::NonBlockingDisk


/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void NonBlockingDisk::wait_until_ready(void)
{
    Console::puts("Calling Non Blocking NonBlockingDisk::wait_until_ready.\n");

    // During Testing it was noticed that the interrupts after read/write are generated
    // instantaneously, indicating that the implementation of 'blocked queue' never
    // came into play.

    // For testing, let's always yield the CPU before issuing a read/write OP.

    // I/O operation is not yet complete.
    ioBlockedQueue->enqueue(Thread::CurrentThread());

    SYSTEM_SCHEDULER->yield();

} // NonBlockingDisk::wait_until_ready


bool NonBlockingDisk::isThreadReady(void)
{
    if ((is_ready()) && (ioBlockedQueue->fetchSize() > 0))
    {
        // Console::puts("Thread is ready.\n");
        return true;
    }

    // Console::puts("Thread is not ready\n");
    return false;

} // NonBlockingDisk::isThreadReady


Thread* NonBlockingDisk::scheduleBlockedThread(void)
{
    if (ioBlockedQueue->fetchSize() >  0)
    {
        Console::puts("Fetching from Blocked queue\n");
        Thread *popThread = ioBlockedQueue->dequeue();
        return popThread;
    }

    return nullptr;

} // NonBlockingDisk::scheduleBlockedThread


void NonBlockingDisk::read(unsigned long _block_no, unsigned char * _buf)
{
    issue_operation(DISK_OPERATION::READ, _block_no);

    wait_until_ready();

    /* read data from port */
	int i;
	unsigned short tmpw;
	for (i = 0; i < 256; i++) {
		tmpw = Machine::inportw(0x1F0);
		_buf[i * 2] = (unsigned char)tmpw;
		_buf[i * 2 + 1] = (unsigned char)(tmpw >> 8);
	}

} // NonBlockingDisk::read


void NonBlockingDisk::write(unsigned long _block_no, unsigned char * _buf)
{
    issue_operation(DISK_OPERATION::WRITE, _block_no);

	wait_until_ready();

	/* write data to port */
	int i;
	unsigned short tmpw;
	for (i = 0; i < 256; i++) {
		tmpw = _buf[2 * i] | (_buf[2 * i + 1] << 8);
		Machine::outportw(0x1F0, tmpw);
	}

} // NonBlockingDisk::write
