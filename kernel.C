/*
	File: kernel.C

	Author: R. Bettati
			Department of Computer Science
			Texas A&M University
	Date  : 2024/09/20


	This file has the main entry point to the operating system.

*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define GB * (0x1 << 30)
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_POOL_START_FRAME ((2 MB) / Machine::PAGE_SIZE)
#define KERNEL_POOL_SIZE ((2 MB) / Machine::PAGE_SIZE)
#define PROCESS_POOL_START_FRAME ((4 MB) / Machine::PAGE_SIZE)
#define PROCESS_POOL_SIZE ((28 MB) / Machine::PAGE_SIZE)
/* definition of the kernel and process memory pools */

#define MEM_HOLE_START_FRAME ((15 MB) / Machine::PAGE_SIZE)
#define MEM_HOLE_SIZE ((1 MB) / Machine::PAGE_SIZE)
/* we have a 1 MB hole in physical memory starting at address 15 MB */

#define FAULT_ADDR (4 MB)
/* used in the code later as address referenced to cause page faults. */
//#define NACCESS ((1 MB) / 4)
#define NACCESS (2 KB)
/* NACCESS integer access (i.e. 4 bytes in each access) are made starting at address FAULT_ADDR */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"        /* LOW-LEVEL STUFF */
#include "console.H"
#include "gdt.H"
#include "idt.H"            /* LOW-LEVEL EXCEPTION MGMT. */
#include "irq.H"
#include "exceptions.H"
#include "interrupts.H"

#include "simple_timer.H"   /* SIMPLE TIMER MANAGEMENT */

#include "page_table.H"
#include "paging_low.H"

#include "vm_pool.H"

/*--------------------------------------------------------------------------*/
/* FORWARD REFERENCES FOR TEST CODE */
/*--------------------------------------------------------------------------*/

void TestPassed();
void TestFailed();

void GeneratePageTableMemoryReferences(unsigned long start_address, int n_references);
void GenerateVMPoolMemoryReferences(VMPool* pool, int size1, int size2);

/*--------------------------------------------------------------------------*/
/* MEMORY ALLOCATION */
/*--------------------------------------------------------------------------*/

// Here we overload the new and delete operators to use our vmpools!

VMPool* current_pool;

typedef unsigned int size_t;

//replace the operator "new"
void* operator new (size_t size)
{
	unsigned long a = current_pool->allocate((unsigned long)size);
	return (void*)a;
}

//replace the operator "new[]"
void* operator new[](size_t size)
{
	unsigned long a = current_pool->allocate((unsigned long)size);
	return (void*)a;
}

//replace the operator "delete"
void operator delete (void* p)
{
	current_pool->release((unsigned long)p);
}

//replace the operator "delete[]"
void operator delete[](void* p)
{
	current_pool->release((unsigned long)p);
}

/*--------------------------------------------------------------------------*/
/* EXCEPTION HANDLERS */
/*--------------------------------------------------------------------------*/

/* -- EXAMPLE OF THE DIVISION-BY-ZERO HANDLER */

void dbz_handler(REGS* r)
{
	Console::puts("DIVISION BY ZERO\n");
	for (;;);
}


