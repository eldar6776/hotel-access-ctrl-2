################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F0xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
CS.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
CS.list \

OBJCOPY_HEX += \
CS.hex \


# All Target
all: CS.elf secondary-outputs

# Tool invocations
CS.elf: $(OBJS) $(USER_OBJS) F:\ProjektiOtvoreni\HotelVucko\fw\VUCKO_RC1\CS\STM32F030F4PX_FLASH.ld
	arm-none-eabi-gcc -o "CS.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0 -T"F:\ProjektiOtvoreni\HotelVucko\fw\VUCKO_RC1\CS\STM32F030F4PX_FLASH.ld" --specs=nosys.specs -Wl,-Map="CS.map" -Wl,--cref -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

CS.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "CS.list"
	@echo 'Finished building: $@'
	@echo ' '

CS.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "CS.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
