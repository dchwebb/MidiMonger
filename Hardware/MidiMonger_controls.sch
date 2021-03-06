EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "MIDI Monger"
Date ""
Rev ""
Comp "Mountjoy Modular"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L thonkiconn:AudioJack2_Ground_Switch J11
U 1 1 5C8DCECD
P 4400 1200
F 0 "J11" H 4404 1542 50  0000 C CNN
F 1 "CV1_Out" H 4404 1451 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 4400 1200 50  0001 C CNN
F 3 "~" H 4400 1200 50  0001 C CNN
	1    4400 1200
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5C8E7D2E
P 4100 1400
F 0 "#PWR011" H 4100 1150 50  0001 C CNN
F 1 "GND" V 4100 1200 50  0000 C CNN
F 2 "" H 4100 1400 50  0001 C CNN
F 3 "" H 4100 1400 50  0001 C CNN
	1    4100 1400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4200 1300 4100 1300
Wire Wire Line
	4100 1300 4100 1400
$Comp
L power:GND #PWR016
U 1 1 5D80DC54
P 6300 3700
F 0 "#PWR016" H 6300 3450 50  0001 C CNN
F 1 "GND" V 6300 3500 50  0000 C CNN
F 2 "" H 6300 3700 50  0001 C CNN
F 3 "" H 6300 3700 50  0001 C CNN
	1    6300 3700
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5D32F73D
P 6000 3700
F 0 "SW1" H 6000 3985 50  0000 C CNN
F 1 "SW_Push" H 6000 3894 50  0000 C CNN
F 2 "Buttons_Switches_THT:SW_PUSH_6mm" H 6000 3900 50  0001 C CNN
F 3 "~" H 6000 3900 50  0001 C CNN
	1    6000 3700
	1    0    0    -1  
$EndComp
$Comp
L thonkiconn:AudioJack2_Ground_Switch J4
U 1 1 5DC12CA0
P 1800 1200
F 0 "J4" H 1804 1542 50  0000 C CNN
F 1 "Gate1_Out" H 1804 1451 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 1800 1200 50  0001 C CNN
F 3 "~" H 1800 1200 50  0001 C CNN
	1    1800 1200
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5DC12CAA
P 1500 1400
F 0 "#PWR04" H 1500 1150 50  0001 C CNN
F 1 "GND" V 1500 1200 50  0000 C CNN
F 2 "" H 1500 1400 50  0001 C CNN
F 3 "" H 1500 1400 50  0001 C CNN
	1    1500 1400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1600 1300 1500 1300
Wire Wire Line
	1500 1300 1500 1400
Wire Wire Line
	1600 1200 1400 1200
Text GLabel 5900 1250 2    50   Input ~ 0
USB_VBUS
Text GLabel 5900 1450 2    50   Input ~ 0
USB_DP
Text GLabel 5900 1550 2    50   Input ~ 0
USB_DM
Wire Wire Line
	5900 1550 5800 1550
Wire Wire Line
	5800 1450 5900 1450
Wire Wire Line
	5800 1250 5900 1250
$Comp
L power:GND #PWR015
U 1 1 5DB5830A
P 5500 1950
F 0 "#PWR015" H 5500 1700 50  0001 C CNN
F 1 "GND" H 5505 1777 50  0000 C CNN
F 2 "" H 5500 1950 50  0001 C CNN
F 3 "" H 5500 1950 50  0001 C CNN
	1    5500 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 1950 5500 1850
NoConn ~ 5400 1850
Text Notes 6400 1250 0    50   ~ 0
red
Text Notes 6300 1450 0    50   ~ 0
green
Text Notes 6300 1550 0    50   ~ 0
white
$Comp
L thonkiconn:AudioJack2_Ground_Switch J12
U 1 1 5DC2B98C
P 4400 2000
F 0 "J12" H 4404 2342 50  0000 C CNN
F 1 "CV2_Out" H 4404 2251 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 4400 2000 50  0001 C CNN
F 3 "~" H 4400 2000 50  0001 C CNN
	1    4400 2000
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5DC2B996
P 4100 2200
F 0 "#PWR012" H 4100 1950 50  0001 C CNN
F 1 "GND" V 4100 2000 50  0000 C CNN
F 2 "" H 4100 2200 50  0001 C CNN
F 3 "" H 4100 2200 50  0001 C CNN
	1    4100 2200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4200 2100 4100 2100
