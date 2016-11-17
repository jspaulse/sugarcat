CURR_DIR 	:= $(realpath .)/

# boo!
BUILD		= build/
FBUILD		= $(CURR_DIR)$(BUILD)

SOURCE		= $(CURR_DIR)source/
ARCH_SOURCE	= $(SOURCE)arch/
MACH_SOURCE	= $(SOURCE)mach/
INCLUDE		= $(CURR_DIR)include/
CFG_DIR		= $(CURR_DIR)config/
CONFIGS		= $(BUILD)*.inc

LINKER		= $(MACH_SOURCE)$(MACH)/kernel.ld
TARGET		= kernel.img
LIST		= kernel.list
MAP			= kernel.map
READELF		= kernel.rf
CFLAGS 		= -I$(INCLUDE) -std=gnu11 -O2 -Wall -Werror -Wextra -Wshadow \
			-nostdlib -nostartfiles -ffreestanding \
			-pedantic -pedantic-errors $(ARCH_CFLAGS)
AFLAGS		= --warn --fatal-warnings -I $(INCLUDE) $(ARCH_AFLAGS)

PASS_FLAGS 	= 'ARCH=$(ARCH)' BUILD='$(FBUILD)' CFLAGS='$(CFLAGS)' AFLAGS='$(AFLAGS)'
PASS_FLAGS	+= GNU_TOOLS='$(GNU_TOOLS)' MACH='$(MACH)'

#PASS_FLAGS	= 'ARCH=$(ARCH)' BUILD='$(FBUILD)' ARCH_AFLAGS='$(ARCH_AFLAGS)'
#PASS_FLAGS	+= ARCH_CFLAGS='$(ARCH_CFLAGS)' GNU_TOOLS='$(GNU_TOOLS)'
#PASS_FLAGS	+= INCLUDE='$(INCLUDE)'

# include configurations
TAR_CFG_DIR = $(CFG_DIR)$@
-include $(CONFIGS)

all: $(LINKER) $(CONFIGS)
	@$(MAKE) -s -C $(MACH_SOURCE)$(MACH) $(PASS_FLAGS)
	@$(MAKE) -s -C $(ARCH_SOURCE)$(ARCH) $(PASS_FLAGS)
	@$(MAKE) -s base
	@$(MAKE) -s target
	
%cfg:
	@if [ -d "$(TAR_CFG_DIR)" ]; then \
		echo "Copying configuration files..."; \
		cp -rf $(TAR_CFG_DIR)/*.inc $(BUILD); \
		#cp -rf $(TAR_CFG_DIR)/*.ld $(LINKER); \
	else \
		echo "Configuration $(TAR_CFG_DIR) doesn't exist!"; \
	fi

base:
	@$(MAKE) -s -C $(SOURCE)util $(PASS_FLAGS)
	@$(MAKE) -s -C $(SOURCE)kernel/ $(PASS_FLAGS)
	#@$(MAKE) -s -C $(SOURCE)kernel/mm/ $(PASS_FLAGS)
	
target: B_OBJ = $(wildcard $(BUILD)*.o)
target: $(TARGET)

.PHONY: clean
clean:
	rm -f kernel.img
	rm -f $(BUILD)*.inc
	rm -f $(BUILD)*.ld
	rm -f $(BUILD)*.o
	rm -f $(BUILD)*.elf
	rm -f *.map
	rm -f *.list
	rm -f *.rf
	rm -f *.dump

$(BUILD)kernel.elf : $(B_OBJ)
	$(GNU_TOOLS)-gcc $(CFLAGS) $(B_OBJ) -T $(LINKER) -Wl,-Map=$(MAP) -o $(BUILD)kernel.elf
	#$(GNU_TOOLS)-ld $(B_OBJ) -T $(LINKER) -Map $(MAP) -o $(BUILD)kernel.elf
	$(GNU_TOOLS)-objdump -D $(BUILD)kernel.elf > $(LIST)

kernel.img : $(BUILD)kernel.elf
	$(GNU_TOOLS)-objcopy $(BUILD)kernel.elf -O binary kernel.img
	readelf -all $(BUILD)kernel.elf > $(READELF)

