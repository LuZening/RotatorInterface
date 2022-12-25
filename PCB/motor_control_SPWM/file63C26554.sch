EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 5
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GNDS #PWR060
U 1 1 63CD5BB3
P 4500 3550
F 0 "#PWR060" H 4500 3300 50  0001 C CNN
F 1 "GNDS" H 4505 3377 50  0000 C CNN
F 2 "" H 4500 3550 50  0001 C CNN
F 3 "" H 4500 3550 50  0001 C CNN
	1    4500 3550
	1    0    0    -1  
$EndComp
Text GLabel 4500 2400 1    50   Input ~ 0
+5VISO
$Comp
L Amplifier_Operational:LM324 U16
U 1 1 63CDD251
P 4600 2900
F 0 "U16" H 4650 3150 50  0000 C CNN
F 1 "LM324" H 4700 3050 50  0000 C CNN
F 2 "lc_lib:SOIC-14_150MIL" H 4550 3000 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4650 3100 50  0001 C CNN
	1    4600 2900
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U16
U 2 1 63CDECCB
P 4550 4300
F 0 "U16" H 4550 4667 50  0000 C CNN
F 1 "LM324" H 4550 4576 50  0000 C CNN
F 2 "lc_lib:SOIC-14_150MIL" H 4500 4400 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4600 4500 50  0001 C CNN
	2    4550 4300
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U16
U 3 1 63CE0042
P 4550 5400
F 0 "U16" H 4550 5767 50  0000 C CNN
F 1 "LM324" H 4550 5676 50  0000 C CNN
F 2 "lc_lib:SOIC-14_150MIL" H 4500 5500 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4600 5600 50  0001 C CNN
	3    4550 5400
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U16
U 4 1 63CE146F
P 4550 6400
F 0 "U16" H 4550 6767 50  0000 C CNN
F 1 "LM324" H 4550 6676 50  0000 C CNN
F 2 "lc_lib:SOIC-14_150MIL" H 4500 6500 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4600 6600 50  0001 C CNN
	4    4550 6400
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM324 U16
U 5 1 63CE22F2
P 4600 2900
F 0 "U16" H 4558 2946 50  0001 L CNN
F 1 "LM324" H 4558 2855 50  0001 L CNN
F 2 "lc_lib:SOIC-14_150MIL" H 4550 3000 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2902-n.pdf" H 4650 3100 50  0001 C CNN
	5    4600 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 3200 4500 3450
Wire Wire Line
	4300 3000 4050 3000
Wire Wire Line
	4050 3000 4050 3350
Wire Wire Line
	4050 3350 4950 3350
Wire Wire Line
	4950 3350 4950 2900
Wire Wire Line
	4950 2900 4900 2900
Connection ~ 4950 2900
Text Label 5050 2900 0    50   ~ 0
VMID
Text Label 3300 4400 0    50   ~ 0
VMID
$Comp
L Device:R_Small R45
U 1 1 63CE660B
P 3700 4400
F 0 "R45" V 3800 4300 50  0000 C CNN
F 1 "1K" V 3800 4450 50  0000 C CNN
F 2 "lc_lib:0805_R" H 3700 4400 50  0001 C CNN
F 3 "~" H 3700 4400 50  0001 C CNN
	1    3700 4400
	0    1    1    0   
$EndComp
Wire Wire Line
	3600 4400 3300 4400
Wire Wire Line
	3800 4400 4000 4400
$Comp
L Device:R_Small R48
U 1 1 63CE7315
P 4350 4600
F 0 "R48" V 4250 4750 50  0000 C CNN
F 1 "1K" V 4250 4600 50  0000 C CNN
F 2 "lc_lib:0805_R" H 4350 4600 50  0001 C CNN
F 3 "~" H 4350 4600 50  0001 C CNN
	1    4350 4600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4250 4600 4000 4600
Wire Wire Line
	4000 4600 4000 4400
Connection ~ 4000 4400
Wire Wire Line
	4000 4400 4250 4400
Wire Wire Line
	4450 4600 4950 4600
Wire Wire Line
	4950 4600 4950 4300
Wire Wire Line
	4950 4300 4850 4300
Wire Wire Line
	3700 2800 4300 2800
$Comp
L Device:R_Small R42
U 1 1 63C2C699
P 3700 2600
F 0 "R42" H 3759 2646 50  0000 L CNN
F 1 "51K" H 3759 2555 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3700 2600 50  0001 C CNN
F 3 "~" H 3700 2600 50  0001 C CNN
	1    3700 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R43
U 1 1 63C2C7E0
P 3700 2950
F 0 "R43" H 3759 2996 50  0000 L CNN
F 1 "47K" H 3759 2905 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3700 2950 50  0001 C CNN
F 3 "~" H 3700 2950 50  0001 C CNN
	1    3700 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3700 2850 3700 2800
Connection ~ 3700 2800
Wire Wire Line
	3700 2800 3700 2700
