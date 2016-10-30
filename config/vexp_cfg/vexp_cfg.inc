GNU_TOOLS 	= arm-none-eabi
ARCH		= arm/armv7
CFG_TARGET	= raspi
MACH		= raspi
ARCH_AFLAGS = -mfloat-abi=hard -march=armv7-a -mfpu=vfpv3 -mcpu=cortex-a9
ARCH_CFLAGS = -mfloat-abi=hard -march=armv7-a -mfpu=vfpv3 -mtune=cortex-a9 -mcpu=cortex-a9
