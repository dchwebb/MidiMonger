################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CommandHandler.cpp \
../src/DACHandler.cpp \
../src/MidiControl.cpp \
../src/configManager.cpp \
../src/initialisation.cpp \
../src/main.cpp \
../src/uartHandler.cpp 

S_SRCS += \
../src/startup_stm32f446zetx.s 

C_SRCS += \
../src/syscalls.c \
../src/sysmem.c \
../src/system_stm32f4xx.c 

S_DEPS += \
./src/startup_stm32f446zetx.d 

C_DEPS += \
./src/syscalls.d \
./src/sysmem.d \
./src/system_stm32f4xx.d 

OBJS += \
./src/CommandHandler.o \
./src/DACHandler.o \
./src/MidiControl.o \
./src/configManager.o \
./src/initialisation.o \
./src/main.o \
./src/startup_stm32f446zetx.o \
./src/syscalls.o \
./src/sysmem.o \
./src/system_stm32f4xx.o \
./src/uartHandler.o 

CPP_DEPS += \
./src/CommandHandler.d \
./src/DACHandler.d \
./src/MidiControl.d \
./src/configManager.d \
./src/initialisation.d \
./src/main.d \
./src/uartHandler.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o src/%.su src/%.cyclo: ../src/%.cpp src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++20 -g3 -DDEBUG -DSTM32F446xx -c -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/Eurorack/MidiMonger/MidiMonger2/src" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBDevice" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBHost" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wno-volatile -Wno-deprecated-enum-enum-conversion -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
src/%.o: ../src/%.s src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I"D:/Eurorack/MidiMonger/MidiMonger2/src" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBDevice" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBHost" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"
src/%.o src/%.su src/%.cyclo: ../src/%.c src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F446xx -c -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/Eurorack/MidiMonger/MidiMonger2/src" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBDevice" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBHost" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src

clean-src:
	-$(RM) ./src/CommandHandler.cyclo ./src/CommandHandler.d ./src/CommandHandler.o ./src/CommandHandler.su ./src/DACHandler.cyclo ./src/DACHandler.d ./src/DACHandler.o ./src/DACHandler.su ./src/MidiControl.cyclo ./src/MidiControl.d ./src/MidiControl.o ./src/MidiControl.su ./src/configManager.cyclo ./src/configManager.d ./src/configManager.o ./src/configManager.su ./src/initialisation.cyclo ./src/initialisation.d ./src/initialisation.o ./src/initialisation.su ./src/main.cyclo ./src/main.d ./src/main.o ./src/main.su ./src/startup_stm32f446zetx.d ./src/startup_stm32f446zetx.o ./src/syscalls.cyclo ./src/syscalls.d ./src/syscalls.o ./src/syscalls.su ./src/sysmem.cyclo ./src/sysmem.d ./src/sysmem.o ./src/sysmem.su ./src/system_stm32f4xx.cyclo ./src/system_stm32f4xx.d ./src/system_stm32f4xx.o ./src/system_stm32f4xx.su ./src/uartHandler.cyclo ./src/uartHandler.d ./src/uartHandler.o ./src/uartHandler.su

.PHONY: clean-src

