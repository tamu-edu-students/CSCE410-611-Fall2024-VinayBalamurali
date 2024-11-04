/*
    File: EOQTimer.C

    Author: Vinay Balamurali
            Texas A&M University
    Date  : 11/03/24

*/

#include "console.H"
#include "scheduler.H"
#include "EOQTimer.H"

// Constructor.
EOQTimer::EOQTimer(int _hz, Scheduler* _scheduler) : SimpleTimer(_hz)
{
    Console::puts("Constructing EOQTimer\n");
    this->SYSTEM_SCHEDULER = _scheduler;

} // EOQTimer::EOQTimer


// Method to override base class interrupt handler to handle
// End Of Quantum preemption.
void EOQTimer::handle_interrupt(REGS *_r)
{
    Console::puts("Time quantum has passed.\n");
    Console::puts("Pre-empting current thread.\n");
    SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();

} // EOQTimer::handle_interrupt