Wire Wire Line
	4100 2100 4100 2200
$Comp
L thonkiconn:AudioJack2_Ground_Switch J13
U 1 1 5DC6C012
P 4400 2800
F 0 "J13" H 4404 3142 50  0000 C CNN
F 1 "CV3_Out" H 4404 3051 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 4400 2800 50  0001 C CNN
F 3 "~" H 4400 2800 50  0001 C CNN
	1    4400 2800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR013
U 1 1 5DC6C01C
P 4100 3000
F 0 "#PWR013" H 4100 2750 50  0001 C CNN
F 1 "GND" V 4100 2800 50  0000 C CNN
F 2 "" H 4100 3000 50  0001 C CNN
F 3 "" H 4100 3000 50  0001 C CNN
	1    4100 3000
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4200 2900 4100 2900
Wire Wire Line
	4100 2900 4100 3000
Text GLabel 1400 1200 0    50   Input ~ 0
GATE1_OUT
$Comp
L Device:LED D1
U 1 1 5DDECEA0
P 8000 1100
F 0 "D1" H 7993 845 50  0000 C CNN
F 1 "GATE1" H 7993 936 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 1100 50  0001 C CNN
F 3 "~" H 8000 1100 50  0001 C CNN
	1    8000 1100
	-1   0    0    1   
$EndComp
Wire Wire Line
	7800 1100 7850 1100
$Comp
L power:GND #PWR017
U 1 1 5DDFEEEC
P 8200 1100
F 0 "#PWR017" H 8200 850 50  0001 C CNN
F 1 "GND" V 8200 900 50  0000 C CNN
F 2 "" H 8200 1100 50  0001 C CNN
F 3 "" H 8200 1100 50  0001 C CNN
	1    8200 1100
	0    -1   1    0   
$EndComp
Wire Wire Line
	8150 1100 8200 1100
$Comp
L thonkiconn:AudioJack2_Ground_Switch J14
U 1 1 5DE30614
P 4400 3600
F 0 "J14" H 4404 3942 50  0000 C CNN
F 1 "CV4_Out" H 4404 3851 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 4400 3600 50  0001 C CNN
F 3 "~" H 4400 3600 50  0001 C CNN
	1    4400 3600
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR014
U 1 1 5DE3061E
P 4100 3800
F 0 "#PWR014" H 4100 3550 50  0001 C CNN
F 1 "GND" V 4100 3600 50  0000 C CNN
F 2 "" H 4100 3800 50  0001 C CNN
F 3 "" H 4100 3800 50  0001 C CNN
	1    4100 3800
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4200 3700 4100 3700
Wire Wire Line
	4100 3700 4100 3800
$Comp
L MIDI_DIN:MIDI_DIN J16
U 1 1 5DD49C19
P 5900 2750
F 0 "J16" H 5906 2475 50  0000 C CNN
F 1 "MIDI_DIN" H 5906 2384 50  0000 C CNN
F 2 "Custom_Footprints:MIDI_DIN" H 5900 2750 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/18/40_c091_abd_e-75918.pdf" H 5900 2750 50  0001 C CNN
	1    5900 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 2650 6350 2650
Wire Wire Line
	5600 2650 5500 2650
$Comp
L thonkiconn:AudioJack2_Ground_Switch J5
U 1 1 5DF35361
P 1800 2000
F 0 "J5" H 1804 2342 50  0000 C CNN
F 1 "Gate2_Out" H 1804 2251 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 1800 2000 50  0001 C CNN
F 3 "~" H 1800 2000 50  0001 C CNN
	1    1800 2000
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5DF35367
P 1500 2200
F 0 "#PWR05" H 1500 1950 50  0001 C CNN
F 1 "GND" V 1500 2000 50  0000 C CNN
F 2 "" H 1500 2200 50  0001 C CNN
F 3 "" H 1500 2200 50  0001 C CNN
	1    1500 2200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1600 2100 1500 2100
