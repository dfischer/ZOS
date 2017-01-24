#!/bin/sh
set -e
. ./iso.sh
 
qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -m 20M -hda image.img -cdrom myos.iso -boot d
