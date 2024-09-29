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
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

/*
PageTable::PageTable()
{
   // 1 Page Table Directory for the entire process.
   unsigned long pdeFrame = kernel_mem_pool->get_frames(1);
   this->page_directory = (unsigned long *)(pdeFrame * PAGE_SIZE);

   // 1 Page table page for mapping to the first entry in the PDE.
   unsigned long ptpFrame = kernel_mem_pool->get_frames(1);
   unsigned long        * page_table_page;
   page_table_page = (unsigned long *)(ptpFrame * PAGE_SIZE);

   unsigned addressRange = 0;
   // Set attributes of the page table page entries to 011 (last 3 bits)
   // 0 --> kernel mode
   // 1 --> read/write
   // 1 --> protection fault
   // Each page_table_page entry references a 4096 bytes of memory
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
}*/


void PageTable::load()
{
   current_page_table = this;

   write_cr3((unsigned long)current_page_table->page_directory[0]);
   Console::puts("Loaded page table\n");
}


void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = true;
   Console::puts("Enabled paging\n");
}


void PageTable::handle_fault(REGS * _r)
{
  unsigned long faultAddress = read_cr2();

  // Only handle exception if page fault was triggerred by
  // page not being present.
  unsigned long errorCode = _r->err_code;
  if ((errorCode & 0x1) != 1)
  {
      Console::puts("Page fault exception cannot be processed!\n");
      return; 
  }

  Console::puts("Page fault exception triggered by page not being present.");

  // get the PDE entry first.
  unsigned long pdeIndex = ((faultAddress >> 22) & 0x3FF);
  unsigned long pageTablePageEntry = current_page_table->page_directory[pdeIndex];
  unsigned long ptpIndex = ((faultAddress >> 12) & 0x3FF);
  //unsigned long frameNumber = pageTablePage[ptpIndex];

  unsigned long *newPage = nullptr;

  if ((pageTablePageEntry & 0x1) == 0x1)
  {
      Console::puts("PDE reference is valid. Page Table Entry is not present.\n");
      unsigned long frame = process_mem_pool->get_frames(1);
      newPage = (unsigned long *)(frame * PAGE_SIZE);

      unsigned long *pageTable = (unsigned long *)(pageTablePageEntry);
      pageTable[ptpIndex] = (unsigned long)newPage | 0x3;
  }
  else
  {
      Console::puts("PDE reference is invalid. A new Page Table Page must be allocatted.\n");
      unsigned long ptpFrame = kernel_mem_pool->get_frames(1);
      newPage = (unsigned long *)(ptpFrame * PAGE_SIZE);

      for (int i = 0; i < ENTRIES_PER_PAGE; i++)
      {
         newPage[i] = 0x2;
      }
  }

  Console::puts("handled page fault\n");

}

