EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 4
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
L MySchLib:TPS54331 U?
U 1 1 66CCB9E4
P 5800 4550
AR Path="/66CBFED4/66CCB9E4" Ref="U?"  Part="1" 
AR Path="/63E76FBF/66CCB9E4" Ref="U?"  Part="1" 
F 0 "U?" H 5800 5017 50  0000 C CNN
F 1 "TPS54331" H 5800 4926 50  0000 C CNN
F 2 "Package_SO:TI_SO-PowerPAD-8_ThermalVias" H 6700 4200 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/tps54336a.pdf" H 6800 4100 50  0001 C CNN
	1    5800 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 4600 3950 4350
$Comp
L Device:R_Small R?
U 1 1 66CD149C
P 4600 4550
AR Path="/66CBFED4/66CD149C" Ref="R?"  Part="1" 
AR Path="/63E76FBF/66CD149C" Ref="R?"  Part="1" 
F 0 "R?" H 4659 4596 50  0000 L CNN
F 1 "169K" H 4659 4505 50  0000 L CNN
F 2 "lc_lib:0805_R" H 4600 4550 50  0001 C CNN
F 3 "~" H 4600 4550 50  0001 C CNN
	1    4600 4550
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 66CD1812
P 4600 5050
AR Path="/66CBFED4/66CD1812" Ref="R?"  Part="1" 
AR Path="/63E76FBF/66CD1812" Ref="R?"  Part="1" 
F 0 "R?" H 4659 5096 50  0000 L CNN
F 1 "14K" H 4659 5005 50  0000 L CNN
F 2 "lc_lib:0805_R" H 4600 5050 50  0001 C CNN
F 3 "~" H 4600 5050 50  0001 C CNN
	1    4600 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 5150 4600 5350
Connection ~ 3950 5350
Wire Wire Line
	3950 5350 3950 4800
Wire Wire Line
	4600 4950 4600 4750
Wire Wire Line
	4600 4450 4600 4350
Connection ~ 4600 4350
Wire Wire Line
	4600 4350 5300 4350
Wire Wire Line
	4600 4750 4900 4750
Wire Wire Line
	4900 4750 4900 4450
Wire Wire Line
	4900 4450 5300 4450
Connection ~ 4600 4750
Wire Wire Line
	4600 4750 4600 4650
$Comp
L Device:C_Small C?
U 1 1 66CD23F7
P 5050 5200
AR Path="/66CBFED4/66CD23F7" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CD23F7" Ref="C?"  Part="1" 
F 0 "C?" H 5000 4950 50  0000 L CNN
F 1 "10pF" H 4950 4850 50  0000 L CNN
F 2 "lc_lib:0805_C" H 5050 5200 50  0001 C CNN
F 3 "~" H 5050 5200 50  0001 C CNN
	1    5050 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 66CD2C23
P 5200 5200
AR Path="/66CBFED4/66CD2C23" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CD2C23" Ref="C?"  Part="1" 
F 0 "C?" H 5292 5246 50  0000 L CNN
F 1 "102" H 5292 5155 50  0000 L CNN
F 2 "lc_lib:0805_C" H 5200 5200 50  0001 C CNN
F 3 "~" H 5200 5200 50  0001 C CNN
	1    5200 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 66CD3ED7
P 5200 4950
AR Path="/66CBFED4/66CD3ED7" Ref="R?"  Part="1" 
AR Path="/63E76FBF/66CD3ED7" Ref="R?"  Part="1" 
F 0 "R?" H 5259 4996 50  0000 L CNN
F 1 "33K" H 5259 4905 50  0000 L CNN
F 2 "lc_lib:0805_R" H 5200 4950 50  0001 C CNN
F 3 "~" H 5200 4950 50  0001 C CNN
	1    5200 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 5100 5200 5050
Wire Wire Line
	5200 4850 5200 4750
Wire Wire Line
	5200 4750 5300 4750
Wire Wire Line
	5050 5100 5050 4750
Wire Wire Line
	5050 4750 5200 4750
