EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 4
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	3200 2600 3200 2350
Wire Wire Line
	3200 3350 3200 2800
Wire Wire Line
	3850 2450 4250 2450
$Comp
L Device:R_Small R?
U 1 1 63F9B28A
P 3950 2800
AR Path="/66CBFED4/63F9B28A" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B28A" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B28A" Ref="R?"  Part="1" 
AR Path="/64D82E7B/63F9B28A" Ref="R64"  Part="1" 
F 0 "R64" H 4009 2846 50  0000 L CNN
F 1 "47K" H 4009 2755 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3950 2800 50  0001 C CNN
F 3 "~" H 3950 2800 50  0001 C CNN
	1    3950 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4650 2950 4650 3350
Wire Wire Line
	4750 3350 4650 3350
Connection ~ 4650 3350
$Comp
L Device:C_Small C?
U 1 1 63F9B2AD
P 5200 2050
AR Path="/66CBFED4/63F9B2AD" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2AD" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2AD" Ref="C?"  Part="1" 
AR Path="/64D82E7B/63F9B2AD" Ref="C13"  Part="1" 
F 0 "C13" V 4971 2050 50  0000 C CNN
F 1 "104" V 5062 2050 50  0000 C CNN
F 2 "lc_lib:0805_C" H 5200 2050 50  0001 C CNN
F 3 "~" H 5200 2050 50  0001 C CNN
	1    5200 2050
	-1   0    0    1   
$EndComp
$Comp
L Device:L_Core_Ferrite L?
U 1 1 63F9B2B3
P 5900 2250
AR Path="/66CBFED4/63F9B2B3" Ref="L?"  Part="1" 
AR Path="/63E76FBF/63F9B2B3" Ref="L?"  Part="1" 
AR Path="/63F91570/63F9B2B3" Ref="L?"  Part="1" 
AR Path="/64D82E7B/63F9B2B3" Ref="L2"  Part="1" 
F 0 "L2" V 6125 2250 50  0000 C CNN
F 1 "22uH 2A" V 6034 2250 50  0000 C CNN
F 2 "Inductor_SMD:L_Taiyo-Yuden_NR-50xx" H 5900 2250 50  0001 C CNN
F 3 "~" H 5900 2250 50  0001 C CNN
	1    5900 2250
	0    -1   -1   0   
$EndComp
$Comp
L MySchLib:D_Schottky_Small D?
U 1 1 63F9B2BF
P 5600 2950
AR Path="/66CBFED4/63F9B2BF" Ref="D?"  Part="1" 
AR Path="/63E76FBF/63F9B2BF" Ref="D?"  Part="1" 
AR Path="/63F91570/63F9B2BF" Ref="D?"  Part="1" 
AR Path="/64D82E7B/63F9B2BF" Ref="D9"  Part="1" 
F 0 "D9" V 5554 3018 50  0000 L CNN
F 1 "SS34" V 5645 3018 50  0000 L CNN
F 2 "lc_lib:SMA(DO-214AC)_S1" V 5600 2950 50  0001 C CNN
F 3 "~" V 5600 2950 50  0001 C CNN
	1    5600 2950
	0    1    1    0   
$EndComp
Wire Wire Line
	5600 3050 5600 3350
Connection ~ 4750 3350
$Comp
L Device:R_Small R?
U 1 1 63F9B2C9
P 6350 2600
AR Path="/66CBFED4/63F9B2C9" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B2C9" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B2C9" Ref="R?"  Part="1" 
AR Path="/64D82E7B/63F9B2C9" Ref="R65"  Part="1" 
F 0 "R65" H 6409 2646 50  0000 L CNN
F 1 "150K" H 6409 2555 50  0000 L CNN
F 2 "lc_lib:0805_R" H 6350 2600 50  0001 C CNN
F 3 "~" H 6350 2600 50  0001 C CNN
	1    6350 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 63F9B2CF