Wire Wire Line
	1500 2100 1500 2200
Wire Wire Line
	1600 2000 1400 2000
Text GLabel 1400 2000 0    50   Input ~ 0
GATE2_OUT
$Comp
L thonkiconn:AudioJack2_Ground_Switch J2
U 1 1 5DF4B93F
P 1750 2800
F 0 "J2" H 1754 3142 50  0000 C CNN
F 1 "Gate3_Out" H 1754 3051 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 1750 2800 50  0001 C CNN
F 3 "~" H 1750 2800 50  0001 C CNN
	1    1750 2800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5DF4B945
P 1450 3000
F 0 "#PWR02" H 1450 2750 50  0001 C CNN
F 1 "GND" V 1450 2800 50  0000 C CNN
F 2 "" H 1450 3000 50  0001 C CNN
F 3 "" H 1450 3000 50  0001 C CNN
	1    1450 3000
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1550 2900 1450 2900
Wire Wire Line
	1450 2900 1450 3000
Wire Wire Line
	1550 2800 1350 2800
Text GLabel 1350 2800 0    50   Input ~ 0
GATE3_OUT
$Comp
L thonkiconn:AudioJack2_Ground_Switch J3
U 1 1 5DF62B3E
P 1750 3550
F 0 "J3" H 1754 3892 50  0000 C CNN
F 1 "Gate4_Out" H 1754 3801 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 1750 3550 50  0001 C CNN
F 3 "~" H 1750 3550 50  0001 C CNN
	1    1750 3550
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5DF62B44
P 1450 3750
F 0 "#PWR03" H 1450 3500 50  0001 C CNN
F 1 "GND" V 1450 3550 50  0000 C CNN
F 2 "" H 1450 3750 50  0001 C CNN
F 3 "" H 1450 3750 50  0001 C CNN
	1    1450 3750
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1550 3650 1450 3650
Wire Wire Line
	1450 3650 1450 3750
Wire Wire Line
	1550 3550 1350 3550
Text GLabel 1350 3550 0    50   Input ~ 0
GATE4_OUT
$Comp
L thonkiconn:AudioJack2_Ground_Switch J7
U 1 1 5E050A7D
P 3150 1200
F 0 "J7" H 3154 1542 50  0000 C CNN
F 1 "Gate5_Out" H 3154 1451 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 3150 1200 50  0001 C CNN
F 3 "~" H 3150 1200 50  0001 C CNN
	1    3150 1200
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5E050A87
P 2850 1400
F 0 "#PWR07" H 2850 1150 50  0001 C CNN
F 1 "GND" V 2850 1200 50  0000 C CNN
F 2 "" H 2850 1400 50  0001 C CNN
F 3 "" H 2850 1400 50  0001 C CNN
	1    2850 1400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2950 1300 2850 1300
Wire Wire Line
	2850 1300 2850 1400
Wire Wire Line
	2950 1200 2750 1200
Text GLabel 2750 1200 0    50   Input ~ 0
GATE5_OUT
$Comp
L thonkiconn:AudioJack2_Ground_Switch J8
U 1 1 5E050A98
P 3150 2000
F 0 "J8" H 3154 2342 50  0000 C CNN
F 1 "Gate6_Out" H 3154 2251 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 3150 2000 50  0001 C CNN
F 3 "~" H 3150 2000 50  0001 C CNN
	1    3150 2000
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR08
U 1 1 5E050AA2
P 2850 2200
F 0 "#PWR08" H 2850 1950 50  0001 C CNN
F 1 "GND" V 2850 2000 50  0000 C CNN
F 2 "" H 2850 2200 50  0001 C CNN
F 3 "" H 2850 2200 50  0001 C CNN
	1    2850 2200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2950 2100 2850 2100
Wire Wire Line
	2850 2100 2850 2200
Wire Wire Line
	2950 2000 2750 2000
