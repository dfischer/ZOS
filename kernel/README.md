# Kernel

## Folder information

* arch - includes the architecture-dependant code in theory. In reality it's a little mixed up
* drivers - includes some of the clear cut drivers, such as the fat32 driver and the ata driver
* kernel - includes the main files of the operating system
* include - all of the .h files 

## Arch/i386

All code that relies specifically on the i386 architecture.

The bootloader (GRUB) hands off execution to the kernel at the start label in boot2.asm. ASM is used because some of the important code like the long jump wasn't working out as well in GAS. boot2.asm first loads the trick GDT, which has a base of 0x40000000. This code is linked at 1MB, so it can run without any issues. The rest of the code and data is linked at 2GB, so after the trick GDT is loaded everything will work out. The code is linked at 0xC0000000 (2GB), so after the trick GDT is loaded and memory past 0xC0000000 is referenced, like the location of the VGA buffer, 0x40000000 will be added to the address which puts it around 1MB, which is the correct linear (physical) address.

So after loading the "trick" GDT, combined with the way the linker is setup, everything can run OK. In particular, we can then enable paging, which maps the physical addresses around 1MB to virtual addresses around 0xC0000000. After this point we can reload the GDT with entries that have a base of 0x00000000. After this, paging is enabled and used to map addresses correctly instead of the GDT trick. Either way, the result is the same: at this point the kernel looks like it is running at 0xC0000000, and a page directory is set up to continue mapping memory