/*
	File: kernel.C

	Author: R. Bettati
			Department of Computer Science
			Texas A&M University
	Date  : 2021/11/28


	This file has the main entry point to the operating system.

	MAIN FILE FOR MACHINE PROBLEM "FILE SYSTEM"

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"         /* LOW-LEVEL STUFF   */
#include "console.H"
#include "gdt.H"
#include "idt.H"             /* EXCEPTION MGMT.   */
#include "irq.H"
#include "exceptions.H"     
#include "interrupts.H"

#include "assert.H"

#include "simple_timer.H"    /* TIMER MANAGEMENT  */

#include "frame_pool.H"      /* MEMORY MANAGEMENT */
#include "mem_pool.H"

#include "simple_disk.H"     /* DISK DEVICE */

#include "file_system.H"     /* FILE SYSTEM */
#include "file.H"

/*--------------------------------------------------------------------------*/
/* MEMORY MANAGEMENT */
/*--------------------------------------------------------------------------*/

/* -- A POOL OF FRAMES FOR THE SYSTEM TO USE */
FramePool* SYSTEM_FRAME_POOL;

/* -- A POOL OF CONTIGUOUS MEMORY FOR THE SYSTEM TO USE */
MemPool* MEMORY_POOL;

typedef unsigned int size_t;

//replace the operator "new"
void* operator new (size_t size) {
	unsigned long a = MEMORY_POOL->allocate((unsigned long)size);
	return (void*)a;
}

//replace the operator "new[]"
void* operator new[](size_t size) {
	unsigned long a = MEMORY_POOL->allocate((unsigned long)size);
	return (void*)a;
}

//replace the operator "delete"
void operator delete (void* p, size_t s) {
	MEMORY_POOL->release((unsigned long)p);
}


//replace the operator "delete[]"
void operator delete[](void* p) {
	MEMORY_POOL->release((unsigned long)p);
}

/*--------------------------------------------------------------------------*/
/* DISK */
/*--------------------------------------------------------------------------*/

/* -- A POINTER TO THE SYSTEM DISK */
IDEController* IDE_CONTROLLER;
SimpleDisk* SYSTEM_DISK;

#define SYSTEM_DISK_SIZE (10 MB)

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM */
/*--------------------------------------------------------------------------*/

/* -- A POINTER TO THE SYSTEM FILE SYSTEM */
FileSystem* FILE_SYSTEM;

/*--------------------------------------------------------------------------*/
/* CODE TO EXERCISE THE FILE SYSTEM */
/*--------------------------------------------------------------------------*/

