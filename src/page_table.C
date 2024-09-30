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
   unsigned long ptpFrame = kernel_mem_pool->get_frames(1);
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
   page_directory[0] = page_directory[0] | 0x3;

   // Set remaining entries to be not present.
   // 0 --> kernel mode
   // 1 --> read/write
   // 0 --> not present
   for (unsigned int i = 1; i < ENTRIES_PER_PAGE; i++)
   {
      page_directory[i] = 0x0 | 0x2;
   }

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

  Console::puts("Handle Fault: Page is not present.\n");

  unsigned long pdeIndex = ((faultAddress >> 22) & 0x3FF);

  unsigned long ptpIndex = ((faultAddress >> 12) & 0x3FF);

  unsigned long *newPage = nullptr;

  // Check if the Page Directory entry is present.
  if ((curr_page_dir[pdeIndex] & 0x1) == 1)
  {
      // If yes, then we need only load the corresponding page table
      // and update the entry by setting bit 0. 
      Console::puts("PDE reference is valid. Page Table Entry is not present.\n");
      // The frames are pulled from the process pool since the test program (application)
      // is trying to allocate memory.
      unsigned long frame = process_mem_pool->get_frames(1);
      newPage = (unsigned long *)(frame * PAGE_SIZE);

      // Update the page table.
      unsigned long *pageTable = (unsigned long *)(curr_page_dir[pdeIndex] & 0xFFFFF000);
      pageTable[ptpIndex] = (unsigned long)newPage | 0x3;
  }
  else
  {
      // If no, then the page table page for that PDE does not exist.
      // It must be allocated first and mapped to the corresponding PDE.
      Console::puts("PDE reference is invalid. A new Page Table Page must be allocated.\n");
      unsigned long ptpFrame = kernel_mem_pool->get_frames(1);
      newPage = (unsigned long *)(ptpFrame * PAGE_SIZE);

      curr_page_dir[pdeIndex] = (unsigned long)newPage | 0x3;

      // Set other values of the page table page as follows:
      // 0 --> kernel mode
      // 1 --> read/write
      // 0 --> page not present.
      for (int i = 0; i < ENTRIES_PER_PAGE; i++)
      {
         newPage[i] = 0x2;
      }
  }

  Console::puts("handled page fault\n");

} // PageTable::handle_fault

