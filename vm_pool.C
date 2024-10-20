/*
 File: vm_pool.C
 
 Author:
 Date  : 2024/09/20
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table)
{
    baseAddress = _base_address;
    size = _size;
    availableMemory = size;
    framePool = _frame_pool;
    pageTable = _page_table;

    next = nullptr;
    totalRegions = 0;

    pageTable->register_pool(this);

    vmRegionInfo *region = (vmRegionInfo *)baseAddress;
    region[0].baseAddress = baseAddress;
    region[0].length = PageTable::PAGE_SIZE;
    allRegions = region;

    totalRegions = totalRegions + 1;

    availableMemory = availableMemory - PageTable::PAGE_SIZE;

    Console::puts("Constructed VMPool object.\n");

} // VMPool::VMPool


unsigned long VMPool::allocate(unsigned long _size)
{
    if (_size > availableMemory)
    {
        Console::puts("Requested Memory greater than available size!\n");
        return 0;
    }

    unsigned long pagesRequired = 0;

    // Divide to get total pages.
    pagesRequired = (_size / PageTable::PAGE_SIZE);

    // We also need to account for the last page. We allocate an entire page
    // and don't care about internal fragmentation.
    unsigned long extraPages = (_size % PageTable::PAGE_SIZE) > 0 ? 1 : 0;
    pagesRequired = pagesRequired + extraPages;

    unsigned long requiredMemory = pagesRequired * PageTable::PAGE_SIZE;
    unsigned long newBaseAddress = allRegions[totalRegions - 1].baseAddress + allRegions[totalRegions - 1].length;;
    // totalRegions indexes the region that is being allocated.
    allRegions[totalRegions].baseAddress = newBaseAddress;
    allRegions[totalRegions].length = requiredMemory;

    availableMemory = availableMemory - requiredMemory;

    totalRegions = totalRegions + 1;

    Console::puts("Allocated region of memory.\n");

    return newBaseAddress;

} // VMPool::allocate


void VMPool::release(unsigned long _start_address)
{
    int indexRegion = -1;

    for (int i = 1 ; i < totalRegions; i++)
    {
        if (allRegions[i].baseAddress == _start_address)
        {
            indexRegion = i;
        }
    }

    if (indexRegion == -1)
    {
        Console::puts("Given start address not found in the current VM pool!\n");
        assert(false);
    }

    unsigned long pagesToRelease = (allRegions[indexRegion].length / PageTable::PAGE_SIZE);

    unsigned long tempAddress = _start_address;
    while (pagesToRelease > 0)
    {
        pageTable->free_page(tempAddress);

        pagesToRelease = pagesToRelease - 1;

        tempAddress = tempAddress + PageTable::PAGE_SIZE;
    }

    availableMemory =  availableMemory + allRegions[indexRegion].length;

    // Shift all regions greater than the one to be freed
    // to delete the current region.
    for (int i = indexRegion; i < totalRegions; i++)
    {
        allRegions[i] = allRegions[i+1];
    }

    totalRegions = totalRegions - 1;

    Console::puts("Released region of memory.\n");

} // VMPool::release


bool VMPool::is_legitimate(unsigned long _address)
{
    Console::puts("Checked whether address is part of an allocated region.\n");
    
    if ((_address < baseAddress) || (_address > (baseAddress + size)))
    {
        return false;
    }

    return true;

} // VMPool::is_legitimate
