<p align="center"><a href="https://www.pcbway.com"> <img src="https://s3-eu-west-1.amazonaws.com/tpd/logos/54695d4a00006400057b939d/0x0.png" alt="PCBWay" width=20%/></a></p>

# MidiMonger v2
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/midimonger2_front.jpg "icon")

MidiMonger is a Eurorack MIDI interface supporting USB and serial MIDI. 

MIDI Monger is a configurable MIDI interface for controlling Eurorack modules from either USB MIDI or Serial MIDI (or both simultaneously). MidiMonger 2 adds support for USB host functionality allowing the use of USB MIDI controllers (and experimentally USB HID mice) to control Eurorack modules.

MIDI Monger has 12 outputs of which 8 are gate outputs and 4 are control voltage outputs. The first eight outputs are configured by default as channel pairs. When assigned a MIDI channel they will act as a monophonic gate/cv driven from a single MIDI channel or in polyphonic mode supporting up to 4 note polyphony.

When not used in a pair a gate output can send a gate for any MIDI note from a channel, for a specific note number on a channel (eg for use as drum triggers) or a clock. The Control Voltage outputs can be configured to send controller data, pitch bend or aftertouch.

Configuration is carried out from a [web interface](https://htmlpreview.github.io/?https://github.com/dchwebb/MidiMonger/blob/master/WebEditor/index.html) using the Chrome browser or via a Virtual COM Port:

![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/console.png "icon")

# USB
MidiMonger can act as either a USB device (typically for connection to a computer) or a USB Host (allowing USB Midi controllers to be connected). The mode is selected using a switch on the front panel and USB connection is via a USB C socket.

In USB device mode USB MIDI data is merged with MIDI Serial data and sent to the CV and gate outputs as configured. In host mode USB devices can be connected directly to the module. This is typically used for MIDI keyboards and controllers that output via USB, but experimental support is also provided for USB HID devices - mouse support is currently implemented, but joysticks etc could be implemented in the future.

When connecting a USB mouse by default the left/right up/down and mouse wheel are output on CV 1, 2 and 3 and the buttons are output to the gates. This can be configured using the serial console.

In host mode the device will output 5v to power connected devices, but it is not recommended use with power hungry devices as the 5v rail is supplied via a linear regulator which has limited power output and also is used to supply a reference voltage to the DAC.

# Technical
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/midimonger2_back3.jpg "icon")
MIDI Monger is controlled by an ARM STM32F446 Microcontroller. The MCU manages USB (device and host modes) and serial MIDI. Control voltages are output via a Maxim MAX5134 16 bit DAC.

Gate outputs are converted to Eurorack standard of 5V using a SN74HCT244 Octal buffer as a level shifter. MIDI Serial signals are buffered through a 6N137 Optocoupler.

A UART connection is also provided to the module allowing access to the serial console when operating in USB host mode.

Serial MIDI is supplied via a 1/8" TRS jack using the MIDI standard wiring.

The module is constructed using three PCBs: a component board, a control board and a panel. Schematics and PCB layout created in KiCad v8 and available in [Hardware folder](https://github.com/dchwebb/MidiMonger/tree/master/Hardware_v2).

[Components schematic](Hardware_v2/MidiMonger_Components.pdf)

[Controls schematic](Hardware_v2/MidiMonger_Controls.pdf)

# Errata (v2 hardware)
- The 3.3v linear regulator runs quite warm - some passive PCB cooling would be a good idea or possibly the use of a switched mode power supply (no analog voltages are supplied from this rail).
- A separate switched mode 5V rail would allow the connection of more power hungry USB devices without risking noise or sagging on the analog 5V reference.
- The USB device/host icons on the mode switch front panel should be swapped.
- To allow PWM brightmess control over the CV LEDs, CV1_LED and CV2_LED should be switched to pins lined to TIM4 (CV3_LED and CV4_LED connected to TIM4_CH2 and CH3)

# Version 1 Hardware
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/midimonger_front.jpg "icon")
The original version of the module works with the latest firmware using the build in [Hardware folder](https://github.com/dchwebb/MidiMonger/tree/master/MidiMonger2/v1Hardware). The main differences are that the v1 hardware does not support USB host mode, uses a full size MIDI DIN connector and a USB B socket.


Sponsorship
-----------

[PCBWay](https://www.pcbway.com/) were kind enough to sponsor the v2 module, manufacturing and supplying the three PCBs used for the component and control boards as well as the front panel. The quality of each PCB was excellent and the fine details were crisp and precise. Hand-soldering the fine-pitch components on the prototypes was extremely easy thanks to the high quality accuracy of the solder mask.

Many thanks to PCBWay for their generosity in sponsoring this project and their services come highly recommended.

<p align="center"><a href="https://www.pcbway.com"> <img src="https://s3-eu-west-1.amazonaws.com/tpd/logos/54695d4a00006400057b939d/0x0.png" alt="PCBWay" width=20%/></a></p>

