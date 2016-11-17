# source/mach/vexpress_a9
# 
# This is the main Makefile for machine vexpress_a9

BOOT		= boot/
INIT		= init/

PASS_FLAGS 	= 'ARCH=$(ARCH)' BUILD='$(BUILD)' CFLAGS='$(CFLAGS)' AFLAGS='$(AFLAGS)'
PASS_FLAGS	+= GNU_TOOLS='$(GNU_TOOLS)' MACH='$(MACH)' CPU='$(CPU)'

SRC_FILES	= $(notdir $(wildcard *.c)) $(notdir $(wildcard *.s))
SUB_FILES	= $(patsubst %.s, %.o, $(SRC_FILES))
OBJ			= $(addprefix $(BUILD), $(patsubst %.c, %.o, $(SUB_FILES)))


all:
	@$(MAKE) -s curr
	@$(MAKE) -s -C $(BOOT) $(PASS_FLAGS)
	@$(MAKE) -s -C $(INIT) $(PASS_FLAGS)

curr: $(OBJ)

$(BUILD)%.o : %.c
	@echo "[GCC]		$<"
	@$(GNU_TOOLS)-gcc $(CFLAGS) -c $< -o $@

$(BUILD)%.o : %.s
	@echo "[ASM]		$<"
	@$(GNU_TOOLS)-as $(AFLAGS) $< -o $@