Text GLabel 2750 2000 0    50   Input ~ 0
GATE6_OUT
$Comp
L thonkiconn:AudioJack2_Ground_Switch J9
U 1 1 5E050AB3
P 3150 2800
F 0 "J9" H 3154 3142 50  0000 C CNN
F 1 "Gate7_Out" H 3154 3051 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 3150 2800 50  0001 C CNN
F 3 "~" H 3150 2800 50  0001 C CNN
	1    3150 2800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR09
U 1 1 5E050ABD
P 2850 3000
F 0 "#PWR09" H 2850 2750 50  0001 C CNN
F 1 "GND" V 2850 2800 50  0000 C CNN
F 2 "" H 2850 3000 50  0001 C CNN
F 3 "" H 2850 3000 50  0001 C CNN
	1    2850 3000
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2950 2900 2850 2900
Wire Wire Line
	2850 2900 2850 3000
Wire Wire Line
	2950 2800 2750 2800
Text GLabel 2750 2800 0    50   Input ~ 0
GATE7_OUT
$Comp
L thonkiconn:AudioJack2_Ground_Switch J10
U 1 1 5E050ACE
P 3150 3600
F 0 "J10" H 3154 3942 50  0000 C CNN
F 1 "Gate8_Out" H 3154 3851 50  0000 C CNN
F 2 "Custom_Footprints:Cliff_3.5mm_Stereo_Jack" H 3150 3600 50  0001 C CNN
F 3 "~" H 3150 3600 50  0001 C CNN
	1    3150 3600
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR010
U 1 1 5E050AD8
P 2850 3800
F 0 "#PWR010" H 2850 3550 50  0001 C CNN
F 1 "GND" V 2850 3600 50  0000 C CNN
F 2 "" H 2850 3800 50  0001 C CNN
F 3 "" H 2850 3800 50  0001 C CNN
	1    2850 3800
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2950 3700 2850 3700
Wire Wire Line
	2850 3700 2850 3800
Wire Wire Line
	2950 3600 2750 3600
Text GLabel 2750 3600 0    50   Input ~ 0
GATE8_OUT
$Comp
L Device:LED D2
U 1 1 5E214707
P 8000 1500
F 0 "D2" H 7993 1245 50  0000 C CNN
F 1 "GATE2" H 7993 1336 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 1500 50  0001 C CNN
F 3 "~" H 8000 1500 50  0001 C CNN
	1    8000 1500
	-1   0    0    1   
$EndComp
Wire Wire Line
	7800 1500 7850 1500
$Comp
L power:GND #PWR018
U 1 1 5E214714
P 8200 1500
F 0 "#PWR018" H 8200 1250 50  0001 C CNN
F 1 "GND" V 8200 1300 50  0000 C CNN
F 2 "" H 8200 1500 50  0001 C CNN
F 3 "" H 8200 1500 50  0001 C CNN
	1    8200 1500
	0    -1   1    0   
$EndComp
Wire Wire Line
	8150 1500 8200 1500
$Comp
L Device:LED D3
U 1 1 5E230C85
P 8000 1900
F 0 "D3" H 7993 1645 50  0000 C CNN
F 1 "GATE3" H 7993 1736 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 1900 50  0001 C CNN
F 3 "~" H 8000 1900 50  0001 C CNN
	1    8000 1900
	-1   0    0    1   
$EndComp
Wire Wire Line
	7800 1900 7850 1900
$Comp
L power:GND #PWR019
U 1 1 5E230C92
P 8200 1900
F 0 "#PWR019" H 8200 1650 50  0001 C CNN
F 1 "GND" V 8200 1700 50  0000 C CNN
F 2 "" H 8200 1900 50  0001 C CNN
F 3 "" H 8200 1900 50  0001 C CNN
	1    8200 1900
	0    -1   1    0   
$EndComp
Wire Wire Line
	8150 1900 8200 1900
$Comp
L Device:LED D4
U 1 1 5E24DDCB
P 8000 2300
F 0 "D4" H 7993 2045 50  0000 C CNN
F 1 "GATE4" H 7993 2136 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 2300 50  0001 C CNN
F 3 "~" H 8000 2300 50  0001 C CNN
	1    8000 2300
	-1   0    0    1   
