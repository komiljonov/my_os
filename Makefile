# Tools
CC       = gcc
AS       = as
LD       = ld

# Flags
GCCPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions \
            -fno-leading-underscore -Wno-write-strings -IInclude
ASPARAMS  = --32
LDPARAMS  = -melf_i386

# Paths
SRCDIR  = src
INCDIR  = Include
BUILDDIR= build

# Output names
KERNEL_BIN = mykernel.bin
ISO        = mykernel.iso

# Source files (adjust if you add/remove files)
CPP_SOURCES = loader.cpp \
 	        common.cpp \
            gdt.cpp \
            drivers/driver.cpp \
            hardwarecommunication/port.cpp \
            hardwarecommunication/interrupts.cpp \
            hardwarecommunication/pci.cpp \
            hardwarecommunication/pit.cpp \
            drivers/keyboard.cpp \
            drivers/mouse.cpp \
            drivers/vga.cpp \
            timer.cpp \
            kernel.cpp

ASM_SOURCES = hardwarecommunication/interruptstubs.s

# Derived lists
CPP_OBJS = $(CPP_SOURCES:%.cpp=$(BUILDDIR)/%.o)
ASM_OBJS = $(ASM_SOURCES:%.s=$(BUILDDIR)/%.o)
OBJECTS  = $(CPP_OBJS) $(ASM_OBJS)

# Default target: running `make` builds the ISO
all: $(ISO)

run: $(ISO)
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'My Operating System' &

# Ensure build dir exists
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile C++ from src/ to build/
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	mkdir -p $(dir $@)
	$(CC) $(GCCPARAMS) -c -o $@ $<

# Assemble from src/ to build/
$(BUILDDIR)/%.o: $(SRCDIR)/%.s | $(BUILDDIR)
	mkdir -p $(dir $@)
	$(AS) $(ASPARAMS) -o $@ $<

# Link kernel
$(KERNEL_BIN): linker.ld $(OBJECTS)
	$(LD) $(LDPARAMS) -T $< -o $@ $(OBJECTS)

# Create ISO (GRUB2)
$(ISO): $(KERNEL_BIN)
	rm -rf iso
	mkdir -p iso/boot/grub
	cp $(KERNEL_BIN) iso/boot/mykernel.bin
	printf "set timeout=0\nset default=0\n\nmenuentry \"My Operating System\" {\n  multiboot /boot/mykernel.bin\n  boot\n}\n" > iso/boot/grub/grub.cfg
	grub-mkrescue --output=$(ISO) iso
	rm -rf iso

install: $(KERNEL_BIN)
	sudo cp $< /boot/mykernel.bin

.PHONY: all run install clean
clean:
	rm -rf $(BUILDDIR) iso
	rm -f $(KERNEL_BIN) $(ISO)