void exercise_larger_file_system(FileSystem* _file_system)
{
	assert(_file_system->CreateFile(5));

	const char* randomString =
    "2XV77nl\\@zo{Gv)3aO))ad\\?&1D_ub(Ni;nMK'y~k9xTaNbB>zc5s4b@u0|s]`eMk.){#JM&puP%^"
    "TOGH|,gb6zYgg2-s/aQ*@Z:xdCCZe=uySO9u_9@!BQ6F2~$:GycqiW]TF!%Nj_h`d<lkm\"):(+\\[O^8"
    "Rf+al!nc0(Ls$4OubTi@GB!nz?/[o5$1hKx0dduZtSvK#VIe;>+F[Y}BN7I?-As?l;mO~`a~R-CayIb,"
    "IW8\"|jUwbS&]&uB$}QPkiJfAhS7'N$A/\\j;}4yc7N{WO%78q<4J>MDM-[#\\.]C;WWw.e-Ea/w7<Za%"
    "{6]C!%S;-rs)*D}ER$y^et5}Np&b{*{wKDmHf'gbw*CIa*Zc&j!~H|Sm/dQe'10?gwuN~iIE.)W%I'_"
    ">;U@BIZdoVkT.9=yQ)/%R1I>\\RRGaS+-Gzt`CvSw^~eE,$nl=!z7xscS\\yr=W/p1Aj]7~g(^KldiJko"
    ">B(bq{b:d+5zC<P4A]+=M!<[YNuE)su[b\\g\"OQ=z=z2mIx^>]\'&LMj;*YYd-6jf<L?de*y8Ks~qJfdO"
    "'F,'RY4(|YQAX=#e0H\"/@yFRmB[uME:nAcs+uvwD.Fi!OsWmIJ.xx7cs+*(5P9<\\Sg._JmCBA$3Y:,u"
    "X6^4~>YI{g)Cv+_vh6=0&H&MY;/EjmM7:|\\P#3EV[#+C]-(vF;Adc)<iI,-PQ@1o/~\"&P1?SU-Oqtcy"
    "@r-<KRW_IGQHmi{H]J.'WE3,}3<KUHi*ph<r@w%DZ#>No?3?zL#],UL[/E=+}Pi2wnzbK)@_}6%j]QDV"
    "|CN.ke5\"w7Yob)<goTTa_KpJF:0`0y$U3Ufy6<pr(Mll=IOx*|HVsEfkih;FfA%E)'dMaSZsNCH-VMDU"
    "sG8QFVsv&KYEahMakWBHYT+>dQRh4!,!X3dAudO)g\\ZJC(0(XTRkC}^OFm<9?z:G?40UM}r;VlheyA9e"
    "J`vgP(=s0UE]}VGD75^\"M@.Z;#<DB_cH.r\"_p~'[>#S}z:Wa(v'iDI$Q#Y{g|\"BJ..E!nU~Tqasdasd";

	{
		File file(_file_system, 5);

		Console::puts("Writing into File...\n");

		Console::puts("Write file");

		file.Write(1029, randomString);

		Console::puts("Closing file...\n");
	}

	{
		Console::puts("Opening File again\n");

		File file(_file_system, 5);

		file.Reset();
		char result[1029];

		assert(file.Read(1029, result) == 1029);

		for (int i = 0; i < 1029; i++)
		{
			assert(result[i] == randomString[i]);
		}

		Console::puts("SUCCESS!!\n");

		Console::puts("Closing File again\n");
	}

	Console::puts("Deleting file...\n");

	assert(_file_system->DeleteFile(5));

	assert(_file_system->LookupFile(5) == nullptr);
}


