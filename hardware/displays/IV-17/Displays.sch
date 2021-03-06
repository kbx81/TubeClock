EESchema Schematic File Version 4
LIBS:IV-17-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 2800 1950 2    50   Input ~ 0
T1_A_PR
Text GLabel 2800 1750 2    50   Input ~ 0
T1_A_M
Text GLabel 2800 1650 2    50   Input ~ 0
T1_A_L
Text GLabel 2800 1550 2    50   Input ~ 0
T1_A_K
Text GLabel 2800 1450 2    50   Input ~ 0
T1_A_J
Text GLabel 2800 1350 2    50   Input ~ 0
T1_A_H
Text GLabel 2800 1250 2    50   Input ~ 0
T1_A_G2
Text GLabel 2800 1150 2    50   Input ~ 0
T1_A_G1
Text GLabel 1400 1150 0    50   Input ~ 0
T1_A_A1
Text GLabel 1400 1250 0    50   Input ~ 0
T1_A_A2
Text GLabel 1400 1350 0    50   Input ~ 0
T1_A_B
Text GLabel 1400 1450 0    50   Input ~ 0
T1_A_C
Text GLabel 1400 1550 0    50   Input ~ 0
T1_A_D2
Text GLabel 1400 1650 0    50   Input ~ 0
T1_A_D1
Text GLabel 1400 1750 0    50   Input ~ 0
T1_A_E
Text GLabel 1400 1850 0    50   Input ~ 0
T1_A_F
Text GLabel 1400 1950 0    50   Input ~ 0
T1_A_PL
Wire Wire Line
	900  2050 900  2150
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CC3697D
P 900 2050
AR Path="/5CC3697D" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CC3697D" Ref="#PWR041"  Part="1" 
F 0 "#PWR041" H 900 1900 50  0001 C CNN
F 1 "+V_HV" H 915 2223 50  0000 C CNN
F 2 "" H 900 2050 50  0001 C CNN
F 3 "" H 900 2050 50  0001 C CNN
	1    900  2050
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T2
U 1 1 5CC3A925
P 4600 1550
F 0 "T2" H 4600 2314 50  0000 C CNN
F 1 "IV-17" H 4600 2223 50  0000 C CNN
F 2 "VFD:IV-17" H 4625 -350 50  0001 C CNN
F 3 "" H 4625 -350 50  0000 C CNN
	1    4600 1550
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T3
U 1 1 5CC3CACE
P 7100 1550
F 0 "T3" H 7100 2314 50  0000 C CNN
F 1 "IV-17" H 7100 2223 50  0000 C CNN
F 2 "VFD:IV-17" H 7125 -350 50  0001 C CNN
F 3 "" H 7125 -350 50  0000 C CNN
	1    7100 1550
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T4
U 1 1 5CC3E165
P 9600 1550
F 0 "T4" H 9600 2314 50  0000 C CNN
F 1 "IV-17" H 9600 2223 50  0000 C CNN
F 2 "VFD:IV-17" H 9625 -350 50  0001 C CNN
F 3 "" H 9625 -350 50  0000 C CNN
	1    9600 1550
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T6
U 1 1 5CC43AEA
P 4600 4050
F 0 "T6" H 4600 4814 50  0000 C CNN
F 1 "IV-17" H 4600 4723 50  0000 C CNN
F 2 "VFD:IV-17" H 4625 2150 50  0001 C CNN
F 3 "" H 4625 2150 50  0000 C CNN
	1    4600 4050
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T7
U 1 1 5CC44E19
P 7100 4050
F 0 "T7" H 7100 4814 50  0000 C CNN
F 1 "IV-17" H 7100 4723 50  0000 C CNN
F 2 "VFD:IV-17" H 7125 2150 50  0001 C CNN
F 3 "" H 7125 2150 50  0000 C CNN
	1    7100 4050
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T8
U 1 1 5CC4606E
P 9600 4050
F 0 "T8" H 9600 4814 50  0000 C CNN
F 1 "IV-17" H 9600 4723 50  0000 C CNN
F 2 "VFD:IV-17" H 9625 2150 50  0001 C CNN
F 3 "" H 9625 2150 50  0000 C CNN
	1    9600 4050
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T9
U 1 1 5CC4962D
P 2100 6550
F 0 "T9" H 2100 7314 50  0000 C CNN
F 1 "IV-17" H 2100 7223 50  0000 C CNN
F 2 "VFD:IV-17" H 2125 4650 50  0001 C CNN
F 3 "" H 2125 4650 50  0000 C CNN
	1    2100 6550
	1    0    0    -1  
