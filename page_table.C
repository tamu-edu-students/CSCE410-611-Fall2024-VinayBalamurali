#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = nullptr;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = nullptr;
ContFramePool * PageTable::process_mem_pool = nullptr;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    assert(false);
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    assert(false);
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    assert(false);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    assert(false);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    assert(false);
    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    assert(false);
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    assert(false);
    Console::puts("freed page\n");
}
