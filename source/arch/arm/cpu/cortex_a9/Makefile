# source/arch/arm/cpu/cortex_a9
# 
# This is the main Makefile for cpu cortex_a9
# cortex_a9 uses (and therefore, compiles) gic v1
#

ARCH_SOURCE	= $(SOURCE)arch/arm/
GIC_SOURCE	= $(ARCH_SOURCE)gic/

GIC_VER		= v1

PASS_FLAGS 	= 'ARCH=$(ARCH)' BUILD='$(BUILD)' CFLAGS='$(CFLAGS)' AFLAGS='$(AFLAGS)'
PASS_FLAGS	+= GNU_TOOLS='$(GNU_TOOLS)' MACH='$(MACH)' CPU='$(CPU)'
PASS_FLAGS	+= SOURCE='$(SOURCE)'

SRC_FILES		= $(notdir $(wildcard *.c)) $(notdir $(wildcard *.s))
SUB_FILES		= $(patsubst %.s, %.o, $(SRC_FILES))
OBJ				= $(addprefix $(BUILD), $(patsubst %.c, %.o, $(SUB_FILES)))

all:
	@$(MAKE) -s curr
	@$(MAKE) -s -C $(GIC_SOURCE)$(GIC_VER) $(PASS_FLAGS)

curr: $(OBJ)

$(BUILD)%.o : %.c
	@echo "[GCC]		$<"
	@$(GNU_TOOLS)-gcc $(CFLAGS) -c $< -o $@

$(BUILD)%.o : %.s
	@echo "[ASM]		$<"
	@$(GNU_TOOLS)-as $(AFLAGS) $< -o $@

