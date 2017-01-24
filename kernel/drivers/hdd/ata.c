#include <stdio.h>

#include <kernel/intex.h>
#include <kernel/timer.h>
#include <drivers/ata.h>
#include <drivers/mbr.h>

struct IDEChannelRegisters {
	unsigned short base;
	unsigned short ctrl;
	unsigned short bmide;
	unsigned char nIEN;
} channels[2];

unsigned short ide_buf[1024] = {0};
unsigned char ide_irq_invoked = 0;
unsigned char atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct ide_device {
	unsigned char reserved;
	unsigned char channel; /* Primary(0) or Secondary(1)*/
	unsigned char drive; /* Master(0) or Slave(1)*/
	unsigned short type; /* ATA(0) or ATAPI(0)*/
	unsigned short sig;
	unsigned short capabilities;
	unsigned int command_sets;
	unsigned int size; /* In sectors*/
	unsigned char model[41];
	partition *first_p;
} ide_devices[4];


/* READ AND WRITE TO REGISTERS */

void ide_write(unsigned char channel, unsigned char reg, unsigned char data) {
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
		
	if (reg < 0x08)
		outportb(channels[channel].base + reg - 0x00, data);
	else if (reg < 0x0C)
		outportb(channels[channel].base + reg - 0x06, data);
	else if (reg < 0x0E)
		outportb(channels[channel].ctrl + reg - 0x0A, data);
	else if (reg < 0x16)
		outportb(channels[channel].bmide + reg - 0x0E, data);
		
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

unsigned char ide_read(unsigned char channel, unsigned char reg) {
	unsigned char result = 0;
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
		
	if (reg < 0x08)
		result = inportb(channels[channel].base + reg - 0x00);
	else if (reg < 0x0C)
		result = inportb(channels[channel].base + reg - 0x06);
	else if (reg < 0x0E)
		result = inportb(channels[channel].ctrl + reg - 0x0A);
	else if (reg < 0x16)
		result = inportb(channels[channel].bmide + reg - 0x0E);
		
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
		
	return result;
}

void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned short *buffer, unsigned int words) {
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
		
	if (reg < 0x08) {
		inportsw(channels[channel].base + reg - 0x00, buffer, words);
	} else if (reg < 0x0C)
		inportsw(channels[channel].base + reg - 0x06, buffer, words);
	else if (reg < 0x0E)
		inportsw(channels[channel].ctrl + reg - 0x0A, buffer, words);
	else if (reg < 0x16)
		inportsw(channels[channel].bmide + reg - 0x0E, buffer, words);
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

unsigned char ide_polling(unsigned char channel, unsigned int advanced_check) {

   // (I) Delay 400 nanosecond for BSY to be set:
   // -------------------------------------------------
   ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
   ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
   ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.
   ide_read(channel, ATA_REG_ALTSTATUS); // Reading Alternate Status Port wastes 100ns.

   // (II) Wait for BSY to be cleared:
   // -------------------------------------------------
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait for BSY to be zero.

   if (advanced_check) {

      unsigned char state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.

      // (III) Check For Errors:
      // -------------------------------------------------
      if (state & ATA_SR_ERR) return 2; // Error.

      // (IV) Check If Device fault:
      // -------------------------------------------------
      if (state & ATA_SR_DF ) return 1; // Device Fault.

      // (V) Check DRQ:
      // -------------------------------------------------
      // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
      if (!(state & ATA_SR_DRQ)) return 3; // DRQ should be set

   }

   return 0; // No Error.

}

unsigned char ide_print_error(unsigned int drive, unsigned char err) {
	if (err == 0) return err;
	
	printf("IDE:");
	if (err == 1) {printf("- Device Fault\n\t"); err = 19;}
	else if (err == 2) {
		unsigned char st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);
		if (st & ATA_ER_AMNF)   {printf("- No Address Mark Found\n\t");   err = 7;}
		if (st & ATA_ER_TK0NF)   {printf("- No Media or Media Error\n\t");   err = 3;}
		if (st & ATA_ER_ABRT)   {printf("- Command Aborted\n\t");      err = 20;}
		if (st & ATA_ER_MCR)   {printf("- No Media or Media Error\n\t");   err = 3;}
		if (st & ATA_ER_IDNF)   {printf("- ID mark not Found\n\t");      err = 21;}
		if (st & ATA_ER_MC)   {printf("- No Media or Media Error\n\t");   err = 3;}
		if (st & ATA_ER_UNC)   {printf("- Uncorrectable Data Error\n\t");   err = 22;}
		if (st & ATA_ER_BBK)   {printf("- Bad Sectors\n\t");       err = 13;}
	} else if (err == 3) {printf("- Reads Nothing\n\t"); err = 23;}
	  else if (err == 4) {printf("- Write Protected\n\t"); err = 8;}
	  else if (err == 5) printf("- Out of Partition Bounds\n\t");
	
	printf("- [%s %s] %s\n", 
		(const char *[]){"Primary", "Secondary"}[ide_devices[drive].channel],
		(const char *[]){"Master", "Slave"}[ide_devices[drive].drive],
		ide_devices[drive].model);
	
	return err;
}

