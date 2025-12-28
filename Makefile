



GPPPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = loader.o kernel.o

%.o: %.cpp
		g++ $(GPPPARAMS) -o $@ -c $<


%.o: %.s
		as $(ASPARAMS) -o $@ $<

# ld $(LDPARAMS) -T $< -o $@ $(objects)

mykernel.bin: linker.ld $(objects)
		i686-elf-ld -T linker.ld -o mykernel.bin loader.o kernel.o


install: mykernel.bin sudo cp $< /boot/mykernel.bin