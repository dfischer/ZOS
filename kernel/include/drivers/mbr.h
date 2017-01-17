#ifndef _DRIVERS_MBR_H
#define _DRIVERS_MBR_H

typedef struct partitionn {
    unsigned char id;
    unsigned char type;
    unsigned char drive;
    unsigned int sector;
    unsigned int length;
    struct partitionn* next;
} partition;

partition* read_partitions(unsigned char device);

#endif
