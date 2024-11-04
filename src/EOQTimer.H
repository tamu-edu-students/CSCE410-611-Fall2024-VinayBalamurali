/* 
    File: EOQTimer.H

    Author: Vinay Balamurali
            Texas A&M University
    Date  : 11/03/24

    This class derives from class SimpleTimer
    and is used primarily to determine the end
    of quantum to pre-empt threads in a round robin
    scheduler.

*/

#ifndef _EOQ_TIMER_H_
#define _EOQ_TIMER_H_

#include "simple_timer.H"

// Forward Declarations.
class Scheduler;

class EOQTimer : public SimpleTimer
{
private:
    // Refernece to scheduler.
    Scheduler *SYSTEM_SCHEDULER;

public:
    // Constructor.
    EOQTimer(int _hz, Scheduler *_scheduler);

    // Method to override base class interrupt handler to handle
    // End Of Quantum preemption.
    virtual void handle_interrupt(REGS *_r) override;
};

#endif /* _EOQ_TIMER_H_ */
