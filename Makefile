TARGET = osiris_os
ISO = $(TARGET).iso
QEMU = qemu-system-x86_64

CC = gcc
LD = ld
AS = nasm
CFLAGS = -ffreestanding -m32 -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -m elf_i386

all: $(ISO)

# Compile the boot assembly file
boot.o: boot/boot.asm
	$(AS) -f elf32 boot/boot.asm -o boot.o

# Compile the kernel C file
kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o kernel.o

# Create the binary from object files
kernel.bin: boot.o kernel.o
	$(LD) $(LDFLAGS) -o kernel.bin boot.o kernel.o

# Create ISO image
$(ISO): kernel.bin
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	echo 'set timeout=0' > isodir/boot/grub/grub.cfg
	echo 'set default=0' >> isodir/boot/grub/grub.cfg
	echo 'menuentry "O.S.I.R.I.S" {' >> isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> isodir/boot/grub/grub.cfg
	echo '  boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) isodir

# Clean up object files and binaries
clean:
	rm -rf *.o *.bin isodir *.iso

# Run the ISO with QEMU
run: $(ISO)
	$(QEMU) -cdrom $(ISO)

# Clean, rebuild, and run the project
test: clean all run