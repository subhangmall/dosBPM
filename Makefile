# Default behaviour for the command 'make': create img if something changed.
all: ./target/floppy.img

# Command to build and run with QEMU the operating system.
run: all
	qemu-system-i386 -fda ./target/floppy.img -d int,cpu_reset -no-reboot -D ./target/floppy.log

# Build and debug os with QEMU and GDB; load .gdb config and scripts.
dbg: all
	echo "\
\nset disassembly-flavor intel\
\ntarget remote | qemu-system-i386 -fda ./target/floppy.img -d int,cpu_reset -no-reboot -D ./target/floppy.log -S -gdb stdio\
\nlayout asm\
\nb *0x7c00\
\nc\
\nx/16xh 0x7dfe" > ./target/debug-script.gdb
	gdb -tui -x ./target/debug-script.gdb

# In order to create the .img file, the .bin file must be created first.
./target/floppy.img: ./target/main.bin
	cp ./target/main.bin ./target/floppy.img
	truncate -s 1440k ./target/floppy.img

# In order to create the .bin file, the .asm file must be modified.
./target/main.bin: ./target ./src/main.asm
	nasm -f bin -o ./target/main.bin ./src/main.asm

# Ensure the target directory exists
./target:
	mkdir -p ./target

# No requirement needed to run 'make clean': just delete ./target/*
clean:
	rm ./target/*
