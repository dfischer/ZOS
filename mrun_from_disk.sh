#!/bin/bash
set -e
. ./img.sh
 
qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 20M -hda hdd.img
