#include "initialisation.h"
#include "USB.h"


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

extern uint32_t SystemCoreClock;
int main(void)
{
	SystemInit();							// Activates floating point coprocessor and resets clock
	SystemClock_Config();					// Configure the clock and PLL - NB Currently done in SystemInit but will need updating for production board
	SystemCoreClockUpdate();				// Update SystemCoreClock (system clock frequency) derived from settings of oscillators, prescalers and PLL
	usb.InitUSB();
	InitDAC();
	InitBtnLED();							// PC13 blue button; PB7 is LD2 Blue; PB14 is LD3 Red

	DAC->DHR12R1 = 2000;
	while (1)
	{

		/*
		Message Type	MS Nybble	LS Nybble		Bytes		Data Byte 1			Data Byte 2
		-----------------------------------------------------------------------------------------
		Note Off		0x8			Channel			2			Note Number			Velocity
		Note On			0x9			Channel			2			Note Number			Velocity
		Poly Pressure	0xA			Channel			2			Note Number			Pressure
		Control Change	0xB			Channel			2			Controller 			Value
		Program Change	0xC			Channel			1			Program Number		-none-
		Ch. Pressure	0xD			Channel			1			Pressure			-none-
		Pitch Bend		0xE			Channel			2			Bend LSB (7-bit)	Bend MSB (7-bits)
		System			0xF			further spec	variable	variable			variable
		*/

		// Get next MIDI event
		if (midiEventWrite != midiEventRead) {

			// Count note on less note off events to establish whether to output gate
			if (midiArray[midiEventRead].msg == 9) {
				noteOn++;

				// Output pitch to DAC Lowest note is C1 (36 or 0x24), highest is C7 (108 or 0x6C)
				uint16_t dacOut = 4095 * (float)(std::min(std::max((int)midiArray[midiEventRead].db1, 36), 108) - 36) / 72;
				DAC->DHR12R1 = dacOut;
			}

			if (midiArray[midiEventRead].msg == 8 && noteOn > 0) 	noteOn--;

			// light up LED (PB14) and transmit gate (PA3)
			if (noteOn > 0) {
				GPIOB->BSRR |= GPIO_BSRR_BS_14;
				GPIOA->BSRR |= GPIO_BSRR_BS_3;
			}
			else {
				GPIOB->BSRR |= GPIO_BSRR_BR_14;
				GPIOA->BSRR |= GPIO_BSRR_BR_3;
			}

			midiEventRead = midiEventRead == MIDIBUFFERSIZE ? 0 : midiEventRead + 1;
		}


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