$EndComp
$Comp
L VFDs:IV-17 T10
U 1 1 5CC4AD31
P 4600 6550
F 0 "T10" H 4600 7314 50  0000 C CNN
F 1 "IV-17" H 4600 7223 50  0000 C CNN
F 2 "VFD:IV-17" H 4625 4650 50  0001 C CNN
F 3 "" H 4625 4650 50  0000 C CNN
	1    4600 6550
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  2150 1400 2150
Text GLabel 2800 1850 2    50   Input ~ 0
T1_A_N
Text GLabel 3900 1150 0    50   Input ~ 0
T2_A_A1
Text GLabel 3900 1250 0    50   Input ~ 0
T2_A_A2
Text GLabel 3900 1350 0    50   Input ~ 0
T2_A_B
Text GLabel 3900 1450 0    50   Input ~ 0
T2_A_C
Text GLabel 3900 1550 0    50   Input ~ 0
T2_A_D2
Text GLabel 3900 1650 0    50   Input ~ 0
T2_A_D1
Text GLabel 3900 1750 0    50   Input ~ 0
T2_A_E
Text GLabel 3900 1850 0    50   Input ~ 0
T2_A_F
Text GLabel 3900 1950 0    50   Input ~ 0
T2_A_PL
Wire Wire Line
	3400 2050 3400 2150
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCA6047
P 3400 2050
AR Path="/5CCA6047" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCA6047" Ref="#PWR042"  Part="1" 
F 0 "#PWR042" H 3400 1900 50  0001 C CNN
F 1 "+V_HV" H 3415 2223 50  0000 C CNN
F 2 "" H 3400 2050 50  0001 C CNN
F 3 "" H 3400 2050 50  0001 C CNN
	1    3400 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 2150 3900 2150
Text GLabel 5300 1950 2    50   Input ~ 0
T2_A_PR
Text GLabel 5300 1750 2    50   Input ~ 0
T2_A_M
Text GLabel 5300 1650 2    50   Input ~ 0
T2_A_L
Text GLabel 5300 1550 2    50   Input ~ 0
T2_A_K
Text GLabel 5300 1450 2    50   Input ~ 0
T2_A_J
Text GLabel 5300 1350 2    50   Input ~ 0
T2_A_H
Text GLabel 5300 1250 2    50   Input ~ 0
T2_A_G2
Text GLabel 5300 1150 2    50   Input ~ 0
T2_A_G1
Text GLabel 5300 1850 2    50   Input ~ 0
T2_A_N
Text GLabel 6400 1150 0    50   Input ~ 0
T3_A_A1
Text GLabel 6400 1250 0    50   Input ~ 0
T3_A_A2
Text GLabel 6400 1350 0    50   Input ~ 0
T3_A_B
Text GLabel 6400 1450 0    50   Input ~ 0
T3_A_C
Text GLabel 6400 1550 0    50   Input ~ 0
T3_A_D2
Text GLabel 6400 1650 0    50   Input ~ 0
T3_A_D1
Text GLabel 6400 1750 0    50   Input ~ 0
T3_A_E
Text GLabel 6400 1850 0    50   Input ~ 0
T3_A_F
Text GLabel 6400 1950 0    50   Input ~ 0
T3_A_PL
Wire Wire Line
	5900 2050 5900 2150
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCAA5FE
P 5900 2050
AR Path="/5CCAA5FE" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCAA5FE" Ref="#PWR043"  Part="1" 
F 0 "#PWR043" H 5900 1900 50  0001 C CNN
F 1 "+V_HV" H 5915 2223 50  0000 C CNN
F 2 "" H 5900 2050 50  0001 C CNN
F 3 "" H 5900 2050 50  0001 C CNN
	1    5900 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 2150 6400 2150
