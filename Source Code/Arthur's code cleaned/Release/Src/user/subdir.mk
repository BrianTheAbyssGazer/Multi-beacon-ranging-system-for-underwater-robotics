################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/user/adc_over_uart.cpp \
../Src/user/cmd_rx.cpp \
../Src/user/pga.cpp 

C_SRCS += \
../Src/user/basic_peak_detector.c \
../Src/user/basic_pulse_stream.c \
../Src/user/index_info_transmit.c 

C_DEPS += \
./Src/user/basic_peak_detector.d \
./Src/user/basic_pulse_stream.d \
./Src/user/index_info_transmit.d 

OBJS += \
./Src/user/adc_over_uart.o \
./Src/user/basic_peak_detector.o \
./Src/user/basic_pulse_stream.o \
./Src/user/cmd_rx.o \
./Src/user/index_info_transmit.o \
./Src/user/pga.o 

CPP_DEPS += \
./Src/user/adc_over_uart.d \
./Src/user/cmd_rx.d \
./Src/user/pga.d 


# Each subdirectory must supply rules for building sources it contributes
Src/user/%.o Src/user/%.su Src/user/%.cyclo: ../Src/user/%.cpp Src/user/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Inc -O3 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/user/%.o Src/user/%.su Src/user/%.cyclo: ../Src/user/%.c Src/user/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-user

clean-Src-2f-user:
	-$(RM) ./Src/user/adc_over_uart.cyclo ./Src/user/adc_over_uart.d ./Src/user/adc_over_uart.o ./Src/user/adc_over_uart.su ./Src/user/basic_peak_detector.cyclo ./Src/user/basic_peak_detector.d ./Src/user/basic_peak_detector.o ./Src/user/basic_peak_detector.su ./Src/user/basic_pulse_stream.cyclo ./Src/user/basic_pulse_stream.d ./Src/user/basic_pulse_stream.o ./Src/user/basic_pulse_stream.su ./Src/user/cmd_rx.cyclo ./Src/user/cmd_rx.d ./Src/user/cmd_rx.o ./Src/user/cmd_rx.su ./Src/user/index_info_transmit.cyclo ./Src/user/index_info_transmit.d ./Src/user/index_info_transmit.o ./Src/user/index_info_transmit.su ./Src/user/pga.cyclo ./Src/user/pga.d ./Src/user/pga.o ./Src/user/pga.su

.PHONY: clean-Src-2f-user

