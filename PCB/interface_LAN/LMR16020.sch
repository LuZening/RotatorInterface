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
	2900 2600 2900 2350
$Comp
L Device:R_Small R?
U 1 1 63F9B266
P 3550 2550
AR Path="/66CBFED4/63F9B266" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B266" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B266" Ref="R?"  Part="1" 
F 0 "R?" H 3609 2596 50  0000 L CNN
F 1 "169K" H 3609 2505 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3550 2550 50  0001 C CNN
F 3 "~" H 3550 2550 50  0001 C CNN
	1    3550 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 63F9B26C
P 3550 3050
AR Path="/66CBFED4/63F9B26C" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B26C" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B26C" Ref="R?"  Part="1" 
F 0 "R?" H 3609 3096 50  0000 L CNN
F 1 "14K" H 3609 3005 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3550 3050 50  0001 C CNN
F 3 "~" H 3550 3050 50  0001 C CNN
	1    3550 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 3150 3550 3350
Connection ~ 2900 3350
Wire Wire Line
	2900 3350 2900 2800
Wire Wire Line
	3550 2450 3550 2350
Connection ~ 3550 2350
Wire Wire Line
	3550 2350 4250 2350
Wire Wire Line
	3850 2450 4250 2450
$Comp
L Device:C_Small C?
U 1 1 63F9B27E
P 3800 3050
AR Path="/66CBFED4/63F9B27E" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B27E" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B27E" Ref="C?"  Part="1" 
F 0 "C?" H 3700 3300 50  0000 L CNN
F 1 "10pF" H 3600 3200 50  0000 L CNN
F 2 "lc_lib:0805_C" H 3800 3050 50  0001 C CNN
F 3 "~" H 3800 3050 50  0001 C CNN
	1    3800 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 63F9B284
P 3950 3050
AR Path="/66CBFED4/63F9B284" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B284" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B284" Ref="C?"  Part="1" 
F 0 "C?" H 3850 3150 50  0000 L CNN
F 1 "102" H 3800 2900 50  0000 L CNN
F 2 "lc_lib:0805_C" H 3950 3050 50  0001 C CNN
F 3 "~" H 3950 3050 50  0001 C CNN
	1    3950 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 63F9B28A
P 3950 2800
AR Path="/66CBFED4/63F9B28A" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B28A" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B28A" Ref="R?"  Part="1" 
F 0 "R?" H 4009 2846 50  0000 L CNN
F 1 "33K" H 4009 2755 50  0000 L CNN
F 2 "lc_lib:0805_R" H 3950 2800 50  0001 C CNN
F 3 "~" H 3950 2800 50  0001 C CNN
	1    3950 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 2950 3950 2900
Wire Wire Line
	3950 2700 3950 2600
Wire Wire Line
	3800 2950 3800 2600
Wire Wire Line
	3800 2600 3950 2600
Connection ~ 3950 2600
Connection ~ 3550 3350
$Comp
L Device:C_Small C?
U 1 1 63F9B29C
P 4150 3050
AR Path="/66CBFED4/63F9B29C" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B29C" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B29C" Ref="C?"  Part="1" 
F 0 "C?" H 3950 3050 50  0000 L CNN
F 1 "8.2pF" H 3900 2950 50  0000 L CNN
F 2 "lc_lib:0805_C" H 4150 3050 50  0001 C CNN
F 3 "~" H 4150 3050 50  0001 C CNN
	1    4150 3050
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4150 3150 4150 3350
Wire Wire Line
	4650 2950 4650 3350
Wire Wire Line
	4750 2950 4750 3150
Wire Wire Line
	4750 3350 4650 3350
Connection ~ 4650 3350
$Comp
L Device:C_Small C?
U 1 1 63F9B2AD
P 5400 2350
AR Path="/66CBFED4/63F9B2AD" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2AD" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2AD" Ref="C?"  Part="1" 
F 0 "C?" V 5171 2350 50  0000 C CNN
F 1 "104" V 5262 2350 50  0000 C CNN
F 2 "lc_lib:0805_C" H 5400 2350 50  0001 C CNN
F 3 "~" H 5400 2350 50  0001 C CNN
	1    5400 2350
	0    1    1    0   
