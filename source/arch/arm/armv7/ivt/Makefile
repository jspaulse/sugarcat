# source/arch/arm/armv7/ivt
# 
# This is the Makefile for armv7 ivt

SRC_FILES		= $(notdir $(wildcard *.c)) $(notdir $(wildcard *.s))
SUB_FILES		= $(patsubst %.s, %.o, $(SRC_FILES))
OBJ				= $(addprefix $(BUILD), $(patsubst %.c, %.o, $(SUB_FILES)))

all: $(OBJ)

$(BUILD)%.o : %.c
	@echo "[GCC]	$<"
	@$(GNU_TOOLS)-gcc $(CFLAGS) -c $< -o $@

$(BUILD)%.o : %.s
	@echo "[ASM]	$<"
	@$(GNU_TOOLS)-as $(AFLAGS) $< -o $@

