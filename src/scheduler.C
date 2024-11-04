/*
 File: scheduler.C
 
 Author:    Vinay Balamurali
 Date  :    11/3/2024
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

// Constructor.
Scheduler::Scheduler(void)
{
    readyQueue = new schedulerQueue();

    Console::puts("Constructed Scheduler.\n");

} // Scheduler::Scheduler


// Method to start executing the thread at the
// head of the ready queue.
void Scheduler::yield(void)
{
    if (Machine::interrupts_enabled())
    {
        Machine::disable_interrupts();
    }

    if (readyQueue->fetchSize() == 0)
    {
        Console::puts("No thread present in the Ready Queue to yield to!\n");
        Machine::enable_interrupts();
        return;
    }

    // Fetch current thread to be dispatched.
    Thread *readyThread = readyQueue->dequeue();

    Machine::enable_interrupts();

    // Dispatch to new thread.
    Thread::dispatch_to(readyThread);

} // Scheduler::yield


// Method to add a thread back to the ready queue
// when it was currently executing.
void Scheduler::resume(Thread * _thread)
{
    // Just call 'add' again.
    add(_thread);

} // Scheduler::resume


// Method to add a thread to the ready Queue
// at the tail.
void Scheduler::add(Thread * _thread)
{
    if (Machine::interrupts_enabled())
    {
        Machine::disable_interrupts();
    }

    readyQueue->enqueue(_thread);

    Machine::enable_interrupts();

} // Scheduler::add


// Method to terminate a given thread once it is done
// executing and remove it from the ready queue.
void Scheduler::terminate(Thread * _thread)
{
    if (Machine::interrupts_enabled())
    {
        Machine::disable_interrupts();
    }

    // Special method to remove a thread based on Thread ID.
    readyQueue->removeThreadById(_thread);

    Machine::enable_interrupts();
}

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   RoundRobinScheduler  */
/*--------------------------------------------------------------------------*/


// Constructor.
RoundRobinScheduler::RoundRobinScheduler() : Scheduler() {};


// Method to signal EOI to the Master Interrupt Controller
// and call base class yield.
void RoundRobinScheduler::yield(void)
{
    /* Send an EOI message to the master interrupt controller. */
    Machine::outportb(0x20, 0x20);

    Scheduler::yield();

} //  RoundRobinScheduler::yield
