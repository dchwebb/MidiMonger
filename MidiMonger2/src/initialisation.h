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

#define USBH_DEBUG_LEVEL 3

#if (USBH_DEBUG_LEVEL > 0U)
#define USBH_UsrLog(...)   do { printf(__VA_ARGS__); printf("\n"); } while (0)
#else
#define USBH_UsrLog(...) do {} while (0)
#endif

#if (USBH_DEBUG_LEVEL > 1U)

#define USBH_ErrLog(...) do { printf("ERROR: "); printf(__VA_ARGS__); printf("\n");} while (0)
#else
#define USBH_ErrLog(...) do {} while (0)
#endif

#if (USBH_DEBUG_LEVEL > 2U)
#define USBH_DbgLog(...)   do { printf("DEBUG : "); printf(__VA_ARGS__); printf("\n");} while (0)
#else
#define USBH_DbgLog(...) do {} while (0)
#endif