void exercise_file_system(FileSystem* _file_system, unsigned int _iteration_no) {

	const char* STRING1 = "01234567890123456789";
	const char* STRING2 = "abcdefghijabcdefghij";

	/* -- Create two files -- */

	Console::puts("Creating File 1 and File 2\n");

	assert(_file_system->CreateFile(1));
	assert(_file_system->CreateFile(2));

	/* -- "Open" the two files -- */

	{
		Console::puts("Opening File 1 and File 2\n");

		File file1(_file_system, 1);

		File file2(_file_system, 2);

		Console::puts("Writing into File 1 and File 2\n");

		/* -- Write into File 1 -- */
		file1.Write(20, ((_iteration_no % 2 == 0) ? STRING1 : STRING2));

		/* -- Write into File 2 -- */

		file2.Write(20, ((_iteration_no % 2 == 0) ? STRING2 : STRING1));

		/* -- Files will get automatically closed when we leave scope  -- */

		Console::puts("Closing File 1 and File 2\n");
	}

	{
		/* -- "Open files again -- */

		Console::puts("Opening File 1 and File 2 again\n");

		File file1(_file_system, 1);
		File file2(_file_system, 2);

		/* -- Read from File 1 and check result -- */

		Console::puts("Checking content of File 1 and File 2\n");

		file1.Reset();
		char result1[30];
		assert(file1.Read(20, result1) == 20);
		for (int i = 0; i < 20; i++) {
			assert(result1[i] == (_iteration_no % 2 == 0) ? STRING1[i] : STRING2[i]);
		}

		/* -- Read from File 2 and check result -- */
		file2.Reset();
		char result2[30];
		assert(file2.Read(20, result2) == 20);
		for (int i = 0; i < 20; i++) {
			assert(result2[i] == (_iteration_no % 2 == 0) ? STRING2[i] : STRING1[i]);
		}
		Console::puts("SUCCESS!!\n");

		/* -- "Close" files again -- */

		Console::puts("Closing File 1 and File 2 again\n");
	}

	/* -- Delete both files -- */

	Console::puts("Deleting File 1 and File 2\n");

	assert(_file_system->DeleteFile(1));

	assert(_file_system->LookupFile(1) == nullptr);

	assert(_file_system->DeleteFile(2));

	assert(_file_system->LookupFile(2) == nullptr);
}

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

	GDT::init();
	Console::init();
	IDT::init();
	ExceptionHandler::init_dispatcher();
	IRQ::init();
	InterruptHandler::init_dispatcher();

	Console::redirect_output(true);

	/* -- EXAMPLE OF AN EXCEPTION HANDLER -- */

	class DBZ_Handler : public ExceptionHandler {
	public:
		virtual void handle_exception(REGS* _regs) {
			Console::puts("DIVISION BY ZERO!\n");
			for (;;);
		}
	} dbz_handler;

	ExceptionHandler::register_handler(0, &dbz_handler);

	/* -- INITIALIZE MEMORY -- */
	/*    NOTE: We don't have paging enabled in this MP. */
	/*    NOTE2: This is not an exercise in memory management. The implementation
				of the memory management is accordingly *very* primitive! */

				/* ---- Initialize a frame pool; details are in its implementation */
	FramePool system_frame_pool;
	SYSTEM_FRAME_POOL = &system_frame_pool;

	/* ---- Create a memory pool of 256 frames. */
	MemPool memory_pool(SYSTEM_FRAME_POOL, 256);
	MEMORY_POOL = &memory_pool;

	/* -- MEMORY ALLOCATOR SET UP. WE CAN NOW USE NEW/DELETE! -- */

	/* -- INITIALIZE THE TIMER (we use a very simple timer).-- */

	/* Question: Why do we want a timer? This will be used in the IDEController. */

	SimpleTimer timer(100); /* timer ticks every 10ms. */
	InterruptHandler::register_handler(0, &timer);
	/* The Timer is implemented as an interrupt handler. */

	/* -- DISK DEVICE -- */

	IDE_CONTROLLER = new IDEController(&timer); // Our Disk will be accessed through an IDE controller

	SYSTEM_DISK = new SimpleDisk(IDE_CONTROLLER, SYSTEM_DISK_SIZE);

	class Disk_Silencer : public InterruptHandler {
	public:
		virtual void handle_interrupt(REGS* _regs) {
			// Do nothing. We just want to shut up the system complaining about disk interrupts.
		}
	} disk_silencer;

	InterruptHandler::register_handler(14, &disk_silencer);

	/* -- FILE SYSTEM -- */

	FILE_SYSTEM = new FileSystem();

	/* NOTE: The timer chip starts periodically firing as
			 soon as we enable interrupts.
			 It is important to install a timer handler, as we
			 would get a lot of uncaptured interrupts otherwise. */

			 /* -- ENABLE INTERRUPTS -- */

	Machine::enable_interrupts();

	/* -- MOST OF WHAT WE NEED IS SETUP. THE KERNEL CAN START. */

	Console::puts("Hello World!\n");

	/* -- HERE WE STRESS TEST THE FILE SYSTEM -- */

	Console::puts("before formatting...");
	assert(FileSystem::Format(SYSTEM_DISK, (1 MB))); // Don't try this at home!
	Console::puts("formatting completed\n");

	Console::puts("before mounting...");
	assert(FILE_SYSTEM->Mount(SYSTEM_DISK)); // 'connect' disk to file system.
	Console::puts("mounting completed\n");

	for (int j = 0; j < 30; j++) {
		Console::puts("exercise file system; iteration "); Console::puti(j); Console::puts("...\n");
		exercise_file_system(FILE_SYSTEM, j);
		Console::puts("iteration done\n");
	}

	Console::puts("EXCELLENT! Your File system seems to work correctly. Congratulations!!\n");
	/* -- AND ALL THE REST SHOULD FOLLOW ... */

	Console::puts("\n\n\n-.********************************************************.-\n\n\n");

    Console::puts("Now we test with a larger file!\n");

	exercise_larger_file_system(FILE_SYSTEM);

    Console::puts("EXCELLENT! Your File system seems to work correctlyeven with larger files. \n Congratulations!!\n");

	for (;;);

	/* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
	return 1;
}