/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main()
{

	GDT::init();
	Console::init();
	IDT::init();
	ExceptionHandler::init_dispatcher();
	IRQ::init();
	InterruptHandler::init_dispatcher();

	/* -- SEND OUTPUT TO TERMINAL -- */
	Console::redirect_output(true);

	/* -- EXAMPLE OF AN EXCEPTION HANDLER -- */

	class DBZ_Handler : public ExceptionHandler {
		/* We derive Division-by-Zero handler from ExceptionHandler
		   and overload the method handle_exception. */
	public:
		virtual void handle_exception(REGS* _regs)
		{
			Console::puts("DIVISION BY ZERO!\n");
			for (;;);
		}
	} dbz_handler;

	/* Register the DBZ handler for exception no.0
	   with the exception dispatcher. */
	ExceptionHandler::register_handler(0, &dbz_handler);


	/* -- INITIALIZE THE TIMER (we use a very simple timer).-- */

	SimpleTimer timer(100); /* timer ticks every 10ms. */

	/* ---- Register timer handler for interrupt no.0
			with the interrupt dispatcher. */
	InterruptHandler::register_handler(0, &timer);

	/* NOTE: The timer chip starts periodically firing as
	 soon as we enable interrupts.
	 It is important to install a timer handler, as we
	 would get a lot of uncaptured interrupts otherwise. */

	 /* -- ENABLE INTERRUPTS -- */

	Machine::enable_interrupts();

	/* -- INITIALIZE FRAME POOLS -- */

	ContFramePool kernel_mem_pool(KERNEL_POOL_START_FRAME,
		KERNEL_POOL_SIZE,
		0);

	unsigned long n_info_frames =
		ContFramePool::needed_info_frames(PROCESS_POOL_SIZE);

	unsigned long process_mem_pool_info_frame =
		kernel_mem_pool.get_frames(n_info_frames);

	ContFramePool process_mem_pool(PROCESS_POOL_START_FRAME,
		PROCESS_POOL_SIZE,
		process_mem_pool_info_frame);

	/* Take care of the hole in the memory. */
	process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);

	Console::puts("POOLS INITIALIZED!\n");

	/* -- INITIALIZE MEMORY (PAGING) -- */

	/* ---- INSTALL PAGE FAULT HANDLER -- */

	class PageFault_Handler : public ExceptionHandler {
		/* We derive the page fault handler from ExceptionHandler
	   and overload the method handle_exception. */
	public:
		virtual void handle_exception(REGS* _regs)
		{
			PageTable::handle_fault(_regs);
		}
	} pagefault_handler;

	/* ---- Register the page fault handler for exception no. 14
			with the exception dispatcher. */
	ExceptionHandler::register_handler(14, &pagefault_handler);

	/* ---- INITIALIZE THE PAGE TABLE -- */

	PageTable::init_paging(&kernel_mem_pool,
		&process_mem_pool,
		4 MB);

	PageTable pt1;

	pt1.load();

	PageTable::enable_paging();

	/* -- INITIALIZE THE TWO VIRTUAL MEMORY PAGE POOLS -- */

	/* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

	Console::puts("Hello World!\n");

	/* BY DEFAULT WE TEST THE PAGE TABLE IN MAPPED MEMORY!
	   (UNCOMMENT THE FOLLOWING LINE TO TEST THE VM Pools! */
#define _TEST_PAGE_TABLE_

#ifdef _TEST_PAGE_TABLE_

	   /* WE TEST JUST THE PAGE TABLE */
	GeneratePageTableMemoryReferences(FAULT_ADDR, NACCESS);

#else

	   /* WE TEST JUST THE VM POOLS */

	   /* -- CREATE THE VM POOLS. */

	   /* ---- We define the code pool to be a 256MB segment starting at virtual address 512MB -- */
	VMPool code_pool(512 MB, 256 MB, &process_mem_pool, &pt1);

	/* ---- We define a 256MB heap that starts at 1GB in virtual memory. -- */
	VMPool heap_pool(1 GB, 256 MB, &process_mem_pool, &pt1);

	/* -- NOW THE POOLS HAVE BEEN CREATED. */

	Console::puts("VM Pools successfully created!\n");

	/* -- GENERATE MEMORY REFERENCES TO THE VM POOLS */

	Console::puts("I am starting with an extensive test\n");
	Console::puts("of the VM Pool memory allocator.\n");
	Console::puts("Please be patient...\n");
	Console::puts("Testing the memory allocation on code_pool...\n");
	GenerateVMPoolMemoryReferences(&code_pool, 50, 100);
	Console::puts("Testing the memory allocation on heap_pool...\n");
	GenerateVMPoolMemoryReferences(&heap_pool, 50, 100);

#endif

	TestPassed();
}

void GeneratePageTableMemoryReferences(unsigned long start_address, int n_references)
{
	// This tests just the page table. 
	int* foo = (int*)start_address;

	for (int i = 0; i < n_references; i++) {
		foo[i] = i;
	}

	Console::puts("DONE WRITING TO MEMORY. Now testing...\n");

	for (int i = 0; i < n_references; i++) {
		if (foo[i] != i) {
			TestFailed();
		}
	}
}

void GenerateVMPoolMemoryReferences(VMPool* pool, int size1, int size2)
{
	// Here we test the VMPool 
	current_pool = pool;
	for (int i = 1; i < size1; i++) {
		int* arr = new int[size2 * i];
		if (pool->is_legitimate((unsigned long)arr) == false) {
			Console::puts("is_legitimate failed!\n");
			TestFailed();
		}
		for (int j = 0; j < size2 * i; j++) {
			arr[j] = j;
		}
		for (int j = size2 * i - 1; j >= 0; j--) {
			if (arr[j] != j) {
				Console::puts("     j = "); Console::puti(j); Console::puts("value check failed!\n");
				TestFailed();
			}
		}
		delete[] arr;
	}
}

void TestFailed()
{
	Console::puts("Test Failed\n");
	Console::puts("YOU CAN TURN OFF THE MACHINE NOW.\n");
	for (;;);
}

void TestPassed()
{
	Console::puts("Test Passed! Congratulations!\n");
	Console::puts("YOU CAN SAFELY TURN OFF THE MACHINE NOW.\n");
	for (;;);
}