$EndComp
$Comp
L Device:L_Core_Ferrite L?
U 1 1 63F9B2B3
P 5900 2350
AR Path="/66CBFED4/63F9B2B3" Ref="L?"  Part="1" 
AR Path="/63E76FBF/63F9B2B3" Ref="L?"  Part="1" 
AR Path="/63F91570/63F9B2B3" Ref="L?"  Part="1" 
F 0 "L?" V 6125 2350 50  0000 C CNN
F 1 "47uH 1A" V 6034 2350 50  0000 C CNN
F 2 "MyPCBLib:L_SMD4030_hand" H 5900 2350 50  0001 C CNN
F 3 "~" H 5900 2350 50  0001 C CNN
	1    5900 2350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5750 2350 5600 2350
Wire Wire Line
	5250 2350 5300 2350
Wire Wire Line
	5500 2350 5600 2350
Wire Wire Line
	5600 2350 5600 2550
Connection ~ 5600 2550
Wire Wire Line
	5600 2550 5300 2550
$Comp
L MySchLib:D_Schottky_Small D?
U 1 1 63F9B2BF
P 5600 2950
AR Path="/66CBFED4/63F9B2BF" Ref="D?"  Part="1" 
AR Path="/63E76FBF/63F9B2BF" Ref="D?"  Part="1" 
AR Path="/63F91570/63F9B2BF" Ref="D?"  Part="1" 
F 0 "D?" V 5554 3018 50  0000 L CNN
F 1 "SS14" V 5645 3018 50  0000 L CNN
F 2 "lc_lib:SMA(DO-214AC)_S1" V 5600 2950 50  0001 C CNN
F 3 "~" V 5600 2950 50  0001 C CNN
	1    5600 2950
	0    1    1    0   
$EndComp
Wire Wire Line
	5600 2850 5600 2550
Wire Wire Line
	5600 3050 5600 3350
Connection ~ 4750 3350
Connection ~ 5600 2350
$Comp
L Device:R_Small R?
U 1 1 63F9B2C9
P 6200 2600
AR Path="/66CBFED4/63F9B2C9" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B2C9" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B2C9" Ref="R?"  Part="1" 
F 0 "R?" H 6259 2646 50  0000 L CNN
F 1 "14K 1%" H 6259 2555 50  0000 L CNN
F 2 "lc_lib:0805_R" H 6200 2600 50  0001 C CNN
F 3 "~" H 6200 2600 50  0001 C CNN
	1    6200 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 63F9B2CF
P 6200 3050
AR Path="/66CBFED4/63F9B2CF" Ref="R?"  Part="1" 
AR Path="/63E76FBF/63F9B2CF" Ref="R?"  Part="1" 
AR Path="/63F91570/63F9B2CF" Ref="R?"  Part="1" 
F 0 "R?" H 6259 3096 50  0000 L CNN
F 1 "1K 1%" H 6259 3005 50  0000 L CNN
F 2 "lc_lib:0805_R" H 6200 3050 50  0001 C CNN
F 3 "~" H 6200 3050 50  0001 C CNN
	1    6200 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3150 6200 3350
Wire Wire Line
	4750 3350 5600 3350
Connection ~ 5600 3350
Wire Wire Line
	5600 3350 6200 3350
Wire Wire Line
	6200 2700 6200 2750
Wire Wire Line
	6200 2500 6200 2350
Wire Wire Line
	6200 2350 6050 2350
Wire Wire Line
	5250 2750 6200 2750
Connection ~ 6200 2750
Wire Wire Line
	6200 2750 6200 2950
Wire Wire Line
	6200 2350 6750 2350
Wire Wire Line
	6750 2350 6750 2700
Connection ~ 6200 2350
Wire Wire Line
	6750 2900 6750 3350
Wire Wire Line
	6750 3350 6200 3350
