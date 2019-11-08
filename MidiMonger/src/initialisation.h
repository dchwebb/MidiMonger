#pragma once

#include "stm32f4xx.h"
#include <algorithm>

extern volatile uint32_t SysTickVal;

#define MIDIBUFFERSIZE 100

void SystemClock_Config(void);
void InitIO(void);
void InitSysTick();
void InitSampleAcquisition();
void InitCoverageTimer();
void InitDebounceTimer();
void InitEncoders();
void InitUART();
void InitDAC();