P 6350 3050
AR Path="/66CBFED4/63F9B2CF" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B2CF" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B2CF" Ref="R?"  Part="1" 
AR Path="/64D82E7B/63F9B2CF" Ref="R66"  Part="1" 
F 0 "R66" H 6409 3096 50  0000 L CNN
F 1 "10K" H 6409 3005 50  0000 L CNN
F 2 "lc_lib:0805_R" H 6350 3050 50  0001 C CNN
F 3 "~" H 6350 3050 50  0001 C CNN
	1    6350 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 3150 6350 3350
Wire Wire Line
	4750 3350 5600 3350
Connection ~ 5600 3350
Wire Wire Line
	6750 2900 6750 3350
Wire Wire Line
	6750 3350 6350 3350
$Comp
L Device:C_Small C?
U 1 1 63F9B2E5
P 6750 2800
AR Path="/66CBFED4/63F9B2E5" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2E5" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2E5" Ref="C?"  Part="1" 
AR Path="/64D82E7B/63F9B2E5" Ref="C23"  Part="1" 
F 0 "C23" H 6550 2800 50  0000 L CNN
F 1 "10uF" H 6550 2700 50  0000 L CNN
F 2 "lc_lib:1210_C" H 6750 2800 50  0001 C CNN
F 3 "~" H 6750 2800 50  0001 C CNN
	1    6750 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 63F9B2EB
P 3200 2700
AR Path="/66CBFED4/63F9B2EB" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2EB" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2EB" Ref="C?"  Part="1" 
AR Path="/64D82E7B/63F9B2EB" Ref="C6"  Part="1" 
F 0 "C6" H 3000 2700 50  0000 L CNN
F 1 "10uF 63V" H 2800 2600 50  0000 L CNN
F 2 "lc_lib:1210_C" H 3200 2700 50  0001 C CNN
F 3 "~" H 3200 2700 50  0001 C CNN
	1    3200 2700
	1    0    0    -1  
$EndComp
Text HLabel 2700 2350 0    50   Input ~ 0
Buck_in
Text HLabel 7400 2250 2    50   Output ~ 0
Buck_out
Text HLabel 2900 3550 3    50   UnSpc ~ 0
Buck_GND
Wire Wire Line
	2900 3350 2900 3550
Connection ~ 3950 3350
Wire Wire Line
	4750 2950 4750 3350
Wire Wire Line
	2900 3350 3200 3350
Connection ~ 3200 2350
Connection ~ 3200 3350
Wire Wire Line
	2700 2350 3200 2350
Wire Wire Line
	3850 2450 3850 2350
Connection ~ 3850 2350
Wire Wire Line
	3850 2350 4250 2350
$Comp
L MySchLib:LMR16020 U11
U 1 1 64D8FDF8
P 4700 2400
F 0 "U11" H 4700 2775 50  0000 C CNN
F 1 "LMR16020" H 4700 2684 50  0000 C CNN
F 2 "Package_SO:SOIC-8-1EP_3.9x4.9mm_P1.27mm_EP2.514x3.2mm_ThermalVias" H 4700 1700 50  0001 C CNN
F 3 "" H 4700 1700 50  0001 C CNN
	1    4700 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 3350 4650 3350
Wire Wire Line
	4250 2550 3950 2550
Wire Wire Line
	3950 2550 3950 2700
Wire Wire Line
	3950 2900 3950 3350
Wire Wire Line
	4700 2700 4700 2800
Wire Wire Line
	4700 2950 4650 2950
Wire Wire Line
	4750 2950 4700 2950
Connection ~ 4700 2950
Wire Wire Line
	5200 2150 5200 2250
Wire Wire Line
	5200 2250 5150 2250
Wire Wire Line
	5200 1950 4250 1950
Wire Wire Line
	4250 1950 4250 2250
Wire Wire Line
	5200 2250 5600 2250
Wire Wire Line
	5600 2250 5600 2850
Connection ~ 5200 2250
Connection ~ 5600 2250
Wire Wire Line
	5750 2250 5600 2250
Wire Wire Line
	6350 2700 6350 2850
Connection ~ 6750 2250
Wire Wire Line
	6750 2250 6750 2700
Wire Wire Line
	6350 2500 6350 2250
