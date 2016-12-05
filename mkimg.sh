#!/bin/sh

#move .img to temp .bin
mv -f kernel.img kernel.tmp

# make image
mkimage -A arm -T kernel -C none -a 0x60008000 -e 0x60008000 -d kernel.tmp kernel.img

# remove old
rm -f kernel.tmp