Text GLabel 7800 1950 2    50   Input ~ 0
T3_A_PR
Text GLabel 7800 1750 2    50   Input ~ 0
T3_A_M
Text GLabel 7800 1650 2    50   Input ~ 0
T3_A_L
Text GLabel 7800 1550 2    50   Input ~ 0
T3_A_K
Text GLabel 7800 1450 2    50   Input ~ 0
T3_A_J
Text GLabel 7800 1350 2    50   Input ~ 0
T3_A_H
Text GLabel 7800 1250 2    50   Input ~ 0
T3_A_G2
Text GLabel 7800 1150 2    50   Input ~ 0
T3_A_G1
Text GLabel 7800 1850 2    50   Input ~ 0
T3_A_N
Text GLabel 8900 1150 0    50   Input ~ 0
T4_A_A1
Text GLabel 8900 1250 0    50   Input ~ 0
T4_A_A2
Text GLabel 8900 1450 0    50   Input ~ 0
T4_A_C
Text GLabel 8900 1550 0    50   Input ~ 0
T4_A_D2
Text GLabel 8900 1750 0    50   Input ~ 0
T4_A_E
Text GLabel 8900 1850 0    50   Input ~ 0
T4_A_F
Text GLabel 8900 1950 0    50   Input ~ 0
T4_A_PL
Wire Wire Line
	8400 2050 8400 2150
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCB0E00
P 8400 2050
AR Path="/5CCB0E00" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCB0E00" Ref="#PWR044"  Part="1" 
F 0 "#PWR044" H 8400 1900 50  0001 C CNN
F 1 "+V_HV" H 8415 2223 50  0000 C CNN
F 2 "" H 8400 2050 50  0001 C CNN
F 3 "" H 8400 2050 50  0001 C CNN
	1    8400 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 2150 8900 2150
Text GLabel 10300 1950 2    50   Input ~ 0
T4_A_PR
Text GLabel 10300 1750 2    50   Input ~ 0
T4_A_M
Text GLabel 10300 1650 2    50   Input ~ 0
T4_A_L
Text GLabel 10300 1550 2    50   Input ~ 0
T4_A_K
Text GLabel 10300 1450 2    50   Input ~ 0
T4_A_J
Text GLabel 10300 1350 2    50   Input ~ 0
T4_A_H
Text GLabel 10300 1250 2    50   Input ~ 0
T4_A_G2
Text GLabel 10300 1150 2    50   Input ~ 0
T4_A_G1
Text GLabel 10300 1850 2    50   Input ~ 0
T4_A_N
Text GLabel 1400 3650 0    50   Input ~ 0
T5_A_A1
Text GLabel 1400 3750 0    50   Input ~ 0
T5_A_A2
Text GLabel 1400 3850 0    50   Input ~ 0
T5_A_B
Text GLabel 1400 3950 0    50   Input ~ 0
T5_A_C
Text GLabel 1400 4050 0    50   Input ~ 0
T5_A_D2
Text GLabel 1400 4150 0    50   Input ~ 0
T5_A_D1
Text GLabel 1400 4250 0    50   Input ~ 0
T5_A_E
Text GLabel 1400 4350 0    50   Input ~ 0
T5_A_F
Text GLabel 1400 4450 0    50   Input ~ 0
T5_A_PL
Wire Wire Line
	900  4550 900  4650
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCD7A79
P 900 4550
AR Path="/5CCD7A79" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCD7A79" Ref="#PWR045"  Part="1" 
F 0 "#PWR045" H 900 4400 50  0001 C CNN
F 1 "+V_HV" H 915 4723 50  0000 C CNN
F 2 "" H 900 4550 50  0001 C CNN
F 3 "" H 900 4550 50  0001 C CNN
	1    900  4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  4650 1400 4650
Text GLabel 3900 3650 0    50   Input ~ 0
T6_A_A1
Text GLabel 3900 3750 0    50   Input ~ 0
T6_A_A2
Text GLabel 3900 3850 0    50   Input ~ 0
T6_A_B
Text GLabel 3900 3950 0    50   Input ~ 0
T6_A_C
Text GLabel 3900 4050 0    50   Input ~ 0
T6_A_D2
Text GLabel 3900 4150 0    50   Input ~ 0
T6_A_D1
Text GLabel 3900 4250 0    50   Input ~ 0
T6_A_E
Text GLabel 3900 4350 0    50   Input ~ 0
T6_A_F
Text GLabel 3900 4450 0    50   Input ~ 0
T6_A_PL
Wire Wire Line
	3400 4550 3400 4650
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCD8DA0
P 3400 4550
AR Path="/5CCD8DA0" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCD8DA0" Ref="#PWR046"  Part="1" 
F 0 "#PWR046" H 3400 4400 50  0001 C CNN
F 1 "+V_HV" H 3415 4723 50  0000 C CNN
F 2 "" H 3400 4550 50  0001 C CNN
F 3 "" H 3400 4550 50  0001 C CNN
	1    3400 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 4650 3900 4650