/* INITIALIZE IDE FUNCTION */
/*extern int root_dev;
extern int root_partition;*/

void ide_initialize(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4) {
	int i, j, k, count = 0;
	 
	channels[ATA_PRIMARY  ].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
	channels[ATA_PRIMARY  ].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
	channels[ATA_SECONDARY].base  = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
	channels[ATA_SECONDARY].ctrl  = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
	channels[ATA_PRIMARY  ].bmide = (BAR4 & 0xFFFFFFFC) + 0; /* Bus Master IDE*/
	channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; /* Bus Master IDE*/
	
    // Not sure why we do this or if we need to, for now I will disable it
	/*outportb(0x3F6, 0x04);
	outportb(0x376, 0x04);
	sleep(4);
	outportb(0x3F6, 0x00);
	outportb(0x376, 0x00);*/
	

    // Disable IRQs for now
	ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
	ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
	for (i = 0; i < 2; i++) { //i = channel
		for (j = 0; j < 2; j++) { //j = drive
			unsigned char err = 0, type = IDE_ATA, status;
			ide_devices[count].reserved = 0;
			
            // Select the correct drive
			ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
			sleep(1);
			
            // Send identify
			ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			sleep(1);

            // Check the status of IDENTIFY
			if (ide_read(i, ATA_REG_STATUS) == 0) continue; // No device
			
			while(1) {
				status = ide_read(i, ATA_REG_STATUS);
				if ((status & ATA_SR_ERR)) {err = 1; break;} // Not ATA, ATAPI
				if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // All good
			}
			
            // Check if it's actually ATAPI instead of ATA
			if (err != 0) {
				unsigned char cl = ide_read(i, ATA_REG_LBA1);
				unsigned char ch = ide_read(i, ATA_REG_LBA2);
				
				if (cl == 0x14 && ch == 0xEB)
					type = IDE_ATAPI;
				else if (cl == 0x69 && ch == 0x96)
					type = IDE_ATAPI;
				else continue; // Unknown type
				
				ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				sleep(1);
			}
            // Actually read the information
			ide_read_buffer(i, ATA_REG_DATA, ide_buf, 256);
			
			ide_devices[count].reserved = 1;
			ide_devices[count].type = type;
			ide_devices[count].channel = i;
			ide_devices[count].drive = j;
			ide_devices[count].sig = *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE));
			ide_devices[count].capabilities = *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES));
			ide_devices[count].command_sets = *((unsigned int *)(ide_buf + ATA_IDENT_COMMANDSETS));
			
			if (ide_devices[count].command_sets & (1 << 26)) // 48 bit addressing
				ide_devices[count].size = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
			else
				ide_devices[count].size = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA));
				
			for (k = 0; k < 20; k++) {
				unsigned short c = ide_buf[ATA_IDENT_MODEL + k];
				ide_devices[count].model[k*2] = c >> 8;
				ide_devices[count].model[k*2+1] = c & 0xFF;
			}
			ide_devices[count].model[40] = 0;
			
			if (type == IDE_ATA) {
				printf("Reading partitions for device number %d\n", count);
				partition *first_p = read_partitions(count);
				if (first_p == 0) {
                    printf("Error reading partitions, aborting\n");
                    count++;
                    continue;
                }
				ide_devices[count].first_p = first_p;
			}
			
			count++;
		}
	}
	
	// MOUNT sda0p5 to root
	/*partition *current_p = ide_devices[root_dev].first_p;
	while (1) {
		if (current_p->id == root_partition) {
			mount_root(current_p);
			break;
		}
		if (current_p->next == 0) {
			printf("Error: mount_root failed, unable to find partition %d on drive %d\n", root_partition, root_dev);
			break;
		}
		current_p = (partition *)current_p->next;
	}*/
	
    hdd = 0xFF;
	for (i = 0; i < 4; i++) {
		if (ide_devices[i].reserved == 0) continue;
		if (ide_devices[i].type == IDE_ATA) hdd = i;
        
		/*printf("Found %s Drive %dGB - %s\n\t%s %s\n",
			(const char *[]){"ATA", "ATAPI"}[ide_devices[i].type],
			ide_devices[i].size / 1024 / 1024 / 2,
			ide_devices[i].model,
			(const char *[]){"Primary", "Secondary"}[ide_devices[i].channel],
			(const char *[]){"Master", "Slave"}[ide_devices[i].drive]);*/
	}

    //printf("Found HDD at drive %d\n", hdd);
}