Connection ~ 5200 4750
Wire Wire Line
	4600 5350 4950 5350
Wire Wire Line
	5200 5350 5200 5300
Connection ~ 4600 5350
Wire Wire Line
	5050 5300 5050 5350
Connection ~ 5050 5350
Wire Wire Line
	5050 5350 5200 5350
$Comp
L Device:C_Small C?
U 1 1 66CD68AB
P 4950 4900
AR Path="/66CBFED4/66CD68AB" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CD68AB" Ref="C?"  Part="1" 
F 0 "C?" H 4750 4900 50  0000 L CNN
F 1 "8.2pF" H 4750 4800 50  0000 L CNN
F 2 "lc_lib:0805_C" H 4950 4900 50  0001 C CNN
F 3 "~" H 4950 4900 50  0001 C CNN
	1    4950 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 4800 4950 4650
Wire Wire Line
	4950 4650 5300 4650
Wire Wire Line
	4950 5000 4950 5350
Connection ~ 4950 5350
Wire Wire Line
	4950 5350 5050 5350
Wire Wire Line
	5700 4950 5700 5350
Wire Wire Line
	5700 5350 5200 5350
Connection ~ 5200 5350
Wire Wire Line
	5800 4950 5800 5350
Wire Wire Line
	5800 5350 5700 5350
Connection ~ 5700 5350
$Comp
L Device:C_Small C?
U 1 1 66CDB6B5
P 6450 4350
AR Path="/66CBFED4/66CDB6B5" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CDB6B5" Ref="C?"  Part="1" 
F 0 "C?" V 6221 4350 50  0000 C CNN
F 1 "104" V 6312 4350 50  0000 C CNN
F 2 "lc_lib:0805_C" H 6450 4350 50  0001 C CNN
F 3 "~" H 6450 4350 50  0001 C CNN
	1    6450 4350
	0    1    1    0   
$EndComp
$Comp
L Device:L_Core_Ferrite L?
U 1 1 66CDCA99
P 6950 4350
AR Path="/66CBFED4/66CDCA99" Ref="L?"  Part="1" 
AR Path="/63E76FBF/66CDCA99" Ref="L?"  Part="1" 
F 0 "L?" V 7175 4350 50  0000 C CNN
F 1 "47uH 1A" V 7084 4350 50  0000 C CNN
F 2 "MyPCBLib:L_SMD4030_hand" H 6950 4350 50  0001 C CNN
F 3 "~" H 6950 4350 50  0001 C CNN
	1    6950 4350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6800 4350 6650 4350
Wire Wire Line
	6300 4350 6350 4350
Wire Wire Line
	6550 4350 6650 4350
Wire Wire Line
	6650 4350 6650 4550
Connection ~ 6650 4550
Wire Wire Line
	6650 4550 6300 4550
$Comp
L MySchLib:D_Schottky_Small D?
U 1 1 66CE065A
P 6650 4950
AR Path="/66CBFED4/66CE065A" Ref="D?"  Part="1" 
AR Path="/63E76FBF/66CE065A" Ref="D?"  Part="1" 
F 0 "D?" V 6604 5018 50  0000 L CNN
F 1 "SS14" V 6695 5018 50  0000 L CNN
F 2 "lc_lib:SMA(DO-214AC)_S1" V 6650 4950 50  0001 C CNN
F 3 "~" V 6650 4950 50  0001 C CNN
	1    6650 4950
	0    1    1    0   
$EndComp
Wire Wire Line
	6650 4850 6650 4550
Wire Wire Line
	6650 5050 6650 5350
Connection ~ 5800 5350
Connection ~ 6650 4350
$Comp
L Device:R_Small R?
U 1 1 66CE2D3E
P 7250 4600
AR Path="/66CBFED4/66CE2D3E" Ref="R?"  Part="1" 
AR Path="/63E76FBF/66CE2D3E" Ref="R?"  Part="1" 
F 0 "R?" H 7309 4646 50  0000 L CNN
F 1 "14K 1%" H 7309 4555 50  0000 L CNN
F 2 "lc_lib:0805_R" H 7250 4600 50  0001 C CNN
F 3 "~" H 7250 4600 50  0001 C CNN
	1    7250 4600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 66CE3320
