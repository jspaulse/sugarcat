# source/mach/vexpress_a9
# 
# This is the main Makefile for machine vexpress_a9

ARCH_SOURCE	= $(SOURCE)arch/
CPU_SOURCE	= $(ARCH_SOURCE)arm/cpu/

BOOT		= boot/
INIT		= init/

PASS_FLAGS 	= 'ARCH=$(ARCH)' BUILD='$(BUILD)' CFLAGS='$(CFLAGS)' AFLAGS='$(AFLAGS)'
PASS_FLAGS	+= GNU_TOOLS='$(GNU_TOOLS)' MACH='$(MACH)' CPU='$(CPU)'
PASS_FLAGS	+= SOURCE='$(SOURCE)' CONFIG_FLAGS='$(CONFIG_FLAGS)'

SRC_FILES	= $(notdir $(wildcard *.c)) $(notdir $(wildcard *.s))
SUB_FILES	= $(patsubst %.s, %.o, $(SRC_FILES))
OBJ			= $(addprefix $(BUILD), $(patsubst %.c, %.o, $(SUB_FILES)))


all:
	@$(MAKE) -s curr
	@$(MAKE) -s -C $(BOOT) $(PASS_FLAGS)
	@$(MAKE) -s -C $(INIT) $(PASS_FLAGS)
	
	# make arch specific
	@$(MAKE) -s -C $(ARCH_SOURCE)$(ARCH) $(PASS_FLAGS)
	@$(MAKE) -s -C $(CPU_SOURCE)$(CPU) $(PASS_FLAGS)

curr: $(OBJ)

$(BUILD)%.o : %.c
	@echo "[GCC]		$<"
	@$(GNU_TOOLS)-gcc -D $(CONFIG_FLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)%.o : %.s
	@echo "[ASM]		$<"
	@$(GNU_TOOLS)-as $(AFLAGS) $< -o $@

