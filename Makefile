.PHONY: all run clean

KERNEL := src/pmodekernel/kernel
BOOT := src/pmodekernel/boot
TMP := tmp
OUT := target

all: $(OUT)/floppy.img

run: $(OUT)/floppy.img
	qemu-system-i386 -fda $(OUT)/floppy.img

$(OUT)/floppy.img: $(TMP)/boot.bin $(TMP)/kernel.bin | $(OUT)
	dd if=/dev/zero of=$(OUT)/floppy.img bs=512 count=2880
	dd if=$(TMP)/boot.bin of=$(OUT)/floppy.img conv=notrunc
	dd if=$(TMP)/kernel.bin of=$(OUT)/floppy.img bs=512 seek=1 conv=notrunc

$(TMP)/boot.bin: $(BOOT)/boot.asm | $(TMP)
	nasm -f bin $(BOOT)/boot.asm -o $(TMP)/boot.bin

$(TMP)/kernel.bin: $(TMP)/kernel.elf
	objcopy -O binary $(TMP)/kernel.elf $(TMP)/kernel.bin

$(TMP)/kernel.elf: $(TMP)/kernel.o $(TMP)/kmemmgt.o $(KERNEL)/linker.ld
	ld -m elf_i386 -T $(KERNEL)/linker.ld -o $(TMP)/kernel.elf $(TMP)/kernel.o $(TMP)/kmemmgt.o

$(TMP)/kernel.o: $(KERNEL)/kernel.c | $(TMP)
	gcc -m32 -ffreestanding -fno-stack-protector -c $< -o $@

$(TMP)/kmemmgt.o: $(KERNEL)/kmemmgt.c | $(TMP)
	gcc -m32 -ffreestanding -fno-stack-protector -c $< -o $@

$(TMP):
	mkdir -p $(TMP)

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -rf $(TMP) $(OUT)