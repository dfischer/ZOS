#!/bin/bash

if [[ $(/usr/bin/id -u) -ne 0 ]]; then
    echo "Please run this script with sudo"
    exit 1
fi

# Make a blank image
dd if=/dev/zero of=hdd.img iflag=fullblock bs=1M count=100 && sync

# Add the image to the loopback
losetup /dev/loop1 hdd.img

# Write partition table
parted /dev/loop1 mklabel msdos

parted /dev/loop1 mkpart primary fat32 1 50
parted /dev/loop1 mkpart primary fat32 50 100

losetup -d /dev/loop1
losetup /dev/loop1 hdd.img

# make filesystem
#parted /dev/loop1 mkpartfs primary fat32 20 80
sudo mkdosfs -F 32 -f 2 /dev/loop1p1

# remove the image from the loopback
losetup -d /dev/loop1
