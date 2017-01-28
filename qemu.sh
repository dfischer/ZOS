#!/bin/sh
set -e
. ./iso.sh
 
qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 20M -hda hdd.img -cdrom myos.iso -boot d
