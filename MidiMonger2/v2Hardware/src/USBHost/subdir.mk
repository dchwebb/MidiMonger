################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/USBHost/MidiHostClass.cpp \
../src/USBHost/USBClass.cpp \
../src/USBHost/USBHost.cpp 

OBJS += \
./src/USBHost/MidiHostClass.o \
./src/USBHost/USBClass.o \
./src/USBHost/USBHost.o 

CPP_DEPS += \
./src/USBHost/MidiHostClass.d \
./src/USBHost/USBClass.d \
./src/USBHost/USBHost.d 


# Each subdirectory must supply rules for building sources it contributes
src/USBHost/%.o src/USBHost/%.su src/USBHost/%.cyclo: ../src/USBHost/%.cpp src/USBHost/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++20 -g3 -DDEBUG -DSTM32F446xx -c -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/Eurorack/MidiMonger/MidiMonger2/src" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBDevice" -I"D:/Eurorack/MidiMonger/MidiMonger2/src/USBHost" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wno-volatile -Wno-deprecated-enum-enum-conversion -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-USBHost

clean-src-2f-USBHost:
	-$(RM) ./src/USBHost/MidiHostClass.cyclo ./src/USBHost/MidiHostClass.d ./src/USBHost/MidiHostClass.o ./src/USBHost/MidiHostClass.su ./src/USBHost/USBClass.cyclo ./src/USBHost/USBClass.d ./src/USBHost/USBClass.o ./src/USBHost/USBClass.su ./src/USBHost/USBHost.cyclo ./src/USBHost/USBHost.d ./src/USBHost/USBHost.o ./src/USBHost/USBHost.su

.PHONY: clean-src-2f-USBHost

