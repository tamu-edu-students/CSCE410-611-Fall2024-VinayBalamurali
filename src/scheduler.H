/* 
    Author: R. Bettati, Joshua Capehart
            Department of Computer Science
            Texas A&M University
			
	    A thread scheduler.

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "thread.H"
#include "console.H"

/*--------------------------------------------------------------------------*/
/* !!! IMPLEMENTATION HINT !!! */
/*--------------------------------------------------------------------------*/
/*
    One way to proceed is to implement the FIFO scheduling policy inside
    class 'Scheduler'. 

    If you plan to implement a Round-Robin Scheduler, derive it from class
    'Scheduler', say as class 'RRScheduler'. The class 'RRScheduler' is 
    really just a FIFO scheduler with THREE MODIFICATIONS:
    1. It manages a timer, which fires at the end-of-quantum (EOQ). 
    (For details on how to set up a timer and how to handle timer interrupts 
    see the 1-second timer in 'kernel.C'.)  The timer is set up in the
    constructor.
    2. It uses an additional function, the EOQ handler. This function gets
    called whenever an EOQ timer event fires. The EOQ handler forces the 
    current thread to call the scheduler's 'yield' function.
    3. The 'yield' function must be modified to account for unused quantum
    time. If a thread voluntarily yields, the EOQ timer must be reset in order
    to not penalize the next thread.
 
    (Note that this qualifies as programming at about the level of a baboon.
     Much better woudl be to have the abstract class 'Scheduler' implement 
     the basic scheduling MECHANISMS and provide abstract funtions to define
     the queue management POLICIES in derived classes, 
     such as 'FIFOScheduler'.)
    
 */

/*--------------------------------------------------------------------------*/
/* Ready Queue  and Node data structure */
/*--------------------------------------------------------------------------*/

// Data structure to hold a Node to be inserted in the ready queue.
// The data is a pointer to the thread to be executed.
// Has a self pointer to the next node in the linked list.
class Node
{
public:

   Thread *threadPtr;  // Pointer to the 'to be executed' thread.
   Node *next;  // Pointer to next node.

public:

   // Constructor.
   Node(Thread *_thread)
   {
      this->threadPtr = _thread;
      this->next = nullptr;
   }

};


// Class to implement the ready queue.
class schedulerQueue
{

private:

   Node *head;  // 'Head' or 'front' of queue.
   Node *tail;  // 'Tail' or 'rear' of queue.
   unsigned int length;  // size of queue.

public:

   // Constructor.
   schedulerQueue(void)
   {
      head = nullptr;
      tail = nullptr;
      length = 0;
   }


   // Method to add an Thread to the ready queue
   // at the tail.
   void enqueue(Thread *_thread)
   {
      Node *newNode = new Node(_thread);
      if (tail == nullptr)
      {
         tail = newNode;
         head = tail;
         length = 1;
         return;
      }

      tail->next = newNode;
      tail = newNode;
      length ++;
      return;
   }

   // Method to remove the element from the front
   // of the queue to be dispatched to the CPU.
   Thread* dequeue(void)
   {
      if (head == nullptr)
      {
         Console::puts("Scheduler queue is empty. Nothing to dispatch!\n");
         return nullptr;
      }

      Node *pop = head;
      Thread *_thread = pop->threadPtr;
      head = head->next;

      if (head == nullptr)
      {
         tail = nullptr;
      }

      length --;
      delete pop;
      return _thread;
      
   }

   // Method to remove a thread from the ready queue
   // as it got terminated.
   void removeThreadById(Thread* _thread)
   {
      Node *curr = head;
      Node *prev = nullptr;

      while ((curr != nullptr) && (curr->threadPtr != _thread))
      {
         prev = curr;
         curr = curr->next;
      }

      if (curr == nullptr)
      {
         Console::puts("Cannot terminate given thread!\n");
         return;
      }

      // If current pointer is at the front,
      // just peform a dequeue.
      if (curr == head)
      {
         Console::puts("Terminating thread at the front.\n");
         Thread *termThread = this->dequeue();
         return;
      }

      // Node to be deleted is the tail.
      if (curr == tail)
      {
         Console::puts("Terminating thread at the tail.\n");
         tail = prev;
         delete(curr);
         length --;
         return;
      }

      Console::puts("Terminating thread in Ready Queue.\n");
      prev->next = curr->next;
      length --;
      delete(curr);

   }

   // Method to fetch the number
   // of elements in the queue.
   unsigned int fetchSize(void)
   {
      return length;
   }

};

/*--------------------------------------------------------------------------*/
/* SCHEDULER */
/*--------------------------------------------------------------------------*/


class Scheduler {

  /* The scheduler may need private members... */
protected:

  schedulerQueue *readyQueue = nullptr;

public:

   Scheduler();
   /* Setup the scheduler. This sets up the ready queue, for example.
      If the scheduler implements some sort of round-robin scheme, then the 
      end_of_quantum handler is installed in the constructor as well. */

   /* NOTE: We are making all functions virtual. This may come in handy when
            you want to derive RRScheduler from this class. */
  
   virtual void yield();
   /* Called by the currently running thread in order to give up the CPU. 
      The scheduler selects the next thread from the ready queue to load onto 
      the CPU, and calls the dispatcher function defined in 'Thread.H' to
      do the context switch. */

   virtual void resume(Thread * _thread);
   /* Add the given thread to the ready queue of the scheduler. This is called
      for threads that were waiting for an event to happen, or that have 
      to give up the CPU in response to a preemption. */

   virtual void add(Thread * _thread);
   /* Make the given thread runnable by the scheduler. This function is called
      after thread creation. Depending on implementation, this function may 
      just add the thread to the ready queue, using 'resume'. */

   virtual void terminate(Thread * _thread);
   /* Remove the given thread from the scheduler in preparation for destruction
      of the thread. 
      Graciously handle the case where the thread wants to terminate itself.*/
  
};

// Class to implement the Round Robin Scheduling mechanism.
class RoundRobinScheduler: public Scheduler
{
public:
   // Constructor.
   RoundRobinScheduler(void);

   // Method to signal EOI to the Master Interrupt Controller
   // and call base class yield.
   virtual void yield(void) override;
};

#endif