Connection ~ 6200 3350
$Comp
L Device:C_Small C?
U 1 1 63F9B2E5
P 6750 2800
AR Path="/66CBFED4/63F9B2E5" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2E5" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2E5" Ref="C?"  Part="1" 
F 0 "C?" H 6550 2800 50  0000 L CNN
F 1 "10uF 50V" H 6350 2700 50  0000 L CNN
F 2 "lc_lib:1210_C" H 6750 2800 50  0001 C CNN
F 3 "~" H 6750 2800 50  0001 C CNN
	1    6750 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 63F9B2EB
P 2900 2700
AR Path="/66CBFED4/63F9B2EB" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2EB" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2EB" Ref="C?"  Part="1" 
F 0 "C?" H 2700 2700 50  0000 L CNN
F 1 "10uF 50V" H 2500 2600 50  0000 L CNN
F 2 "lc_lib:1210_C" H 2900 2700 50  0001 C CNN
F 3 "~" H 2900 2700 50  0001 C CNN
	1    2900 2700
	1    0    0    -1  
$EndComp
Text HLabel 2700 2350 0    50   Input ~ 0
Buck_in
Text HLabel 6850 2350 2    50   Output ~ 0
Buck_out
Wire Wire Line
	6750 2350 6850 2350
Connection ~ 6750 2350
Wire Wire Line
	2700 2350 2900 2350
Connection ~ 2900 2350
Wire Wire Line
	3100 3350 2900 3350
Wire Wire Line
	3550 3350 3100 3350
Connection ~ 3100 3350
Wire Wire Line
	3100 2800 3100 3350
Wire Wire Line
	3100 2350 3550 2350
Wire Wire Line
	2900 2350 3100 2350
Connection ~ 3100 2350
Wire Wire Line
	3100 2600 3100 2350
$Comp
L Device:C_Small C?
U 1 1 63F9B2FF
P 3100 2700
AR Path="/66CBFED4/63F9B2FF" Ref="C?"  Part="1" 
AR Path="/63E76FBF/63F9B2FF" Ref="C?"  Part="1" 
AR Path="/63F91570/63F9B2FF" Ref="C?"  Part="1" 
F 0 "C?" H 2900 2700 50  0000 L CNN
F 1 "10uF 50V" H 2700 2600 50  0000 L CNN
F 2 "lc_lib:1210_C" H 3100 2700 50  0001 C CNN
F 3 "~" H 3100 2700 50  0001 C CNN
	1    3100 2700
	-1   0    0    -1  
$EndComp
Text HLabel 2900 3550 3    50   UnSpc ~ 0
Buck_GND
Wire Wire Line
	2900 3350 2900 3550
$Comp
L MySchLib:TPS54160 U?
U 1 1 63F9C888
P 4600 2250
F 0 "U?" H 4775 2415 50  0000 C CNN
F 1 "TPS54160" H 4775 2324 50  0000 C CNN
F 2 "" H 4600 2250 50  0001 C CNN
F 3 "" H 4600 2250 50  0001 C CNN
	1    4600 2250
	1    0    0    -1  
$EndComp
Connection ~ 5300 2550
Wire Wire Line
	5300 2550 5250 2550
Connection ~ 4750 3150
Wire Wire Line
	4750 3150 4750 3350
Connection ~ 5300 2350
Wire Wire Line
	3550 3350 3800 3350
Wire Wire Line
	3550 2650 3550 2950
Wire Wire Line
	3950 2600 4250 2600
Wire Wire Line
	3800 3150 3800 3350
Connection ~ 3800 3350
Wire Wire Line
	3950 3150 3950 3350
Wire Wire Line
	3800 3350 3950 3350
Connection ~ 3950 3350
Wire Wire Line
	3950 3350 4150 3350
Connection ~ 4150 3350
Wire Wire Line
	4150 3350 4650 3350
Wire Wire Line
	4150 2950 4150 2700
Wire Wire Line
	4150 2700 4250 2700
$EndSCHEMATC
