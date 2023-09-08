################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/STM32-IDE/STM32-Library/STM32_ETH_W5500/ETH_W5500.c 

OBJS += \
./STM32_ETH_W5500/ETH_W5500.o 

C_DEPS += \
./STM32_ETH_W5500/ETH_W5500.d 


# Each subdirectory must supply rules for building sources it contributes
STM32_ETH_W5500/ETH_W5500.o: C:/STM32-IDE/STM32-Library/STM32_ETH_W5500/ETH_W5500.c STM32_ETH_W5500/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/STM32-IDE/STM32-LIBRARY/STRING" -I"C:/STM32-IDE/STM32-LIBRARY/ModBUS" -I"C:/STM32-IDE/STM32-LIBRARY/STM32_ETH_W5500" -I"C:/STM32-IDE/STM32-LIBRARY/HTTP" -I"C:/STM32-IDE/STM32-LIBRARY/LoRa" -I"C:/STM32-IDE/STM32-LIBRARY/ESP8266" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-STM32_ETH_W5500

clean-STM32_ETH_W5500:
	-$(RM) ./STM32_ETH_W5500/ETH_W5500.cyclo ./STM32_ETH_W5500/ETH_W5500.d ./STM32_ETH_W5500/ETH_W5500.o ./STM32_ETH_W5500/ETH_W5500.su

.PHONY: clean-STM32_ETH_W5500