$Comp
L power:GNDS #PWR059
U 1 1 63C2D9DA
P 3700 3200
F 0 "#PWR059" H 3700 2950 50  0001 C CNN
F 1 "GNDS" H 3705 3027 50  0000 C CNN
F 2 "" H 3700 3200 50  0001 C CNN
F 3 "" H 3700 3200 50  0001 C CNN
	1    3700 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3700 3200 3700 3150
Text GLabel 3700 2400 1    50   Input ~ 0
+5VISO
Wire Wire Line
	3700 2400 3700 2500
$Comp
L Device:C_Small C17
U 1 1 63CC6612
P 3500 3000
F 0 "C17" H 3700 3000 50  0000 R CNN
F 1 "104" H 3700 2900 50  0000 R CNN
F 2 "lc_lib:0805_C" H 3500 3000 50  0001 C CNN
F 3 "~" H 3500 3000 50  0001 C CNN
	1    3500 3000
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3500 2900 3500 2800
Wire Wire Line
	3500 2800 3700 2800
Wire Wire Line
	3500 3100 3500 3150
Wire Wire Line
	3500 3150 3700 3150
Connection ~ 3700 3150
Wire Wire Line
	3700 3150 3700 3050
Text HLabel 2950 4200 0    50   Input ~ 0
VISense1
Text HLabel 2950 5300 0    50   Input ~ 0
VISense2
$Comp
L Device:R_Small R47
U 1 1 63D06B15
P 3700 5500
F 0 "R47" V 3800 5400 50  0000 C CNN
F 1 "1K" V 3800 5550 50  0000 C CNN
F 2 "lc_lib:0805_R" H 3700 5500 50  0001 C CNN
F 3 "~" H 3700 5500 50  0001 C CNN
	1    3700 5500
	0    1    1    0   
$EndComp
Text Label 3350 5500 0    50   ~ 0
VMID
Wire Wire Line
	3350 5500 3600 5500
$Comp
L Device:R_Small R49
U 1 1 63D075D8
P 4500 5700
F 0 "R49" V 4400 5850 50  0000 C CNN
F 1 "1K" V 4400 5700 50  0000 C CNN
F 2 "lc_lib:0805_R" H 4500 5700 50  0001 C CNN
F 3 "~" H 4500 5700 50  0001 C CNN
	1    4500 5700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3800 5500 4050 5500
Wire Wire Line
	4400 5700 4050 5700
Wire Wire Line
	4050 5700 4050 5500
Connection ~ 4050 5500
Wire Wire Line
	4050 5500 4250 5500
Wire Wire Line
	4600 5700 4950 5700
Wire Wire Line
	4950 5700 4950 5400
Wire Wire Line
	4950 5400 4850 5400
Wire Wire Line
	4950 5400 5050 5400
Connection ~ 4950 5400
Wire Wire Line
	4950 4300 5050 4300
Connection ~ 4950 4300
Text HLabel 5050 4300 2    50   Output ~ 0
VISenseDec1
Text HLabel 5050 5400 2    50   Output ~ 0
VISenseDec2
Text Notes 5050 4200 0    50   ~ 0
Decenterized
Text Notes 3900 2800 0    50   ~ 0
2.4V
Wire Wire Line
	4500 2400 4500 2500
$Comp
L Device:C_Small C18
U 1 1 63D120EA
P 4750 2500
F 0 "C18" H 4950 2500 50  0000 R CNN
F 1 "104" H 4950 2400 50  0000 R CNN
F 2 "lc_lib:0805_C" H 4750 2500 50  0001 C CNN
F 3 "~" H 4750 2500 50  0001 C CNN
	1    4750 2500
	0    1    -1   0   
$EndComp
Wire Wire Line
	4650 2500 4500 2500
Connection ~ 4500 2500
Wire Wire Line
	4500 2500 4500 2600
$Comp
L power:GNDS #PWR061
U 1 1 63D1402A
P 4950 2550
F 0 "#PWR061" H 4950 2300 50  0001 C CNN
F 1 "GNDS" H 4955 2377 50  0000 C CNN
F 2 "" H 4950 2550 50  0001 C CNN
F 3 "" H 4950 2550 50  0001 C CNN
	1    4950 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2550 4950 2500
Wire Wire Line
	4950 2500 4850 2500
Wire Wire Line
	2950 4200 3050 4200
Wire Wire Line
	2950 5300 3400 5300
Wire Wire Line
	4250 6300 4100 6300
Wire Wire Line
	4100 6300 4100 6800
$Comp
L power:GNDS #PWR0108
U 1 1 64A0F01D
P 4100 6800
F 0 "#PWR0108" H 4100 6550 50  0001 C CNN
F 1 "GNDS" H 4105 6627 50  0000 C CNN
F 2 "" H 4100 6800 50  0001 C CNN
F 3 "" H 4100 6800 50  0001 C CNN
	1    4100 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 6500 4250 6700
Wire Wire Line
	4250 6700 4900 6700
Wire Wire Line
	4900 6700 4900 6400
Wire Wire Line
	4900 6400 4850 6400
$Comp
L Connector:TestPoint TP13
U 1 1 64BBDC06
P 5450 2900
F 0 "TP13" H 5508 3018 50  0000 L CNN
F 1 "VMID" H 5508 2927 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 5650 2900 50  0001 C CNN
F 3 "~" H 5650 2900 50  0001 C CNN
	1    5450 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2900 5450 2900
