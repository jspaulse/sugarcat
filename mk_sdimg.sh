#!/bin/sh
FILE="sd.bin"
F_SZ=64M
BOOT_SZ=16MiB

# create the file
qemu-img create -f raw $FILE $F_SZ

# mount it
losetup -f -P $FILE

# grab the device file
LOOP_DEV=$(losetup --raw | grep $FILE | cut -d ' ' -f 1)

# partition it
sudo parted $LOOP_DEV mklabel msdos
sudo parted $LOOP_DEV mkpart primary fat16 1MiB $BOOT_SZ
sudo parted $LOOP_DEV mkpart primary ext2 $BOOT_SZ 100%

#format it
sudo mkfs.fat -F 16 $LOOP_DEV"p1"
sudo mkfs.ext2 $LOOP_DEV"p2"

# umount it
losetup -d $LOOP_DEV