Text GLabel 6400 3650 0    50   Input ~ 0
T7_A_A1
Text GLabel 6400 3750 0    50   Input ~ 0
T7_A_A2
Text GLabel 6400 3850 0    50   Input ~ 0
T7_A_B
Text GLabel 6400 3950 0    50   Input ~ 0
T7_A_C
Text GLabel 6400 4050 0    50   Input ~ 0
T7_A_D2
Text GLabel 6400 4150 0    50   Input ~ 0
T7_A_D1
Text GLabel 6400 4250 0    50   Input ~ 0
T7_A_E
Text GLabel 6400 4350 0    50   Input ~ 0
T7_A_F
Text GLabel 6400 4450 0    50   Input ~ 0
T7_A_PL
Wire Wire Line
	5900 4550 5900 4650
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCD9F3F
P 5900 4550
AR Path="/5CCD9F3F" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCD9F3F" Ref="#PWR047"  Part="1" 
F 0 "#PWR047" H 5900 4400 50  0001 C CNN
F 1 "+V_HV" H 5915 4723 50  0000 C CNN
F 2 "" H 5900 4550 50  0001 C CNN
F 3 "" H 5900 4550 50  0001 C CNN
	1    5900 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 4650 6400 4650
Text GLabel 8900 3650 0    50   Input ~ 0
T8_A_A1
Text GLabel 8900 3750 0    50   Input ~ 0
T8_A_A2
Text GLabel 8900 3850 0    50   Input ~ 0
T8_A_B
Text GLabel 8900 3950 0    50   Input ~ 0
T8_A_C
Text GLabel 8900 4050 0    50   Input ~ 0
T8_A_D2
Text GLabel 8900 4150 0    50   Input ~ 0
T8_A_D1
Text GLabel 8900 4250 0    50   Input ~ 0
T8_A_E
Text GLabel 8900 4350 0    50   Input ~ 0
T8_A_F
Text GLabel 8900 4450 0    50   Input ~ 0
T8_A_PL
Wire Wire Line
	8400 4550 8400 4650
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCDB046
P 8400 4550
AR Path="/5CCDB046" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCDB046" Ref="#PWR048"  Part="1" 
F 0 "#PWR048" H 8400 4400 50  0001 C CNN
F 1 "+V_HV" H 8415 4723 50  0000 C CNN
F 2 "" H 8400 4550 50  0001 C CNN
F 3 "" H 8400 4550 50  0001 C CNN
	1    8400 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 4650 8900 4650
