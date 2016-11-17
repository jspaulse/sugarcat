GNU_TOOLS 	= arm-none-eabi
ARCH		= arm/armv7
CPU			= cortex_a9
MACH		= vexpress_a9
ARCH_AFLAGS = -mfloat-abi=hard -march=armv7-a -mfpu=vfpv3 -mcpu=cortex-a9
ARCH_CFLAGS = -mfloat-abi=hard -march=armv7-a -mfpu=vfpv3 -mtune=cortex-a9 -mcpu=cortex-a9
