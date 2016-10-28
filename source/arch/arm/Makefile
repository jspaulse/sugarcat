CFLAGS 		= -I $(INCLUDE) -std=gnu99 -O2 -Wall -Werror -Wextra -Werror -Wshadow \
			-nostdlib -nostartfiles -ffreestanding \
			-pedantic -pedantic-errors $(ARCH_CFLAGS)
AFLAGS		= --warn --fatal-warnings -I $(INCLUDE) $(ARCH_AFLAGS)
CC			= arm-none-eabi-gcc
AS			= arm-none-eabi-as

# why not?
SOURCE			= $(ARCH)/
SRC_FILES		= $(notdir $(wildcard $(SOURCE)*))

SUB_FILES		= $(patsubst %.s, %.o, $(SRC_FILES))
R_OBJ			= $(addprefix $(BUILD), $(patsubst %.c, %.o, $(SUB_FILES)))
OBJ				= $(patsubst %.S, %.o, $(R_OBJ))
all: $(OBJ)

$(BUILD)%.o : $(SOURCE)%.c
	$(GNU_TOOLS)-gcc $(CFLAGS) -c $< -o $@

$(BUILD)%.o : $(SOURCE)%.s
	$(GNU_TOOLS)-as $(AFLAGS) -c $< -o $@

$(BUILD)%.o : $(SOURCE)%.S
	$(GNU_TOOLS)-as $(AFLAGS) -c $< -o $@
	#$(AS) $(AFLAGS) -c $< -o $@