$EndComp
Wire Wire Line
	7800 2300 7850 2300
$Comp
L power:GND #PWR020
U 1 1 5E24DDD8
P 8200 2300
F 0 "#PWR020" H 8200 2050 50  0001 C CNN
F 1 "GND" V 8200 2100 50  0000 C CNN
F 2 "" H 8200 2300 50  0001 C CNN
F 3 "" H 8200 2300 50  0001 C CNN
	1    8200 2300
	0    -1   1    0   
$EndComp
Wire Wire Line
	8150 2300 8200 2300
$Comp
L Device:LED D9
U 1 1 5E26B071
P 9500 1100
F 0 "D9" H 9493 845 50  0000 C CNN
F 1 "GATE5" H 9493 936 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 9500 1100 50  0001 C CNN
F 3 "~" H 9500 1100 50  0001 C CNN
	1    9500 1100
	-1   0    0    1   
$EndComp
Wire Wire Line
	9300 1100 9350 1100
$Comp
L power:GND #PWR025
U 1 1 5E26B086
P 9700 1100
F 0 "#PWR025" H 9700 850 50  0001 C CNN
F 1 "GND" V 9700 900 50  0000 C CNN
F 2 "" H 9700 1100 50  0001 C CNN
F 3 "" H 9700 1100 50  0001 C CNN
	1    9700 1100
	0    -1   1    0   
$EndComp
Wire Wire Line
	9650 1100 9700 1100
$Comp
L Device:LED D10
U 1 1 5E26B093
P 9500 1500
F 0 "D10" H 9493 1245 50  0000 C CNN
F 1 "GATE6" H 9493 1336 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 9500 1500 50  0001 C CNN
F 3 "~" H 9500 1500 50  0001 C CNN
	1    9500 1500
	-1   0    0    1   
$EndComp
Wire Wire Line
	9300 1500 9350 1500
$Comp
L power:GND #PWR026
U 1 1 5E26B0A8
P 9700 1500
F 0 "#PWR026" H 9700 1250 50  0001 C CNN
F 1 "GND" V 9700 1300 50  0000 C CNN
F 2 "" H 9700 1500 50  0001 C CNN
F 3 "" H 9700 1500 50  0001 C CNN
	1    9700 1500
	0    -1   1    0   
$EndComp
Wire Wire Line
	9650 1500 9700 1500
$Comp
L Device:LED D11
U 1 1 5E26B0B5
P 9500 1900
F 0 "D11" H 9493 1645 50  0000 C CNN
F 1 "GATE7" H 9493 1736 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 9500 1900 50  0001 C CNN
F 3 "~" H 9500 1900 50  0001 C CNN
	1    9500 1900
	-1   0    0    1   
$EndComp
Wire Wire Line
	9300 1900 9350 1900
$Comp
L power:GND #PWR027
U 1 1 5E26B0CA
P 9700 1900
F 0 "#PWR027" H 9700 1650 50  0001 C CNN
F 1 "GND" V 9700 1700 50  0000 C CNN
F 2 "" H 9700 1900 50  0001 C CNN
F 3 "" H 9700 1900 50  0001 C CNN
	1    9700 1900
	0    -1   1    0   
$EndComp
Wire Wire Line
	9650 1900 9700 1900
$Comp
L Device:LED D12
U 1 1 5E26B0D7
P 9500 2300
F 0 "D12" H 9493 2045 50  0000 C CNN
F 1 "GATE8" H 9493 2136 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 9500 2300 50  0001 C CNN
F 3 "~" H 9500 2300 50  0001 C CNN
	1    9500 2300
	-1   0    0    1   
$EndComp
Wire Wire Line
	9300 2300 9350 2300
$Comp
L power:GND #PWR028
U 1 1 5E26B0EC
P 9700 2300
F 0 "#PWR028" H 9700 2050 50  0001 C CNN
F 1 "GND" V 9700 2100 50  0000 C CNN
F 2 "" H 9700 2300 50  0001 C CNN
F 3 "" H 9700 2300 50  0001 C CNN
	1    9700 2300
	0    -1   1    0   