P 7250 5050
AR Path="/66CBFED4/66CE3320" Ref="R?"  Part="1" 
AR Path="/63E76FBF/66CE3320" Ref="R?"  Part="1" 
F 0 "R?" H 7309 5096 50  0000 L CNN
F 1 "1K 1%" H 7309 5005 50  0000 L CNN
F 2 "lc_lib:0805_R" H 7250 5050 50  0001 C CNN
F 3 "~" H 7250 5050 50  0001 C CNN
	1    7250 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7250 5150 7250 5350
Wire Wire Line
	5800 5350 6650 5350
Connection ~ 6650 5350
Wire Wire Line
	6650 5350 7250 5350
Wire Wire Line
	7250 4700 7250 4750
Wire Wire Line
	7250 4500 7250 4350
Wire Wire Line
	7250 4350 7100 4350
Wire Wire Line
	6300 4750 7250 4750
Connection ~ 7250 4750
Wire Wire Line
	7250 4750 7250 4950
Wire Wire Line
	7250 4350 7800 4350
Wire Wire Line
	7800 4350 7800 4700
Connection ~ 7250 4350
Wire Wire Line
	7800 4900 7800 5350
Wire Wire Line
	7800 5350 7250 5350
Connection ~ 7250 5350
$Comp
L Device:C_Small C?
U 1 1 66CE71FC
P 7800 4800
AR Path="/66CBFED4/66CE71FC" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CE71FC" Ref="C?"  Part="1" 
F 0 "C?" H 7600 4800 50  0000 L CNN
F 1 "10uF 50V" H 7400 4700 50  0000 L CNN
F 2 "lc_lib:1210_C" H 7800 4800 50  0001 C CNN
F 3 "~" H 7800 4800 50  0001 C CNN
	1    7800 4800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 66CED0DA
P 3950 4700
AR Path="/66CBFED4/66CED0DA" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CED0DA" Ref="C?"  Part="1" 
F 0 "C?" H 3750 4700 50  0000 L CNN
F 1 "10uF 50V" H 3550 4600 50  0000 L CNN
F 2 "lc_lib:1210_C" H 3950 4700 50  0001 C CNN
F 3 "~" H 3950 4700 50  0001 C CNN
	1    3950 4700
	1    0    0    -1  
$EndComp
Text HLabel 3750 4350 0    50   Input ~ 0
Buck_in
Text HLabel 7900 4350 2    50   Output ~ 0
Buck_out
Wire Wire Line
	7800 4350 7900 4350
Connection ~ 7800 4350
Wire Wire Line
	3750 4350 3950 4350
Connection ~ 3950 4350
Wire Wire Line
	4150 5350 3950 5350
Wire Wire Line
	4600 5350 4150 5350
Connection ~ 4150 5350
Wire Wire Line
	4150 4800 4150 5350
Wire Wire Line
	4150 4350 4600 4350
Wire Wire Line
	3950 4350 4150 4350
Connection ~ 4150 4350
Wire Wire Line
	4150 4600 4150 4350
$Comp
L Device:C_Small C?
U 1 1 66CED849
P 4150 4700
AR Path="/66CBFED4/66CED849" Ref="C?"  Part="1" 
AR Path="/63E76FBF/66CED849" Ref="C?"  Part="1" 
F 0 "C?" H 3950 4700 50  0000 L CNN
F 1 "10uF 50V" H 3750 4600 50  0000 L CNN
F 2 "lc_lib:1210_C" H 4150 4700 50  0001 C CNN
F 3 "~" H 4150 4700 50  0001 C CNN
	1    4150 4700
	-1   0    0    -1  
$EndComp
Text HLabel 3950 5550 3    50   UnSpc ~ 0
Buck_GND
Wire Wire Line
	3950 5350 3950 5550
$EndSCHEMATC
