# MidiMonger v2
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/midimonger2_front.jpg "icon")

MidiMonger is a Eurorack MIDI interface supporting USB and serial MIDI. 

MIDI Monger is a configurable MIDI interface for controlling Eurorack modules from either USB MIDI or Serial MIDI (or both simultaneously). MidiMonger 2 adds support for USB host functionality allowing the use of HID devices (eg a mouse) to control Eurorack modules.

MIDI Monger has 12 outputs of which 8 are gate outputs and 4 are control voltage outputs. The first eight outputs are configured by default as channel pairs. When assigned a MIDI channel they will act as a monophonic gate/cv driven from a single MIDI channel or in polyphonic mode supporting up to 4 note polyphony.

When not used in a pair a gate output can send a gate for any MIDI on note from a channel, for a specific note number on a channel (eg for use as drum triggers) or a clock. The Control Voltage outputs can be configured to send controller data, pitch bend or aftertouch.

Configuration is carried out from a [web interface](https://htmlpreview.github.io/?https://github.com/dchwebb/MidiMonger/blob/master/WebEditor/index.html) using the Chrome browser or via a Virtual COM Port.

# Technical
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/midimonger_components.jpg "icon")
MIDI Monger is based around an ARM STM32F446 Microcontroller. The MCU handles both MIDI and UART Serial interfacing and outputs control voltage via an external 16 bit DAC (Maxim MAX5134). Gate outputs are converted to Eurorack standard of 5V using a SN74HCT244 Octal buffer as a level shifter. MIDI Serial signals are buffered through a 6N137 Optocoupler.

The module is constructed using three PCBs: a component board, a control board and a panel. Schematics and PCB layout created in KiCad and available in [Hardware folder](https://github.com/dchwebb/MidiMonger/tree/master/Hardware).

Annotated component PCB
-----------------------
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/components.png "icon")

Legend
1) Maxim MAX5134 DAC and TL074 Buffer
2) 3.3V to 5V Level shifter using SN74HCT244
3) STM32F446 Microcontroller
4) MIDI Serial buffer using 6N137 Optocoupler
5) Eurorack Power input and conversion to 3.3V and 5V rails
6) SWD Programming header (eg using ST Link)

Control PCB
-----------
![Image](https://github.com/dchwebb/MidiMonger/raw/master/pictures/controls.png "icon")

Sponsorship
-----------

[PCBWay](https://www.pcbway.com/) were kind enough to sponsor the v2 module, manufacturing and supplying the three PCBs used for the component and control boards as well as the front panel. The quality of each PCB was excellent and the fine details were crisp and precise. Hand-soldering the fine-pitch components on the prototypes was extremely easy thanks to the high quality accuracy of the solder mask.

<p align="center"><a href="https://www.pcbway.com"> <img src="https://s3-eu-west-1.amazonaws.com/tpd/logos/54695d4a00006400057b939d/0x0.png" alt="PCBWay" width=20%/></a></p>