partition* get_partition(unsigned char device, unsigned char pnum) {
    partition* cpart = ide_devices[device].first_p;

    while (cpart && cpart->id != pnum) {
        cpart = cpart->next;
    }

    // That partition number does not exist!
    if (!cpart) {
        printf("Error: Requested partition number %d on device %d cannot be found\n", pnum, device);
        return 0;
    }

    return cpart;
}

// direction = 0: reading, direction = 1: writing
// drive is the drive number, 0-3
// lba: LBA address
// selector: segment selector to read from or write to. Right now it doesn't do anything, though maybe when
// the switch to userspace happens it will be needed
// pointer to the buffer to read/write from/to
unsigned char ide_ata_access(unsigned char direction, unsigned char drive, unsigned int lba, unsigned char numsects, unsigned short selector, unsigned short* buffer) {
    unsigned char lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
    unsigned char lba_io[6];
    unsigned int  channel      = ide_devices[drive].channel; // Read the Channel.
    unsigned int  slavebit      = ide_devices[drive].drive; // Read the Drive [Master/Slave]
    unsigned int  bus      = channels[channel].base; // The Bus Base, like [0x1F0] which is also data port.
    unsigned int  words      = 256; // Approximatly all ATA-Drives has sector-size of 512-byte.
    unsigned short cyl, i; unsigned char head, sect, err;

    // Disable IRQs (for now...)
    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = (ide_irq_invoked = 0x0) + 0x02);

    // (I) Select one from LBA28, LBA48 or CHS;
    if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are giving a wrong LBA.
      // LBA48:
        lba_mode  = 2;
        lba_io[0] = (lba & 0x000000FF)>> 0;
        lba_io[1] = (lba & 0x0000FF00)>> 8;
        lba_io[2] = (lba & 0x00FF0000)>>16;
        lba_io[3] = (lba & 0xFF000000)>>24;
        lba_io[4] = 0; // We said that we lba is integer, so 32-bit are enough to access 2TB.
        lba_io[5] = 0; // We said that we lba is integer, so 32-bit are enough to access 2TB.
        head      = 0; // Lower 4-bits of HDDEVSEL are not used here.
    } else if (ide_devices[drive].capabilities & 0x200)  { // Drive supports LBA?
      // LBA28:
        lba_mode  = 1;
        lba_io[0] = (lba & 0x00000FF)>> 0;
        lba_io[1] = (lba & 0x000FF00)>> 8;
        lba_io[2] = (lba & 0x0FF0000)>>16;
        lba_io[3] = 0; // These Registers are not used here.
        lba_io[4] = 0; // These Registers are not used here.
        lba_io[5] = 0; // These Registers are not used here.
        head      = (lba & 0xF000000)>>24;
    } else {
      // CHS:
        lba_mode  = 0;
        sect      = (lba % 63) + 1;
        cyl       = (lba + 1  - sect)/(16*63);
        lba_io[0] = sect;
        lba_io[1] = (cyl>>0) & 0xFF;
        lba_io[2] = (cyl>>8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head      = (lba + 1  - sect)%(16*63)/(63); // Head number is written to HDDEVSEL lower 4-bits.
    }

    // (II) See if Drive Supports DMA or not;
    dma = 0; // Supports or doesn't, we don't support !!

     // (III) Wait if the drive is busy;
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait if Busy.

    // (IV) Select Drive from the controller;
    if (lba_mode == 0) ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit<<4) | head);   // Select Drive CHS.
    else         ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit<<4) | head);   // Select Drive LBA.

    // (V) Write Parameters;
    if (lba_mode == 2) {
        ide_write(channel, ATA_REG_SECCOUNT1,   0);
        ide_write(channel, ATA_REG_LBA3,   lba_io[3]);
        ide_write(channel, ATA_REG_LBA4,   lba_io[4]);
        ide_write(channel, ATA_REG_LBA5,   lba_io[5]);
    }
    ide_write(channel, ATA_REG_SECCOUNT0,   numsects);
    ide_write(channel, ATA_REG_LBA0,   lba_io[0]);
    ide_write(channel, ATA_REG_LBA1,   lba_io[1]);
    ide_write(channel, ATA_REG_LBA2,   lba_io[2]);

    // (VI) Select the command and send it;
    // These were not implemented in the tutorial, and for now it doesn't really matter
    // Routine that is followed:
    // If ( DMA & LBA48)   DO_DMA_EXT;
    // If ( DMA & LBA28)   DO_DMA_LBA;
    // If ( DMA & LBA28)   DO_DMA_CHS;
    // If (!DMA & LBA48)   DO_PIO_EXT;
    // If (!DMA & LBA28)   DO_PIO_LBA;
    // If (!DMA & !LBA#)   DO_PIO_CHS;
    
    if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;   
    if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;   
    if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
    if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
    ide_write(channel, ATA_REG_COMMAND, cmd);               // Send the Command.

    // Now do the actual read or write:
    if (dma) {
        if (direction == 0) {}
        // DMA Read.
        else {}; // DMA Write.
    } else {
        if (direction == 0) {
            // PIO Read.
            for (int j = 0; j < numsects; j++) {
                err = ide_polling(channel, 1);

                //printf("j: %d, numsects: %d, buffer: %x, ", j, numsects, buffer);

                if (err) return err; // Polling, then set error and exit if there is.
                //asm("pushw %es" :);
                //asm("mov %%ax, %%es"::"a"(selector));
                __asm__ __volatile__("rep insw"::"c"(words), "d"(bus), "D"(buffer)); // Receive Data.
                //asm("popw %es" :);
                //printf("buffer: %x\n", buffer);
                //buffer += words; // I'm pretty sure we don't actually need this, the asm takes care of it
            }
        } else {
            // PIO Write.
            for (i = 0; i < numsects; i++) {
                ide_polling(channel, 0); // Polling.
                //asm("pushw %%ds");
                //asm("mov %%ax, %%ds"::"a"(selector));
                asm("rep outsw"::"c"(words), "d"(bus), "S"(buffer)); // Send Data 
                //asm("popw %%ds");
                //buffer += words; // I'm pretty sure we don't actually need this, the asm takes care of it
            }
            ide_write(channel, ATA_REG_COMMAND, (char []) {   ATA_CMD_CACHE_FLUSH,
                            ATA_CMD_CACHE_FLUSH,
                            ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
            ide_polling(channel, 0); // Polling.
        }
    }
    return 0; // Easy, ... Isn't it?
}

unsigned char ide_read_sectors(unsigned char drive, unsigned char numsects, unsigned int lba, unsigned short es, unsigned short* buffer) {

   // 1: Check if the drive presents:
   // ==================================
   if (drive > 3 || ide_devices[drive].reserved == 0) return 1;      // Drive Not Found!

   // 2: Check if inputs are valid:
   // ==================================
   else if (((lba + numsects) > ide_devices[drive].size) && (ide_devices[drive].type == IDE_ATA))
      return 2;                     // Seeking to invalid position.

   // 3: Read in PIO Mode through Polling & IRQs:
   // ============================================
   else {
      unsigned char err;
      if (ide_devices[drive].type == IDE_ATA) {
         err = ide_ata_access(ATA_READ, drive, lba, numsects, es, buffer);
      } else if (ide_devices[drive].type == IDE_ATAPI) {
          printf("Unsupported operation, read/write from/to ATAPI device\n");
          err = 4;
         /*for (i = 0; i < numsects; i++)
            err = ide_atapi_read(drive, lba + i, 1, es, edi + (i*2048));*/
      }
      return ide_print_error(drive, err);
   }
}

unsigned char ide_write_sectors(unsigned char drive, unsigned char numsects, unsigned int lba, unsigned short es, unsigned short* buffer) {

   // 1: Check if the drive presents:
   // ==================================
   if (drive > 3 || ide_devices[drive].reserved == 0) return 1;      // Drive Not Found!
   // 2: Check if inputs are valid:
   // ==================================
   else if (((lba + numsects) > ide_devices[drive].size) && (ide_devices[drive].type == IDE_ATA))
      return 2;                     // Seeking to invalid position.
   // 3: Read in PIO Mode through Polling & IRQs:
   // ============================================
   else {
      unsigned char err;
      if (ide_devices[drive].type == IDE_ATA)
         err = ide_ata_access(ATA_WRITE, drive, lba, numsects, es, buffer);
      else if (ide_devices[drive].type == IDE_ATAPI)
         err = 4; // Write-Protected. Not yet supported
      return ide_print_error(drive, err);
   }
}

// This is all old stuff, from the first version of the operating system. I believe it should work, but the coe above will definitely work.
/*extern unsigned int partition_sector;
extern unsigned int partition_length;

unsigned char ide_read_sectors(int drive, unsigned char sectorcount, unsigned int lba, unsigned short *buffer) { // 28 bit PIO
	unsigned char c = ide_devices[drive].channel;
	ide_write(c, ATA_REG_HDDEVSEL, 0xE0 | (ide_devices[drive].drive << 4) | ((lba >> 24) & 0x0F));
	ide_write(c, ATA_REG_FEATURES, 0);
	
	//if (lba > partition_length) return 5;
	//lba += partition_sector;
	//printf("drive: %d, sectors: %d, lba: %d\n", drive, sectorcount, lba);
	// Setup position and length
	ide_write(c, ATA_REG_SECCOUNT0, sectorcount);
	ide_write(c, ATA_REG_LBA0, (unsigned char)lba);
	ide_write(c, ATA_REG_LBA1, (unsigned char)(lba >> 8));
	ide_write(c, ATA_REG_LBA2, (unsigned char)(lba >> 16));
	
	// Send read command
	ide_write(c, ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	
	for (int i = 0; i < sectorcount; i++) {
		//printf("reading sector %d\n", i);
		unsigned char err = ide_polling(c, 1);
		if (err > 0) return err;
		__asm__ __volatile__ ("rep insw" : : "c" (256), "d" (channels[c].base), "D" (buffer));
		*//*for (int j = 0; j < 512; j++) {
			printf("%x ", ((unsigned char *)buffer)[j]);
			sleep(1);
		}
		printf("\n\n\n\n\n");*/
		//buffer += 64;
/*	}
	return 0;
}
	
unsigned char ide_write_sectors(int drive, unsigned char sectorcount, unsigned int lba, unsigned short *buffer) { // 28 bit PIO
	unsigned char c = ide_devices[drive].channel;
	ide_write(c, ATA_REG_HDDEVSEL, 0xE0 | (ide_devices[drive].drive << 4) | ((lba >> 24) & 0x0F));
	ide_write(c, ATA_REG_FEATURES, 0);
	
	//if (lba > partition_length) return 5;
	//lba += partition_sector;
	
	// Setup position and length
	ide_write(c, ATA_REG_SECCOUNT0, sectorcount);
	ide_write(c, ATA_REG_LBA0, (unsigned char)lba);
	ide_write(c, ATA_REG_LBA1, (unsigned char)(lba >> 8));
	ide_write(c, ATA_REG_LBA2, (unsigned char)(lba >> 16));
	
	// Send write command
	ide_write(c, ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
	
	for (int i = 0; i < sectorcount; i++) {
		unsigned char err = ide_polling(c, 1);
		if (err > 0) return err;
		outportsw(channels[c].base, buffer, 256);
		//printf("(to check if buffer is advancing) buffer is %d\n", buffer);
		buffer += 256;
	}
	ide_write(c, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
	ide_polling(c, 0);
	return 0;
}*/