$EndComp
Wire Wire Line
	9650 2300 9700 2300
$Comp
L Device:LED D5
U 1 1 5E2B2791
P 8000 2850
F 0 "D5" H 7993 2595 50  0000 C CNN
F 1 "CV1" H 7993 2686 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 2850 50  0001 C CNN
F 3 "~" H 8000 2850 50  0001 C CNN
	1    8000 2850
	-1   0    0    1   
$EndComp
Wire Wire Line
	8150 2850 8200 2850
$Comp
L Device:LED D6
U 1 1 5E2B27A8
P 8000 3250
F 0 "D6" H 7993 2995 50  0000 C CNN
F 1 "CV2" H 7993 3086 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 3250 50  0001 C CNN
F 3 "~" H 8000 3250 50  0001 C CNN
	1    8000 3250
	-1   0    0    1   
$EndComp
Wire Wire Line
	8150 3250 8200 3250
Wire Wire Line
	8150 4050 8200 4050
$Comp
L Device:LED D8
U 1 1 5E2B27D6
P 8000 4050
F 0 "D8" H 7993 3795 50  0000 C CNN
F 1 "CV4" H 7993 3886 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 4050 50  0001 C CNN
F 3 "~" H 8000 4050 50  0001 C CNN
	1    8000 4050
	-1   0    0    1   
$EndComp
Wire Wire Line
	8150 3650 8200 3650
$Comp
L Device:LED D7
U 1 1 5E2B27BF
P 8000 3650
F 0 "D7" H 7993 3395 50  0000 C CNN
F 1 "CV3" H 7993 3486 50  0000 C CNN
F 2 "LEDs:LED_D3.0mm" H 8000 3650 50  0001 C CNN
F 3 "~" H 8000 3650 50  0001 C CNN
	1    8000 3650
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR021
U 1 1 5E2FD471
P 8200 2850
F 0 "#PWR021" H 8200 2600 50  0001 C CNN
F 1 "GND" V 8200 2650 50  0000 C CNN
F 2 "" H 8200 2850 50  0001 C CNN
F 3 "" H 8200 2850 50  0001 C CNN
	1    8200 2850
	0    -1   1    0   
$EndComp
$Comp
L power:GND #PWR022
U 1 1 5E2FD47C
P 8200 3250
F 0 "#PWR022" H 8200 3000 50  0001 C CNN
F 1 "GND" V 8200 3050 50  0000 C CNN
F 2 "" H 8200 3250 50  0001 C CNN
F 3 "" H 8200 3250 50  0001 C CNN
	1    8200 3250
	0    -1   1    0   
$EndComp
$Comp
L power:GND #PWR023
U 1 1 5E2FD487
P 8200 3650
F 0 "#PWR023" H 8200 3400 50  0001 C CNN
F 1 "GND" V 8200 3450 50  0000 C CNN
F 2 "" H 8200 3650 50  0001 C CNN
F 3 "" H 8200 3650 50  0001 C CNN
	1    8200 3650
	0    -1   1    0   
$EndComp
$Comp
L power:GND #PWR024
U 1 1 5E2FD492
P 8200 4050
F 0 "#PWR024" H 8200 3800 50  0001 C CNN
F 1 "GND" V 8200 3850 50  0000 C CNN
F 2 "" H 8200 4050 50  0001 C CNN
F 3 "" H 8200 4050 50  0001 C CNN
	1    8200 4050
	0    -1   1    0   
$EndComp
$Comp
L USB_B_Wurth:USB_B_Wurth J15
U 1 1 5DE73E83
P 5500 1450
F 0 "J15" H 5557 1917 50  0000 C CNN
F 1 "USB_B_Wurth" H 5557 1826 50  0000 C CNN
F 2 "Custom_Footprints:USB_B_Wurth" H 5650 1400 50  0001 C CNN
F 3 "https://docs.rs-online.com/9952/0900766b81023ab2.pdf" H 5650 1400 50  0001 C CNN
	1    5500 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 2000 4200 2000
Wire Wire Line
	4000 1200 4200 1200
