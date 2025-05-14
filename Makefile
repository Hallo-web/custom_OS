
TARGET = voidblade_os
ISO = $(TARGET).iso

CC = gcc
LD = ld
AS = nasm
CFLAGS = -ffreestanding -m32 -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -m elf_i386

all: $(ISO)

boot.o: boot/boot.asm
	$(AS) -f elf32 boot/boot.asm -o boot.o

kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o kernel.o

kernel.bin: boot.o kernel.o
	$(LD) $(LDFLAGS) -o kernel.bin boot.o kernel.o

$(ISO): kernel.bin
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	echo 'set timeout=0' > isodir/boot/grub/grub.cfg
	echo 'set default=0' >> isodir/boot/grub/grub.cfg
	echo 'menuentry "VoidBlade OS" {' >> isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> isodir/boot/grub/grub.cfg
	echo '  boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	xorriso -as mkisofs -R -b boot/grub/stage2_eltorito \
	  -no-emul-boot -boot-load-size 4 -boot-info-table \
	  -o $(ISO) isodir

clean:
	rm -rf *.o *.bin isodir *.iso
