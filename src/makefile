AS=nasm
PLATFORM=$(shell uname)
$(info $(PLATFORM))

ifeq ($(PLATFORM), Linux)
GCC=x86_64-linux-gnu-gcc
LD=x86_64-linux-gnu-ld
endif

ifeq ($(PLATFORM), Darwin)
GCC=x86_64-elf-gcc
LD=x86_64-elf-ld
endif

GCC_OPTIONS = -m32 -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector -fleading-underscore -fno-asynchronous-unwind-tables -fno-pie

all: kernel.bin

clean:
	rm -f *.o *.bin

run:
	qemu-system-x86_64 -kernel kernel.bin -serial stdio

debug:
	qemu-system-x86_64 -s -S -kernel kernel.bin

# ==== KERNEL ENTRY POINT ====

start.o: start.asm 
	$(AS) -f elf -o start.o start.asm

utils.o: utils.C utils.H
	$(GCC) $(GCC_OPTIONS) -c -o utils.o utils.C

assert.o: assert.C assert.H
	$(GCC) $(GCC_OPTIONS) -c -o assert.o assert.C

# ==== VARIOUS LOW-LEVEL STUFF =====

machine.o: machine.C machine.H
	$(GCC) $(GCC_OPTIONS) -c -o machine.o machine.C

machine_low.o: machine_low.asm machine_low.H
	$(AS) -f elf -o machine_low.o machine_low.asm

# ==== DEVICES =====

console.o: console.C console.H
	$(GCC) $(GCC_OPTIONS) -c -o console.o console.C

# ==== MEMORY =====

cont_frame_pool.o: cont_frame_pool.C cont_frame_pool.H
	$(GCC) $(GCC_OPTIONS) -c -o cont_frame_pool.o cont_frame_pool.C

# ==== KERNEL MAIN FILE =====

kernel.o: kernel.C console.H 
	$(GCC) $(GCC_OPTIONS) -c -o kernel.o kernel.C

kernel.bin: start.o utils.o kernel.o assert.o console.o \
   cont_frame_pool.o machine.o machine_low.o  
	$(LD) -melf_i386 -T linker.ld -o kernel.bin start.o utils.o \
   kernel.o assert.o console.o \
   cont_frame_pool.o  machine.o machine_low.o 
