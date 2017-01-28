# Kernel/kernel

These files control the kernel operation directoy

## File information

Most of the files are self explanatory based on OSDev pages. Some exceptions:

* isrs.c - In an older version isrs and irqs were separated, but now they're both in irqs. isrs.c isn't even built anymore
* misc.c - just a never ending for loop, for the syscall!
* read_eld - Again, at first this was used but now all of the functionality is in task.c. One thing this is still good for is reading the program and section headers from the ELF file.
* task.c - This is one of larger files, and much of it is commented out. I believe it is now in a good state execpt that the permissions on the pages should be though of and changed. Almost all the permissions should be changed to | 3, i.e. not user readable or writable, but you should check that everything still works after this
* terminal.c - This was something I started, but now I would rather use this as a user space program based on syscalls. You should move this into its own project/program!
* vfs.c - This is still somewhat of a work in progress because there is only read functionality