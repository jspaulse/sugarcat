# source/mach/vexpress/boot

#CFLAGS 		= -I $(INCLUDE) -std=gnu99 -O2 -Wall -Werror -Wextra -Werror -Wshadow \
#			-nostdlib -nostartfiles -ffreestanding \
#			-pedantic -pedantic-errors $(ARCH_CFLAGS)
#AFLAGS		= --warn --fatal-warnings -I $(INCLUDE) $(ARCH_AFLAGS)

SRC_FILES	= $(notdir $(wildcard *.c)) $(notdir $(wildcard *.s))
SUB_FILES	= $(patsubst %.s, %.o, $(SRC_FILES))
OBJ			= $(addprefix $(BUILD), $(patsubst %.c, %.o, $(SUB_FILES)))

all: $(OBJ)

$(BUILD)%.o : %.c
	@echo "[GCC]		$<"
	@$(GNU_TOOLS)-gcc $(CFLAGS) -c $< -o $@

$(BUILD)%.o : %.s
	@echo "[ASM]		$<"
	@$(GNU_TOOLS)-as $(AFLAGS) $< -o $@

