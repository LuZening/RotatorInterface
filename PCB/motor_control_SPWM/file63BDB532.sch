EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 5
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
L Amplifier_Operational:LM358 U?
U 1 1 63BDBFCF
P 3950 2900
F 0 "U?" H 4000 3150 50  0000 C CNN
F 1 "LM358" H 4050 3050 50  0000 C CNN
F 2 "" H 3950 2900 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2904-n.pdf" H 3950 2900 50  0001 C CNN
	1    3950 2900
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM358 U?
U 2 1 63BDD85D
P 4000 4200
F 0 "U?" H 4000 4567 50  0000 C CNN
F 1 "LM358" H 4000 4476 50  0000 C CNN
F 2 "" H 4000 4200 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2904-n.pdf" H 4000 4200 50  0001 C CNN
	2    4000 4200
	1    0    0    -1  
$EndComp
$Comp
L Amplifier_Operational:LM358 U?
U 3 1 63BDF913
P 3950 2900
F 0 "U?" H 3908 2946 50  0001 L CNN
F 1 "LM358" H 3908 2855 50  0001 L CNN
F 2 "" H 3950 2900 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm2904-n.pdf" H 3950 2900 50  0001 C CNN
	3    3950 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R?
U 1 1 63BF30AE
P 2450 3400
F 0 "R?" H 2518 3446 50  0000 L CNN
F 1 "100m" H 2518 3355 50  0000 L CNN
F 2 "" V 2490 3390 50  0001 C CNN
F 3 "~" H 2450 3400 50  0001 C CNN
	1    2450 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 3250 2450 2800
Wire Wire Line
	2450 2800 3050 2800
$Comp
L Device:R_Small R?
U 1 1 63BF41A3
P 3150 2800
F 0 "R?" V 2954 2800 50  0000 C CNN
F 1 "1K" V 3045 2800 50  0000 C CNN
F 2 "" H 3150 2800 50  0001 C CNN
F 3 "~" H 3150 2800 50  0001 C CNN
	1    3150 2800
	0    1    1    0   
$EndComp
Wire Wire Line
	3250 2800 3650 2800
$Comp
L Device:R_Small R?
U 1 1 63BF6430
P 3450 3200
F 0 "R?" H 3509 3246 50  0000 L CNN
F 1 "1K" H 3509 3155 50  0000 L CNN
F 2 "" H 3450 3200 50  0001 C CNN
F 3 "~" H 3450 3200 50  0001 C CNN
	1    3450 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 3100 3450 3000
Wire Wire Line
	3450 3000 3650 3000
$EndSCHEMATC
