# Drivers

This contains some of the more specific hardware drivers so far.

## File information

* fat32_2.c - This replaces fat32.c, which was used in an earlier version. fat32.c can really be deleted now, if you want. This controls reading and writing to FAT32 file systems, though as of now only reading is supported
* hdd/mbr.c - This reads partitions from an MBR, and stores information about them
* hdd/ata.c - This deals with the actual reading and writing to a connected device. This was largely taken from a IDE_ATA tutorial, so if you want more information look that up.