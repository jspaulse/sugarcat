GNU_TOOLS 	= arm-none-eabi
ARCH		= armv6
MACH		= raspi
ARCH_AFLAGS = -march=armv6 -mcpu=arm1176jzf-s -mfpu=vfp
ARCH_CFLAGS = -mhard-float -march=armv6 -mfpu=vfp
