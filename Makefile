CURR_DIR 	:= $(realpath .)/
CONFIG 		?= raspi.inc
include 	$(CONFIG)
ARCH 		?= armv6
GNU_TOOLS	?= arm-none-eabi
BUILD		= build/
FBUILD		= $(CURR_DIR)$(BUILD)
SOURCE		= source/
INCLUDE		= $(CURR_DIR)include/
LIBS		=
LINKER		= kernel.ld
TARGET		= kernel.img
LIST		= kernel.list
MAP			= kernel.map
READELF		= kernel.rf
CFLAGS 		= -I$(INCLUDE) -std=gnu99 -O2 -Wall -Werror -Wextra -Wshadow \
			-nostdlib -nostartfiles -ffreestanding \
			-pedantic -pedantic-errors $(ARCH_CFLAGS)
AFLAGS		= --warn --fatal-warnings -I $(INCLUDE) $(ARCH_AFLAGS)

PASS_FLAGS	= ARCH=$(ARCH) BUILD=$(FBUILD) ARCH_AFLAGS='$(ARCH_AFLAGS)'
PASS_FLAGS	+= ARCH_CFLAGS='$(ARCH_CFLAGS)' GNU_TOOLS=$(GNU_TOOLS)
PASS_FLAGS	+= INCLUDE='$(INCLUDE)'

all:
	$(MAKE) -C $(SOURCE)util $(PASS_FLAGS)
	$(MAKE) raspi
	
raspi:
	$(MAKE) -C $(SOURCE)arch/arm $(PASS_FLAGS)
	$(MAKE) base

base:
	$(MAKE) -C $(SOURCE)kernel/ $(PASS_FLAGS)
	$(MAKE) -C $(SOURCE)kernel/mm/ $(PASS_FLAGS)
	$(MAKE) target
	
target: B_OBJ = $(wildcard $(BUILD)*) 
target: $(TARGET)

clean:
	rm -f $(BUILD)*.o
	rm -f $(BUILD)*.elf
	rm -f *.img
	rm -f *.map
	rm -f *.list
	rm -f *.rf
	rm -f *.dump

$(BUILD)kernel.elf : $(LINKER) $(B_OBJ)
	$(GNU_TOOLS)-ld $(B_OBJ) -Llib $(LIBS) -T $(LINKER) -Map $(MAP) -o $(BUILD)kernel.elf
	$(GNU_TOOLS)-objdump -D $(BUILD)kernel.elf > $(LIST)

kernel.img : $(BUILD)kernel.elf
	$(GNU_TOOLS)-objcopy $(BUILD)kernel.elf -O binary kernel.img
	readelf -all $(BUILD)kernel.elf > $(READELF)

