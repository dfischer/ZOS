#!/bin/bash
set -e
. ./build.sh

if [[ $(/usr/bin/id -u) -ne 0 ]]; then
    echo "Please run this script with sudo"
    exit 1
fi

mkdir -p sysroot/boot
mkdir -p sysroot/boot/grub
 
cat > sysroot/boot/grub/grub.cfg << EOF
menuentry "myos" {
    multiboot /boot/myos.kernel
}
EOF

#cat > sysroot/boot/grub/device.map <<EOF
#(hd0)   /dev/loop1
#(hd0,1) /dev/loop1p1
#EOF


. ./clean_image.sh

losetup /dev/loop1 hdd.img
losetup /dev/loop2 hdd.img -o 1048576

mkdir -p /media/zack/hdd_img
mount /dev/loop2 /media/zack/hdd_img

cp -r sysroot/* /media/zack/hdd_img
cp -r test_prog/* /media/zack/hdd_img
grub-install --no-floppy --force --modules="normal part_msdos multiboot" --root-directory=/media/zack/hdd_img /dev/loop1

umount /media/zack/hdd_img

losetup -d /dev/loop1
losetup -d /dev/loop2