Wire Wire Line
	4000 2800 4200 2800
Wire Wire Line
	4000 3600 4200 3600
NoConn ~ 1850 3650
Text GLabel 2800 4950 2    50   Input ~ 0
GATE5_OUT
Text GLabel 2800 6150 2    50   Input ~ 0
GATE6_OUT
Text GLabel 2800 5050 2    50   Input ~ 0
GATE7_OUT
Text GLabel 2800 6250 2    50   Input ~ 0
GATE8_OUT
Text GLabel 1400 5950 2    50   Input ~ 0
GATE1_OUT
Text GLabel 1400 6150 2    50   Input ~ 0
GATE2_OUT
Text GLabel 1400 6250 2    50   Input ~ 0
GATE3_OUT
Text GLabel 1400 6450 2    50   Input ~ 0
GATE4_OUT
Wire Wire Line
	7800 4050 7850 4050
Wire Wire Line
	7800 3650 7850 3650
Wire Wire Line
	7800 3250 7850 3250
Wire Wire Line
	7800 2850 7850 2850
Text GLabel 7800 1100 0    50   Input ~ 0
GATE1_LED_OUT
Text GLabel 7800 1500 0    50   Input ~ 0
GATE2_LED_OUT
Text GLabel 7800 1900 0    50   Input ~ 0
GATE3_LED_OUT
Text GLabel 7800 2300 0    50   Input ~ 0
GATE4_LED_OUT
Text GLabel 9300 1100 0    50   Input ~ 0
GATE5_LED_OUT
Text GLabel 9300 1500 0    50   Input ~ 0
GATE6_LED_OUT
Text GLabel 9300 1900 0    50   Input ~ 0
GATE7_LED_OUT
Text GLabel 9300 2300 0    50   Input ~ 0
GATE8_LED_OUT
Text GLabel 1400 5100 2    50   Input ~ 0
CV1_LED_OUT
Text GLabel 1400 5200 2    50   Input ~ 0
CV2_LED_OUT
Text GLabel 1400 5400 2    50   Input ~ 0
CV4_LED_OUT
Text GLabel 1400 5300 2    50   Input ~ 0
CV3_LED_OUT
Text GLabel 1400 5850 2    50   Input ~ 0
GATE1_LED_OUT
Text GLabel 1400 6050 2    50   Input ~ 0
GATE2_LED_OUT
Text GLabel 1400 6350 2    50   Input ~ 0
GATE3_LED_OUT
Text GLabel 1400 6550 2    50   Input ~ 0
GATE4_LED_OUT
Text GLabel 2800 4750 2    50   Input ~ 0
GATE5_LED_OUT
Text GLabel 2800 5950 2    50   Input ~ 0
GATE6_LED_OUT
Text GLabel 2800 5850 2    50   Input ~ 0
GATE7_LED_OUT
Text GLabel 2800 6050 2    50   Input ~ 0
GATE8_LED_OUT
Text GLabel 6350 2650 2    50   Input ~ 0
MIDI_DATA
Text GLabel 5500 2650 0    50   Input ~ 0
MIDI_VREF
Text GLabel 2800 6450 2    50   Input ~ 0
MIDI_DATA
Text GLabel 2800 6550 2    50   Input ~ 0
MIDI_VREF
Text GLabel 5700 3700 0    50   Input ~ 0
RESET_SWITCH
Text GLabel 2800 4650 2    50   Input ~ 0
RESET_SWITCH
Text GLabel 4000 1200 0    50   Input ~ 0
CV1_OUT
Text GLabel 4000 2000 0    50   Input ~ 0
CV2_OUT
Text GLabel 4000 2800 0    50   Input ~ 0
CV3_OUT
Text GLabel 4000 3600 0    50   Input ~ 0
CV4_OUT
Text GLabel 1400 4600 2    50   Input ~ 0
CV1_OUT
Text GLabel 1400 4700 2    50   Input ~ 0
CV2_OUT
Text GLabel 1400 4900 2    50   Input ~ 0
CV3_OUT
Text GLabel 1400 5000 2    50   Input ~ 0
CV4_OUT
$Comp
L power:GND #PWR01
U 1 1 5E745851
P 2800 4850
F 0 "#PWR01" H 2800 4600 50  0001 C CNN
F 1 "GND" V 2800 4650 50  0000 C CNN
F 2 "" H 2800 4850 50  0001 C CNN
F 3 "" H 2800 4850 50  0001 C CNN
	1    2800 4850
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5E983194
P 2800 6350
F 0 "#PWR06" H 2800 6100 50  0001 C CNN
F 1 "GND" V 2800 6150 50  0000 C CNN
F 2 "" H 2800 6350 50  0001 C CNN
F 3 "" H 2800 6350 50  0001 C CNN
	1    2800 6350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6200 3700 6300 3700
