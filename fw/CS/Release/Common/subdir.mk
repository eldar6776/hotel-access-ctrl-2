################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/ProjektiOtvoreni/HotelVucko/fw/VUCKO_RC1/Common/common.c 

OBJS += \
./Common/common.o 

C_DEPS += \
./Common/common.d 


# Each subdirectory must supply rules for building sources it contributes
Common/common.o: F:/ProjektiOtvoreni/HotelVucko/fw/VUCKO_RC1/Common/common.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F030x6 -DCARD_STACKER -DAPPLICATION -DUSE_WATCHDOG -c -I../Core/Inc -I../../Common -I../Drivers/CMSIS/Include -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Common/common.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

