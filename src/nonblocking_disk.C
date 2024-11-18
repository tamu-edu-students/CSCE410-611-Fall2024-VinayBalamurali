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

// We are limiting this system to have a maxium of 1000 concurrent threads
const int maxThreads = 1000;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

NonBlockingDisk::NonBlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size)
{
    level = new int[maxThreads];
    for (int i = 0; i < maxThreads; i++)
    {
        level[i] = -1;
    }

    victim = new int [maxThreads - 1];
    ioBlockedQueue = new schedulerQueue();

    Console::puts("Constructed object of non blocking disk.\n");

} // NonBlockingDisk::NonBlockingDisk

/*--------------------------------------------------------------------------*/
/* ADDITIONAL FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool NonBlockingDisk::checkIfEqualOrGreater(int currentThread, int index)
{
    // Verify whether any thread has higher priority when compared to the
    // current thread.
    for (int i = 0; i < maxThreads; i++)
    {
        if ((i != currentThread) && (level[i] >= index))
        {
            return true;
        }
    }

    return false;

} // NonBlockingDisk::checkIfEqualOrGreater


void NonBlockingDisk::acquireLock(void)
{
    int threadID = Thread::CurrentThread()->ThreadId();
    for (int i = 0; i < (maxThreads - 1); i++)
    {
        level[threadID] = i; // set currrent thread.
        victim[i] = threadID; // mark current thread as the victim.

        // Acquire the lock if there is no thread with a higher level.
        // Otherwise busy loop.
        while (checkIfEqualOrGreater(threadID, i) &&
               (victim[i] == threadID));
    }

    Console::puts("Lock acquired\n");

} // NonBlockingDisk::acquireLock


void NonBlockingDisk::releaseLock(void)
{
    int threadID = Thread::CurrentThread()->ThreadId();
    level[threadID] = -1; // set current level to -1.

    Console::puts("Lock released\n");

} // NonBlockingDisk::releaseLock


bool NonBlockingDisk::isThreadReady(void)
{
    // Call base class is_ready() and ensure the queue is not empty.
    if ((is_ready()) && (ioBlockedQueue->fetchSize() > 0))
    {
        return true;
    }

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

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void NonBlockingDisk::wait_until_ready(void)
{
    // During Testing it was noticed that the interrupts after read/write are generated
    // instantaneously, indicating that the implementation of 'blocked queue' never
    // came into play.

    // For testing, let's always yield the CPU before issuing a read/write OP.

    // I/O operation is not yet complete.
    ioBlockedQueue->enqueue(Thread::CurrentThread());

    SYSTEM_SCHEDULER->yield();

} // NonBlockingDisk::wait_until_ready


void NonBlockingDisk::read(unsigned long _block_no, unsigned char * _buf)
{
    // Acquire lock before accessing disk.
    acquireLock();
    issue_operation(DISK_OPERATION::READ, _block_no);
    releaseLock();

    wait_until_ready();

    // Acquire lock before accessing disk.
    /* read data from port */
    acquireLock();
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i * 2] = (unsigned char)tmpw;
        _buf[i * 2 + 1] = (unsigned char)(tmpw >> 8);
    }
    releaseLock();

} // NonBlockingDisk::read


void NonBlockingDisk::write(unsigned long _block_no, unsigned char * _buf)
{
    // Acquire lock before accessing disk.
    acquireLock();
    issue_operation(DISK_OPERATION::WRITE, _block_no);
    releaseLock();

    wait_until_ready();

    // Acquire lock before accessing disk.
    /* write data to port */
    acquireLock();
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = _buf[2 * i] | (_buf[2 * i + 1] << 8);
        Machine::outportw(0x1F0, tmpw);
    }
    releaseLock();

} // NonBlockingDisk::write
