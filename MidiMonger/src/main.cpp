#include "initialisation.h"
#include "USB.h"
#include "MidiHandler.h"
#include "DACHandler.h"

USB usb;
uint32_t usbEvents[200];
uint32_t reqEvents[100];
uint8_t usbEventNo = 0;
uint8_t reqEventNo = 0;
uint8_t midiEventRead = 0;
uint8_t midiEventWrite = 0;
uint8_t eventOcc = 0;
uint16_t noteOn = 0;


bool noteDown = false;

extern "C" {
#include "interrupts.h"
}

MidiData midiArray[MIDIBUFFERSIZE];
MidiHandler midiHandler;
DACHandler dacHandler;

extern uint32_t SystemCoreClock;
int main(void)
{
	SystemInit();							// Activates floating point coprocessor and resets clock
	SystemClock_Config();					// Configure the clock and PLL - NB Currently done in SystemInit but will need updating for production board
	SystemCoreClockUpdate();				// Update SystemCoreClock (system clock frequency) derived from settings of oscillators, prescalers and PLL
	//usb.InitUSB();
	InitDAC();
	dacHandler.initDAC();
	InitBtnLED();							// PC13 blue button; PB7 is LD2 Blue; PB14 is LD3 Red

	// Bind the usb.dataHandler function to the midiHandler's event handler
	//usb.dataHandler = std::bind(&MidiHandler::eventHandler, midiHandler, std::placeholders::_1);

	// Initialise internal DAC
	DAC->DHR12R1 = 2000;



	while (1) {
		for (uint16_t d = 0; d < 0xFFFF; ++d) {
			dacHandler.sendData(WriteChannel, ChannelA, d);
		}
	}
	dacHandler.sendData(WriteChannel, ChannelB, 0xFEDC);

	while (1)
	{

		// Code to output midi note
		uint8_t noteOn[4];
		noteOn[0] = 0x08;
		noteOn[1] = 0x90;		// 9 = note on 0 = channel
		noteOn[2] = 60;			// note number 60 = C3
		noteOn[3] = 100;		// Velocity 47

		uint8_t noteOff[4];
		noteOff[0] = 0x08;
		noteOff[1] = 0x80;		// 9 = note off 0 = channel
		noteOff[2] = 60;		// note number 60 = C4
		noteOff[3] = 47;		// Velocity

		if (GPIOC->IDR & GPIO_IDR_IDR_13) {
			GPIOB->BSRR |= GPIO_BSRR_BS_7;
			if (!noteDown) {
				noteDown = true;
				usb.SendReport(noteOn, 4);
			}
		}
		else {
 			GPIOB->BSRR |= GPIO_BSRR_BR_7;
			if (noteDown) {
				usb.SendReport(noteOff, 4);
				noteDown = false;
			}
		}

	}
}

