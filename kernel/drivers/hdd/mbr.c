#include <stdio.h>
#include <string.h>

#include <kernel/kmalloc.h>
#include <drivers/mbr.h>
#include <drivers/ata.h>
#include <kernel/timer.h>

typedef struct {
	unsigned char boot_flag;
	unsigned char s_head;
	unsigned char s_sector;
	unsigned char s_cylinder;
	unsigned char sysid;
	unsigned char e_head;
	unsigned char e_sector;
	unsigned char e_cylinder;
	unsigned int lba;
	unsigned int sectors;
} __attribute__((packed)) pt_entry;

typedef struct {
	unsigned char bootstrap[436];
	unsigned char diskID[10];
	pt_entry p1;
	pt_entry p2;
	pt_entry p3;
	pt_entry p4;
	unsigned short sig; // Should be (0x55, 0xAA)
} __attribute__((packed)) mbr_table;

partition* read_partitions(unsigned char device) {
	mbr_table *mbr = kmalloc(sizeof(mbr_table));
    if (!mbr) {
        printf("Error allocating space for the mbr, returning\n");
        return 0;
    }

	unsigned char err;
	err = ide_read_sectors(device, 1, 0, 0, (unsigned short *)mbr);
	if (err > 0) {ide_print_error(device, err); return 0;}
	
	/*printf("Partition 1 at %d, length: %d sectors, system id: %x, bootflag: %x\n\
Partition 2 at %d, length: %d sectors, system id: %x, bootflag: %x\n\
Partition 3 at %d, length: %d sectors, system id: %x, bootflag: %x\n\
Partition 4 at %d, length: %d sectors, system id: %x, bootflag: %x\n",
		mbr->p1.lba, mbr->p1.sectors, mbr->p1.sysid, mbr->p1.boot_flag,
		mbr->p2.lba, mbr->p2.sectors, mbr->p2.sysid, mbr->p2.boot_flag,
		mbr->p3.lba, mbr->p3.sectors, mbr->p3.sysid, mbr->p3.boot_flag,
		mbr->p4.lba, mbr->p4.sectors, mbr->p4.sysid, mbr->p4.boot_flag);*/
		
	partition *current_p = 0;
	partition *first_p = 0;
	unsigned char first = 1;
	if (mbr->p1.sectors) {
		partition *p = kmalloc(sizeof(partition));
		p->id = 1;
		p->type = mbr->p1.sysid;
		p->drive = device;
		p->sector = mbr->p1.lba;
		p->length = mbr->p1.sectors;
		p->next = 0;
		
		if (first) {
			first_p = current_p = p;
			first = 0;
		} else {
			current_p->next = p;
			current_p = p;
		}
	}
	
	if (mbr->p2.sectors) {
		partition *p = kmalloc(sizeof(partition));
		p->id = 2;
		p->type = mbr->p2.sysid;
		p->drive = device;
		p->sector = mbr->p2.lba;
		p->length = mbr->p2.sectors;
		p->next = 0;
		
		if (first) {
			first_p = current_p = p;
			first = 0;
		} else {
			current_p->next = p;
			current_p = p;
		}
	}
	
	if (mbr->p3.sectors) {
		partition *p = kmalloc(sizeof(partition));
		p->id = 3;
		p->type = mbr->p3.sysid;
		p->drive = device;
		p->sector = mbr->p3.lba;
		p->length = mbr->p3.sectors;
		p->next = 0;
		
		if (first) {
			first_p = current_p = p;
			first = 0;
		} else {
			current_p->next = p;
			current_p = p;
		}
	}
	
	if (mbr->p4.sectors) {
		partition *p = kmalloc(sizeof(partition));
		p->id = 4;
		p->type = mbr->p4.sysid;
		p->drive = device;
		p->sector = mbr->p4.lba;
		p->length = mbr->p4.sectors;
		p->next = 0;
		
		if (first) {
			first_p = current_p = p;
			first = 0;
		} else {
			current_p->next = p;
			current_p = p;
		}
	}
	
	pt_entry ep;
	int ep_used = 0;
	if (mbr->p1.sysid == 0x05 || mbr->p1.sysid == 0x0F) {ep = mbr->p1; ep_used = 1;}
	else if (mbr->p2.sysid == 0x05 || mbr->p2.sysid == 0x0F) {ep = mbr->p2; ep_used = 1;}
	else if (mbr->p3.sysid == 0x05 || mbr->p3.sysid == 0x0F) {ep = mbr->p3; ep_used = 1;}
	else if (mbr->p4.sysid == 0x05 || mbr->p4.sysid == 0x0F) {ep = mbr->p4; ep_used = 1;}
	
	if (ep_used) {
		unsigned char count = 5;
		unsigned int starting_ep;
		unsigned int lba = starting_ep = ep.lba;
		while (1) {
			memset(mbr, 0, 512);
			err = ide_read_sectors(device, 1, lba, 0, (unsigned short *)mbr);
			if (err > 0) {ide_print_error(device, err); return 0;}
			
			partition *p = kmalloc(sizeof(partition));
			p->id = count;
			p->type = mbr->p1.sysid;
			p->drive = device;
			p->sector = mbr->p1.lba+starting_ep;
			p->length = mbr->p1.sectors;
			p->next = 0;
			current_p->next = p;
			current_p = p;
			
			printf("Partition %d at %d, length: %d sectors, system id: %x, bootflag: %x\n",
				count, mbr->p1.lba+starting_ep, mbr->p1.sectors, mbr->p1.sysid, mbr->p1.boot_flag);
				
			count++;
			if (mbr->p2.lba) {
				lba = starting_ep + mbr->p2.lba;
			} else {
				break;
			}
		}
	}
	kfree(mbr);
	return first_p;
}