Text GLabel 2800 4450 2    50   Input ~ 0
T5_A_PR
Text GLabel 2800 4250 2    50   Input ~ 0
T5_A_M
Text GLabel 2800 4150 2    50   Input ~ 0
T5_A_L
Text GLabel 2800 4050 2    50   Input ~ 0
T5_A_K
Text GLabel 2800 3950 2    50   Input ~ 0
T5_A_J
Text GLabel 2800 3850 2    50   Input ~ 0
T5_A_H
Text GLabel 2800 3750 2    50   Input ~ 0
T5_A_G2
Text GLabel 2800 3650 2    50   Input ~ 0
T5_A_G1
Text GLabel 2800 4350 2    50   Input ~ 0
T5_A_N
Text GLabel 5300 4450 2    50   Input ~ 0
T6_A_PR
Text GLabel 5300 4250 2    50   Input ~ 0
T6_A_M
Text GLabel 5300 4150 2    50   Input ~ 0
T6_A_L
Text GLabel 5300 4050 2    50   Input ~ 0
T6_A_K
Text GLabel 5300 3950 2    50   Input ~ 0
T6_A_J
Text GLabel 5300 3850 2    50   Input ~ 0
T6_A_H
Text GLabel 5300 3750 2    50   Input ~ 0
T6_A_G2
Text GLabel 5300 3650 2    50   Input ~ 0
T6_A_G1
Text GLabel 5300 4350 2    50   Input ~ 0
T6_A_N
Text GLabel 7800 4450 2    50   Input ~ 0
T7_A_PR
Text GLabel 7800 4250 2    50   Input ~ 0
T7_A_M
Text GLabel 7800 4150 2    50   Input ~ 0
T7_A_L
Text GLabel 7800 4050 2    50   Input ~ 0
T7_A_K
Text GLabel 7800 3950 2    50   Input ~ 0
T7_A_J
Text GLabel 7800 3850 2    50   Input ~ 0
T7_A_H
Text GLabel 7800 3750 2    50   Input ~ 0
T7_A_G2
Text GLabel 7800 3650 2    50   Input ~ 0
T7_A_G1
Text GLabel 7800 4350 2    50   Input ~ 0
T7_A_N
Text GLabel 10300 4450 2    50   Input ~ 0
T8_A_PR
Text GLabel 10300 4250 2    50   Input ~ 0
T8_A_M
Text GLabel 10300 4150 2    50   Input ~ 0
T8_A_L
Text GLabel 10300 4050 2    50   Input ~ 0
T8_A_K
Text GLabel 10300 3950 2    50   Input ~ 0
T8_A_J
Text GLabel 10300 3850 2    50   Input ~ 0
T8_A_H
Text GLabel 10300 3750 2    50   Input ~ 0
T8_A_G2
Text GLabel 10300 3650 2    50   Input ~ 0
T8_A_G1
Text GLabel 10300 4350 2    50   Input ~ 0
T8_A_N
Text GLabel 1400 6150 0    50   Input ~ 0
T9_A_A1
Text GLabel 1400 6250 0    50   Input ~ 0
T9_A_A2
Text GLabel 1400 6350 0    50   Input ~ 0
T9_A_B
Text GLabel 1400 6450 0    50   Input ~ 0
T9_A_C
Text GLabel 1400 6550 0    50   Input ~ 0
T9_A_D2
Text GLabel 1400 6650 0    50   Input ~ 0
T9_A_D1
Text GLabel 1400 6750 0    50   Input ~ 0
T9_A_E
Text GLabel 1400 6850 0    50   Input ~ 0
T9_A_F
Text GLabel 1400 6950 0    50   Input ~ 0
T9_A_PL
Wire Wire Line
	900  7050 900  7150
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCECE8A
P 900 7050
AR Path="/5CCECE8A" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCECE8A" Ref="#PWR049"  Part="1" 
F 0 "#PWR049" H 900 6900 50  0001 C CNN
F 1 "+V_HV" H 915 7223 50  0000 C CNN
F 2 "" H 900 7050 50  0001 C CNN
F 3 "" H 900 7050 50  0001 C CNN
	1    900  7050
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  7150 1400 7150
Text GLabel 3900 6150 0    50   Input ~ 0
T10_A_A1
Text GLabel 3900 6250 0    50   Input ~ 0
T10_A_A2
Text GLabel 3900 6350 0    50   Input ~ 0
T10_A_B
Text GLabel 3900 6450 0    50   Input ~ 0
T10_A_C
Text GLabel 3900 6550 0    50   Input ~ 0
T10_A_D2
Text GLabel 3900 6650 0    50   Input ~ 0
T10_A_D1
Text GLabel 3900 6750 0    50   Input ~ 0
T10_A_E
Text GLabel 3900 6850 0    50   Input ~ 0
T10_A_F
Text GLabel 3900 6950 0    50   Input ~ 0
T10_A_PL
Wire Wire Line
	3400 7050 3400 7150
$Comp
L TubeClock:+V_HV #PWR?
U 1 1 5CCEDE55
P 3400 7050
AR Path="/5CCEDE55" Ref="#PWR?"  Part="1" 
AR Path="/5CC20074/5CCEDE55" Ref="#PWR050"  Part="1" 
F 0 "#PWR050" H 3400 6900 50  0001 C CNN
F 1 "+V_HV" H 3415 7223 50  0000 C CNN
F 2 "" H 3400 7050 50  0001 C CNN
F 3 "" H 3400 7050 50  0001 C CNN
	1    3400 7050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 7150 3900 7150