$Comp
L Device:C_Small C?
U 1 1 64DA35BB
P 6150 2600
AR Path="/66CBFED4/64DA35BB" Ref="C?"  Part="1" 
AR Path="/63E76FBF/64DA35BB" Ref="C?"  Part="1" 
AR Path="/63F91570/64DA35BB" Ref="C?"  Part="1" 
AR Path="/64D82E7B/64DA35BB" Ref="C15"  Part="1" 
F 0 "C15" V 5921 2600 50  0000 C CNN
F 1 "330pF" V 6012 2600 50  0000 C CNN
F 2 "lc_lib:0805_C" H 6150 2600 50  0001 C CNN
F 3 "~" H 6150 2600 50  0001 C CNN
	1    6150 2600
	1    0    0    -1  
$EndComp
Connection ~ 6350 2250
Wire Wire Line
	6350 2250 6750 2250
Connection ~ 6350 3350
Wire Wire Line
	5600 3350 6350 3350
Wire Wire Line
	6050 2250 6150 2250
Wire Wire Line
	6150 2500 6150 2250
Connection ~ 6150 2250
Wire Wire Line
	6150 2250 6350 2250
Wire Wire Line
	6150 2700 6150 2850
Wire Wire Line
	6150 2850 6350 2850
Connection ~ 6350 2850
Wire Wire Line
	6350 2850 6350 2950
Wire Wire Line
	5150 2550 5800 2550
Wire Wire Line
	5800 2550 5800 2850
Wire Wire Line
	5800 2850 6150 2850
Connection ~ 6150 2850
Connection ~ 6750 3350
Wire Wire Line
	6750 3350 6950 3350
Wire Wire Line
	6750 2250 6950 2250
$Comp
L Device:CP_Small C29
U 1 1 64DB070F
P 6950 2800
F 0 "C29" H 7038 2846 50  0000 L CNN
F 1 "47uF 25V" H 7038 2755 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_6.3x5.9" H 6950 2800 50  0001 C CNN
F 3 "~" H 6950 2800 50  0001 C CNN
	1    6950 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 2700 6950 2250
Connection ~ 6950 2250
Wire Wire Line
	6950 2250 7400 2250
Wire Wire Line
	6950 2900 6950 3350
Wire Wire Line
	5150 2350 5300 2350
Wire Wire Line
	5300 2350 5300 2800
Wire Wire Line
	5300 2800 4700 2800
Connection ~ 4700 2800
Wire Wire Line
	4700 2800 4700 2950
Wire Wire Line
	3200 3350 3650 3350
Wire Wire Line
	3200 2350 3850 2350
$Comp
L Device:R_POT RV1
U 1 1 639FBEE0
P 3650 3050
F 0 "RV1" H 3581 3096 50  0000 R CNN
F 1 "10K" H 3581 3005 50  0000 R CNN
F 2 "Potentiometer_SMD:Potentiometer_Bourns_3224W_Vertical" H 3650 3050 50  0001 C CNN
F 3 "~" H 3650 3050 50  0001 C CNN
	1    3650 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 3200 3650 3250
Connection ~ 3650 3350
Wire Wire Line
	3650 3350 3950 3350
$Comp
L Device:R_Small R?
U 1 1 639FD5E8
P 3650 2700
AR Path="/66CBFED4/639FD5E8" Ref="R?"  Part="1" 
AR Path="/63E76FBF/639FD5E8" Ref="R?"  Part="1" 
AR Path="/63F91570/639FD5E8" Ref="R?"  Part="1" 
AR Path="/64D82E7B/639FD5E8" Ref="R3"  Part="1" 
F 0 "R3" H 3709 2746 50  0000 L CNN
F 1 "47K" H 3709 2655 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3650 2700 50  0001 C CNN
F 3 "~" H 3650 2700 50  0001 C CNN
	1    3650 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 2800 3650 2900
Wire Wire Line
	3650 2600 3650 2550
Wire Wire Line
	3650 2550 3950 2550
Connection ~ 3950 2550
Wire Wire Line
	3800 3050 3800 3250
Wire Wire Line
	3800 3250 3650 3250
Connection ~ 3650 3250
Wire Wire Line
	3650 3250 3650 3350
Text Notes 3550 3550 0    50   ~ 0
RT =22.5k-19.4k\nf = 1.04MHz-1.19MHz
$EndSCHEMATC