$Comp
L Connector:TestPoint TP12
U 1 1 64BC149E
P 5200 3450
F 0 "TP12" H 5258 3568 50  0000 L CNN
F 1 "GNDS" H 5258 3477 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 5400 3450 50  0001 C CNN
F 3 "~" H 5400 3450 50  0001 C CNN
	1    5200 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 3450 4500 3450
Connection ~ 4500 3450
Wire Wire Line
	4500 3450 4500 3550
$Comp
L Connector:TestPoint TP10
U 1 1 64BC3325
P 4950 4050
F 0 "TP10" H 5008 4168 50  0000 L CNN
F 1 "VID1" H 5008 4077 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 5150 4050 50  0001 C CNN
F 3 "~" H 5150 4050 50  0001 C CNN
	1    4950 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 4050 4950 4300
$Comp
L Connector:TestPoint TP11
U 1 1 64BC5DBD
P 4950 5250
F 0 "TP11" H 5008 5368 50  0000 L CNN
F 1 "VID2" H 5008 5277 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 5150 5250 50  0001 C CNN
F 3 "~" H 5150 5250 50  0001 C CNN
	1    4950 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 5250 4950 5400
$Comp
L Connector:TestPoint TP8
U 1 1 64BC7BC4
P 3050 4100
F 0 "TP8" H 3108 4218 50  0000 L CNN
F 1 "VIS1" H 3108 4127 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 3250 4100 50  0001 C CNN
F 3 "~" H 3250 4100 50  0001 C CNN
	1    3050 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 4100 3050 4200
$Comp
L Connector:TestPoint TP9
U 1 1 64BC9079
P 3400 5250
F 0 "TP9" H 3458 5368 50  0000 L CNN
F 1 "VIS2" H 3458 5277 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.0mm" H 3600 5250 50  0001 C CNN
F 3 "~" H 3600 5250 50  0001 C CNN
	1    3400 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 5250 3400 5300
Connection ~ 3400 5300
Wire Wire Line
	3400 5300 3600 5300
Connection ~ 3050 4200
Wire Wire Line
	3050 4200 3600 4200
$Comp
L Device:R_Small R22
U 1 1 638CAB19
P 3700 4200
F 0 "R22" V 3800 4100 50  0000 C CNN
F 1 "1K" V 3800 4250 50  0000 C CNN
F 2 "lc_lib:0805_R" H 3700 4200 50  0001 C CNN
F 3 "~" H 3700 4200 50  0001 C CNN
	1    3700 4200
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 4200 4000 4200
$Comp
L Device:R_Small R24
U 1 1 638CB355
P 4000 4000
F 0 "R24" V 3900 4150 50  0000 C CNN
F 1 "1K" V 3900 4000 50  0000 C CNN
F 2 "lc_lib:0805_R" H 4000 4000 50  0001 C CNN
F 3 "~" H 4000 4000 50  0001 C CNN
	1    4000 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 4100 4000 4200
Connection ~ 4000 4200
Wire Wire Line
	4000 4200 4250 4200
$Comp
L power:GNDS #PWR0109
U 1 1 638CCC50
P 4150 3900
F 0 "#PWR0109" H 4150 3650 50  0001 C CNN
F 1 "GNDS" H 4155 3727 50  0000 C CNN
F 2 "" H 4150 3900 50  0001 C CNN
F 3 "" H 4150 3900 50  0001 C CNN
	1    4150 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 3900 4150 3850
Wire Wire Line
	4150 3850 4000 3850
Wire Wire Line
	4000 3850 4000 3900
$Comp
L Device:R_Small R23
U 1 1 638CE2EE
P 3700 5300
F 0 "R23" V 3600 5450 50  0000 C CNN
F 1 "1K" V 3600 5300 50  0000 C CNN
F 2 "lc_lib:0805_R" H 3700 5300 50  0001 C CNN
F 3 "~" H 3700 5300 50  0001 C CNN
	1    3700 5300
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 5300 4050 5300
$Comp
L Device:R_Small R25
U 1 1 638CE819
P 4050 5200
F 0 "R25" V 3950 5350 50  0000 C CNN
F 1 "1K" V 3950 5200 50  0000 C CNN
F 2 "lc_lib:0805_R" H 4050 5200 50  0001 C CNN
F 3 "~" H 4050 5200 50  0001 C CNN
	1    4050 5200
	1    0    0    -1  
$EndComp
Connection ~ 4050 5300
Wire Wire Line
	4050 5300 4250 5300
$Comp
L power:GNDS #PWR0110
U 1 1 638CECC0
P 4200 5000
F 0 "#PWR0110" H 4200 4750 50  0001 C CNN
F 1 "GNDS" H 4205 4827 50  0000 C CNN
F 2 "" H 4200 5000 50  0001 C CNN
F 3 "" H 4200 5000 50  0001 C CNN
	1    4200 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 5000 4050 5000
Wire Wire Line
	4050 5000 4050 5100
$EndSCHEMATC