Text GLabel 2800 6950 2    50   Input ~ 0
T9_A_PR
Text GLabel 2800 6750 2    50   Input ~ 0
T9_A_M
Text GLabel 2800 6650 2    50   Input ~ 0
T9_A_L
Text GLabel 2800 6550 2    50   Input ~ 0
T9_A_K
Text GLabel 2800 6450 2    50   Input ~ 0
T9_A_J
Text GLabel 2800 6350 2    50   Input ~ 0
T9_A_H
Text GLabel 2800 6250 2    50   Input ~ 0
T9_A_G2
Text GLabel 2800 6150 2    50   Input ~ 0
T9_A_G1
Text GLabel 2800 6850 2    50   Input ~ 0
T9_A_N
Text GLabel 5300 6950 2    50   Input ~ 0
T10_A_PR
Text GLabel 5300 6750 2    50   Input ~ 0
T10_A_M
Text GLabel 5300 6650 2    50   Input ~ 0
T10_A_L
Text GLabel 5300 6550 2    50   Input ~ 0
T10_A_K
Text GLabel 5300 6450 2    50   Input ~ 0
T10_A_J
Text GLabel 5300 6350 2    50   Input ~ 0
T10_A_H
Text GLabel 5300 6250 2    50   Input ~ 0
T10_A_G2
Text GLabel 5300 6150 2    50   Input ~ 0
T10_A_G1
Text GLabel 5300 6850 2    50   Input ~ 0
T10_A_N
Wire Wire Line
	2000 2550 1500 2550
Wire Wire Line
	2200 2550 4500 2550
Wire Wire Line
	7200 2550 9500 2550
Wire Wire Line
	9700 2550 10000 2550
Wire Wire Line
	7200 5050 9500 5050
Wire Wire Line
	9700 5050 10000 5050
Wire Wire Line
	2200 7550 4500 7550
Text GLabel 8900 1350 0    50   Input ~ 0
T4_A_B
Text GLabel 8900 1650 0    50   Input ~ 0
T4_A_D1
$Comp
L VFDs:IV-17 T1
U 1 1 5CC36BC3
P 2100 1550
F 0 "T1" H 2100 2314 50  0000 C CNN
F 1 "IV-17" H 2100 2223 50  0000 C CNN
F 2 "VFD:IV-17" H 2125 -350 50  0001 C CNN
F 3 "" H 2125 -350 50  0000 C CNN
	1    2100 1550
	1    0    0    -1  
$EndComp
Text GLabel 1500 2550 0    50   UnSpc ~ 0
FILAMENT_AC1
Text GLabel 1500 5050 0    50   UnSpc ~ 0
FILAMENT_AC1
Text GLabel 1500 2700 0    50   UnSpc ~ 0
FILAMENT_AC2
Text GLabel 5000 7550 2    50   UnSpc ~ 0
FILAMENT_AC2
Wire Wire Line
	5000 7550 4700 7550
$Comp
L VFDs:IV-17 T5
U 1 1 5CC41425
P 2100 4050
F 0 "T5" H 2100 4814 50  0000 C CNN
F 1 "IV-17" H 2100 4723 50  0000 C CNN
F 2 "VFD:IV-17" H 2125 2150 50  0001 C CNN
F 3 "" H 2125 2150 50  0000 C CNN
	1    2100 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	1500 2700 4700 2700
Wire Wire Line
	4700 2700 4700 2550
Text GLabel 10000 2550 2    50   UnSpc ~ 0
FILAMENT_AC1
Text GLabel 10000 2700 2    50   UnSpc ~ 0
FILAMENT_AC2
Wire Wire Line
	7000 2550 7000 2700
Wire Wire Line
	7000 2700 10000 2700
Text GLabel 10000 5050 2    50   UnSpc ~ 0
FILAMENT_AC1
Text GLabel 10000 5200 2    50   UnSpc ~ 0
FILAMENT_AC2
Wire Wire Line
	7000 5050 7000 5200
Wire Wire Line
	7000 5200 10000 5200
Wire Wire Line
	1500 5050 2000 5050
Wire Wire Line
	2200 5050 4500 5050
Text GLabel 1500 5200 0    50   UnSpc ~ 0
FILAMENT_AC2
Wire Wire Line
	1500 5200 4700 5200
Wire Wire Line
	4700 5200 4700 5050
Text GLabel 1500 7550 0    50   UnSpc ~ 0
FILAMENT_AC1
Wire Wire Line
	1500 7550 2000 7550
Text GLabel 10000 2850 2    50   UnSpc ~ 0
FILAMENT_CT
Wire Wire Line
	10000 2850 9500 2850
Wire Wire Line
	9500 2850 9500 2550
Connection ~ 9500 2550
$EndSCHEMATC