Wire Wire Line
	5700 3700 5800 3700
Text GLabel 7800 2850 0    50   Input ~ 0
CV1_LED_OUT
Text GLabel 7800 3250 0    50   Input ~ 0
CV2_LED_OUT
Text GLabel 7800 4050 0    50   Input ~ 0
CV4_LED_OUT
Text GLabel 7800 3650 0    50   Input ~ 0
CV3_LED_OUT
Text GLabel 2800 5350 2    50   Input ~ 0
USB_VBUS
Text GLabel 2800 5150 2    50   Input ~ 0
USB_DP
Text GLabel 2800 5250 2    50   Input ~ 0
USB_DM
$Comp
L Connector:Conn_01x08_Female J21
U 1 1 5DE6CBEA
P 1200 6150
F 0 "J21" H 1050 6750 50  0000 L CNN
F 1 "Conn_01x08_Female" H 750 6600 50  0000 L CNN
F 2 "Socket_Strips:Socket_Strip_Straight_1x08_Pitch2.54mm" H 1200 6150 50  0001 C CNN
F 3 "~" H 1200 6150 50  0001 C CNN
	1    1200 6150
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Female J22
U 1 1 5DE75999
P 2600 4950
F 0 "J22" H 2450 5550 50  0000 L CNN
F 1 "Conn_01x08_Female" H 2150 5400 50  0000 L CNN
F 2 "Socket_Strips:Socket_Strip_Straight_1x08_Pitch2.54mm" H 2600 4950 50  0001 C CNN
F 3 "~" H 2600 4950 50  0001 C CNN
	1    2600 4950
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Female J23
U 1 1 5DE79E27
P 2600 6150
F 0 "J23" H 2450 6750 50  0000 L CNN
F 1 "Conn_01x08_Female" H 2150 6600 50  0000 L CNN
F 2 "Socket_Strips:Socket_Strip_Straight_1x08_Pitch2.54mm" H 2600 6150 50  0001 C CNN
F 3 "~" H 2600 6150 50  0001 C CNN
	1    2600 6150
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x09_Female J20
U 1 1 5DD622C7
P 1200 5000
F 0 "J20" H 1092 5585 50  0000 C CNN
F 1 "Conn_01x09_Female" H 1092 5494 50  0000 C CNN
F 2 "Socket_Strips:Socket_Strip_Straight_1x09_Pitch2.54mm" H 1200 5000 50  0001 C CNN
F 3 "~" H 1200 5000 50  0001 C CNN
	1    1200 5000
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5DD65DC5
P 1400 4800
F 0 "#PWR0101" H 1400 4550 50  0001 C CNN
F 1 "GND" V 1400 4600 50  0000 C CNN
F 2 "" H 1400 4800 50  0001 C CNN
F 3 "" H 1400 4800 50  0001 C CNN
	1    1400 4800
	0    -1   -1   0   
$EndComp
NoConn ~ 1600 1100
NoConn ~ 2950 1100
NoConn ~ 4200 1100
NoConn ~ 4200 1900
NoConn ~ 4200 2700
NoConn ~ 4200 3500
NoConn ~ 2950 3500
NoConn ~ 2950 2700
NoConn ~ 2950 1900
NoConn ~ 1600 1900
NoConn ~ 1550 2700
NoConn ~ 1550 3450
$EndSCHEMATC
