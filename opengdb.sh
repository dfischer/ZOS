#!/bin/sh

gdb -tui -ex "target remote localhost:1234" -ex "symbol-file isodir/boot/myos.kernel" -ex "layout split"
