#pragma once

#include "stm32f4xx.h"
#include <algorithm>
#include "GpioPin.h"

extern volatile uint32_t SysTickVal;
extern bool hostMode;
static constexpr uint32_t sysTickInterval = 1000;

void InitHardware();
void InitClocks();
void InitSysTick();
void DelayMS(uint32_t ms);
void InitMidiUART();
void InitTimer();
void Reboot();
void JumpToBootloader();

extern GpioPin modeSwitch;

