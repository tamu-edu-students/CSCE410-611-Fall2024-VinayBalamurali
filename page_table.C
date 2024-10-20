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

VMPool * PageTable::poolHead = nullptr;


// Method to initialize paging properties.
void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;

   Console::puts("Initialized Paging System\n");

} // PageTable::init_paging


// Constructor.
PageTable::PageTable()
{
   // 1 Page Table Directory for the entire process.
   unsigned long pdeFrame = kernel_mem_pool->get_frames(1);
   this->page_directory = (unsigned long *)(pdeFrame * PAGE_SIZE);

   // 1 Page table page for mapping to the first entry in the PDE.
   unsigned long ptpFrame = process_mem_pool->get_frames(1);
   unsigned long * page_table_page;
   page_table_page = (unsigned long *)(ptpFrame * PAGE_SIZE);

   unsigned long addressRange = 0;
   // Set attributes of the page table page entries to 011 (last 3 bits)
   // 0 --> kernel mode
   // 1 --> read/write
   // 1 --> protection fault
   // Each page_table_page entry references 4096 bytes of memory
   for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++)
   {
       page_table_page[i] = addressRange | 0x3;
       addressRange += PAGE_SIZE;
   }

   // Set the first entry of the PDE to be the page_table_page.
   page_directory[0] = (unsigned long)page_table_page;
   // Map the first 4MB of memory.
   page_directory[0] = page_directory[0] | 0x3;

   // Set remaining entries to be not present.
   // 0 --> kernel mode
   // 1 --> read/write
   // 0 --> not present
   for (unsigned int i = 1; i < (ENTRIES_PER_PAGE - 1); i++)
   {
       page_directory[i] = 0x0 | 0x2;
   }
   // Point last entry back to itself.
   page_directory[1023] = (unsigned long)page_directory | 0x3;

   Console::puts("Constructed Page Table object\n");

} // PageTable::PageTable


// Method to set 'this' object as the current page table.
void PageTable::load()
{
   current_page_table = this;

   // Write the current page directort to the CR3 register.
   write_cr3((unsigned long)current_page_table->page_directory);
   Console::puts("Loaded page table\n");

} // PageTable::load


// Method to enable paging by writing a high bit into
// the cr0 register.
void PageTable::enable_paging()
{
   // Enable paging on x86 by setting the MSB of
   // CR0 to 'high'.
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;

   Console::puts("Enabled paging\n");

} // PageTable::enable_paging


// Method to handle the page fault exception.
void PageTable::handle_fault(REGS * _r)
{
   Console::puts("Page fault exception triggered!\n");
   // Fetch inaccessible address from register CR2.
   unsigned long faultAddress = read_cr2();

   // Load current page directory of the process from CR3.
   unsigned long *curr_page_dir = (unsigned long*)read_cr3();
   if (curr_page_dir == nullptr)
   {
       Console::puts("Failed to fetch Page Directory!\n");
       Console::puts("Exiting\n");
       assert(false);
   }

   // Only handle exception if page fault was triggerred by
   // page not being present.
   unsigned long errorCode = _r->err_code;
   if ((errorCode & 0x1) == 1)
   {
       Console::puts("Handle Fault: Page is already present! Protection violation!\n");
       Console::puts("Exiting!\n");
       assert(false);
   }

   VMPool *node = poolHead;

   if (node == nullptr)
   {
       Console::puts("Region cannot be freed! VM pool does not exist.\n");
       assert(false);
   }

   bool isLegitmate = false;
   while (node != nullptr)
   {
       if (node->is_legitimate(faultAddress))
       {
           isLegitmate = true;
           break;
       }

       node = node->next;
   }

   if (!(isLegitmate))
   {
       Console::puts("Not a legitimate address! Memory has not been allocated for this region!\n");
       assert(false);
   }

   Console::puts("Handle Fault: Page is not present.\n");

   unsigned long pdeIndex = ((faultAddress >> 22) & 0x3FF);

   unsigned long ptpIndex = ((faultAddress >> 12) & 0x3FF);

   // Check if the Page Directory entry is present.
   if ((curr_page_dir[pdeIndex] & 0x1) == 1)
   {
       unsigned long frame = process_mem_pool->get_frames(1);
       unsigned long *newPage = (unsigned long *)(frame * PAGE_SIZE);

       unsigned long *pageEntry = (unsigned long *)((0x3FF << 22) | (pdeIndex << 12));

       pageEntry[ptpIndex] =  (unsigned long)newPage | 0x3;
   }
   else
   {
       // PDE is not valid.
       unsigned long ptpFrame = process_mem_pool->get_frames(1);
       unsigned long *newPageTable = (unsigned long *)(ptpFrame * PAGE_SIZE);

       unsigned long *pdeEntry = (unsigned long *)(0xFFFFF << 12);
       pdeEntry[pdeIndex] = (unsigned long)newPageTable | 0x3;

       // Set other values of the page table page as follows:
      // 0 --> kernel mode
      // 1 --> read/write
      // 0 --> page not present.
      for (int i = 0; i < ENTRIES_PER_PAGE; i++)
      {
          newPageTable[i] = 0x2;
      }

   }

   Console::puts("handled page fault\n");

} // // PageTable::handle_fault


void PageTable::register_pool(VMPool * _vm_pool)
{
    if (PageTable::poolHead == nullptr)
    {
        // first pool that is going to be registered.
        PageTable::poolHead = _vm_pool;
    }
    else
    {
        VMPool *currNode = PageTable::poolHead;

        while (currNode->next != nullptr)
        {
            currNode = currNode->next;
        }

        currNode->next = _vm_pool;
    }

    Console::puts("registered VM pool\n");

} // PageTable::register_pool


void PageTable::free_page(unsigned long _page_no)
{
    unsigned long pdeIndex = ((_page_no >> 22) & 0x3FF);

    unsigned long ptpIndex = ((_page_no >> 12) & 0x3FF);

    unsigned long *pageTablePage = (unsigned long *)((0x3FF << 22) | (pdeIndex << 12));

    unsigned long frame = (pageTablePage[ptpIndex] & 0xFFFFF000) / PAGE_SIZE;

    process_mem_pool->release_frames(frame);

    pageTablePage[ptpIndex] = pageTablePage[ptpIndex] | 0x2;

    // Flush the TLB.
    load();

    Console::puts("freed page\n");

} // PageTable::free_page
