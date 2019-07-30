EESchema Schematic File Version 4
LIBS:IN-18-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 1350 5800 0    50   Input ~ 0
~LE~
Text GLabel 1350 5500 0    50   Input ~ 0
CLK
Text GLabel 1350 5600 0    50   Input ~ 0
DATA_IN
Text GLabel 1350 5700 0    50   Output ~ 0
DATA_THROUGH
Text GLabel 1350 5950 0    50   Input ~ 0
~BL~
Wire Wire Line
	1350 5500 1450 5500
Wire Wire Line
	1450 5600 1350 5600
Wire Wire Line
	1350 5700 1450 5700
Wire Wire Line
	1450 5800 1350 5800
Wire Wire Line
	1350 5950 1450 5950
Text GLabel 2050 4900 1    50   Output ~ 0
T1_K_9
Text GLabel 2150 4900 1    50   Output ~ 0
T1_K_8
Text GLabel 2250 4900 1    50   Output ~ 0
T1_K_7
Text GLabel 2350 4900 1    50   Output ~ 0
T1_K_6
Text GLabel 2450 4900 1    50   Output ~ 0
T1_K_5
Text GLabel 2550 4900 1    50   Output ~ 0
T1_K_4
Text GLabel 2650 4900 1    50   Output ~ 0
T1_K_3
Text GLabel 2750 4900 1    50   Output ~ 0
T1_K_2
Text GLabel 2850 4900 1    50   Output ~ 0
T1_K_1
Text GLabel 2950 4900 1    50   Output ~ 0
T1_K_0
Text GLabel 3600 5500 2    50   Output ~ 0
T2_K_9
Text GLabel 3600 5600 2    50   Output ~ 0
T2_K_8
Text GLabel 3600 5700 2    50   Output ~ 0
T2_K_7
Text GLabel 3600 5800 2    50   Output ~ 0
T2_K_6
Text GLabel 3600 5900 2    50   Output ~ 0
T2_K_5
Text GLabel 3600 6000 2    50   Output ~ 0
T2_K_4
Text GLabel 3600 6100 2    50   Output ~ 0
T2_K_3
Text GLabel 3600 6200 2    50   Output ~ 0
T2_K_2
Text GLabel 3600 6300 2    50   Output ~ 0
T2_K_1
Text GLabel 3600 6400 2    50   Output ~ 0
T2_K_0
Text GLabel 3000 7050 3    50   Output ~ 0
T3_K_9
Text GLabel 2900 7050 3    50   Output ~ 0
T3_K_8
Text GLabel 2800 7050 3    50   Output ~ 0
T3_K_7
Text GLabel 2700 7050 3    50   Output ~ 0
T3_K_6
Text GLabel 2600 7050 3    50   Output ~ 0
T3_K_5
Text GLabel 2500 7050 3    50   Output ~ 0
T3_K_4
Text GLabel 2400 7050 3    50   Output ~ 0
T3_K_3
Text GLabel 2300 7050 3    50   Output ~ 0
T3_K_2
$Comp
L TubeClock:+V_IN #PWR023
U 1 1 5CA122CA
P 1350 5200
F 0 "#PWR023" H 1350 5050 50  0001 C CNN
F 1 "+V_IN" H 1365 5373 50  0000 C CNN
F 2 "" H 1350 5200 50  0001 C CNN
F 3 "" H 1350 5200 50  0001 C CNN
	1    1350 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 5200 1350 5300
Wire Wire Line
	1350 5300 1450 5300
Wire Wire Line
	650  6100 650  6050
Wire Wire Line
	650  5300 1350 5300
Connection ~ 1350 5300
Wire Wire Line
	1450 6050 650  6050
Connection ~ 650  6050
Wire Wire Line
	650  6050 650  5300
Wire Wire Line
	2050 4900 2050 5000
Wire Wire Line
	2300 7050 2300 6950
Wire Wire Line
	2400 6950 2400 7050
Wire Wire Line
	2500 7050 2500 6950
Wire Wire Line
	2600 6950 2600 7050
Wire Wire Line
	2700 7050 2700 6950
Wire Wire Line
	2800 6950 2800 7050
Wire Wire Line
	2900 7050 2900 6950
Wire Wire Line
	3000 6950 3000 7050
Wire Wire Line
	2150 4900 2150 5000
Wire Wire Line
	2250 5000 2250 4900
Wire Wire Line
	2350 4900 2350 5000
Wire Wire Line
	2450 5000 2450 4900
Wire Wire Line
	2550 4900 2550 5000
Wire Wire Line
	2650 5000 2650 4900
Wire Wire Line
	2750 4900 2750 5000
Wire Wire Line
	2850 5000 2850 4900
Wire Wire Line
	2950 4900 2950 5000
Wire Wire Line
	3500 5500 3600 5500
Wire Wire Line
	3600 5600 3500 5600
Wire Wire Line
	3500 5700 3600 5700
Wire Wire Line
	3600 5800 3500 5800
Wire Wire Line
	3500 5900 3600 5900
Wire Wire Line
	3600 6000 3500 6000
Wire Wire Line
	3500 6100 3600 6100
Wire Wire Line
	3600 6200 3500 6200
Wire Wire Line
	3500 6300 3600 6300
Wire Wire Line
	3600 6400 3500 6400
Wire Wire Line
	9200 1950 9200 2400
Wire Wire Line
	11000 1950 9200 1950
Wire Wire Line
	11000 2400 11000 1950
Wire Wire Line
	9200 650  9200 1100
Wire Wire Line
	11000 650  9200 650 
Wire Wire Line
	11000 1100 11000 650 
Wire Wire Line
	8300 2400 8300 1950
Wire Wire Line
	6500 650  6500 1100
Wire Wire Line
	8300 650  6500 650 
Wire Wire Line
	8300 1100 8300 650 
Wire Wire Line
	3800 1950 3800 2400
Wire Wire Line
	5600 1950 3800 1950
Wire Wire Line
	5600 2400 5600 1950
Wire Wire Line
	3800 650  3800 1100
Wire Wire Line
	5600 650  3800 650 
Wire Wire Line
	5600 1100 5600 650 
Text GLabel 8950 3350 3    50   Input ~ 0
T10_K
Connection ~ 8700 2400
Wire Wire Line
	8700 2500 8700 2400
$Comp
L Device:R R10
U 1 1 5C99F4CE
P 8700 2650
F 0 "R10" H 8630 2604 50  0000 R CNN
F 1 "470K" H 8630 2695 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 8630 2650 50  0001 C CNN
F 3 "~" H 8700 2650 50  0001 C CNN
	1    8700 2650
	-1   0    0    1   
$EndComp
$Comp
L Device:Lamp_Neon NE4
U 1 1 5C99F4C8
P 8950 3100
F 0 "NE4" V 8685 3100 50  0000 C CNN
F 1 "Lamp_Neon" V 8776 3100 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" V 8950 3200 50  0001 C CNN
F 3 "~" V 8950 3200 50  0001 C CNN
	1    8950 3100
	-1   0    0    1   
$EndComp
Wire Wire Line
	9100 2400 9200 2400
Wire Wire Line
	8700 2400 8800 2400
Wire Wire Line
	8700 2300 8700 2400
$Comp
L TubeClock:+V_HV #PWR010
U 1 1 5C99F4BF
P 8700 2300
F 0 "#PWR010" H 8700 2150 50  0001 C CNN
F 1 "+V_HV" H 8715 2473 50  0000 C CNN
F 2 "" H 8700 2300 50  0001 C CNN
F 3 "" H 8700 2300 50  0001 C CNN
	1    8700 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5C99F4B9
P 8950 2400
F 0 "R6" V 8743 2400 50  0000 C CNN
F 1 "2K2" V 8834 2400 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 8880 2400 50  0001 C CNN
F 3 "~" H 8950 2400 50  0001 C CNN
	1    8950 2400
	0    1    1    0   
$EndComp
$Comp
L Device:R R9
U 1 1 5C99BD46
P 8500 2650
F 0 "R9" H 8650 2600 50  0000 R CNN
F 1 "470K" H 8750 2700 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 8430 2650 50  0001 C CNN
F 3 "~" H 8500 2650 50  0001 C CNN
	1    8500 2650
	-1   0    0    1   
$EndComp
$Comp
L Device:Lamp_Neon NE3
U 1 1 5C99BD40
P 8500 3100
F 0 "NE3" V 8235 3100 50  0000 C CNN
F 1 "Lamp_Neon" V 8326 3100 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" V 8500 3200 50  0001 C CNN
F 3 "~" V 8500 3200 50  0001 C CNN
	1    8500 3100
	-1   0    0    1   
$EndComp
Wire Wire Line
	9100 1100 9200 1100
Wire Wire Line
	8700 1100 8800 1100
Wire Wire Line
	8700 1000 8700 1100
$Comp
L TubeClock:+V_HV #PWR05
U 1 1 5C99BD37
P 8700 1000
F 0 "#PWR05" H 8700 850 50  0001 C CNN
F 1 "+V_HV" H 8715 1173 50  0000 C CNN
F 2 "" H 8700 1000 50  0001 C CNN
F 3 "" H 8700 1000 50  0001 C CNN
	1    8700 1000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5C99BD31
P 8950 1100
F 0 "R3" V 8743 1100 50  0000 C CNN
F 1 "2K2" V 8834 1100 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 8880 1100 50  0001 C CNN
F 3 "~" H 8950 1100 50  0001 C CNN
	1    8950 1100
	0    1    1    0   
$EndComp
Connection ~ 6000 2400
Wire Wire Line
	6000 2500 6000 2400
$Comp
L Device:R R8
U 1 1 5C995BA6
P 6000 2650
F 0 "R8" H 5930 2604 50  0000 R CNN
F 1 "470K" H 5930 2695 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 5930 2650 50  0001 C CNN
F 3 "~" H 6000 2650 50  0001 C CNN
	1    6000 2650
	-1   0    0    1   
$EndComp
Wire Wire Line
	6000 2400 6100 2400
Wire Wire Line
	6000 2300 6000 2400
$Comp
L TubeClock:+V_HV #PWR09
U 1 1 5C995B97
P 6000 2300
F 0 "#PWR09" H 6000 2150 50  0001 C CNN
F 1 "+V_HV" H 6015 2473 50  0000 C CNN
F 2 "" H 6000 2300 50  0001 C CNN
F 3 "" H 6000 2300 50  0001 C CNN
	1    6000 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5C995B91
P 6250 2400
F 0 "R5" V 6043 2400 50  0000 C CNN
F 1 "2K2" V 6134 2400 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 6180 2400 50  0001 C CNN
F 3 "~" H 6250 2400 50  0001 C CNN
	1    6250 2400
	0    1    1    0   
$EndComp
Wire Wire Line
	10550 2850 10550 2750
Wire Wire Line
	10450 2750 10450 2850
Wire Wire Line
	10350 2850 10350 2750
Wire Wire Line
	10250 2750 10250 2850
Wire Wire Line
	10150 2850 10150 2750
Wire Wire Line
	10050 2750 10050 2850
Wire Wire Line
	9950 2850 9950 2750
Wire Wire Line
	9850 2750 9850 2850
Wire Wire Line
	9750 2850 9750 2750
Wire Wire Line
	9650 2750 9650 2850
Wire Wire Line
	10550 1450 10550 1550
Wire Wire Line
	10450 1550 10450 1450
Wire Wire Line
	10350 1450 10350 1550
Wire Wire Line
	10250 1550 10250 1450
Wire Wire Line
	10150 1450 10150 1550
Wire Wire Line
	10050 1550 10050 1450
Wire Wire Line
	9950 1450 9950 1550
Wire Wire Line
	9850 1550 9850 1450
Wire Wire Line
	9750 1450 9750 1550
Wire Wire Line
	9650 1550 9650 1450
Wire Wire Line
	7850 2850 7850 2750
Wire Wire Line
	7750 2750 7750 2850
Wire Wire Line
	7650 2850 7650 2750
Wire Wire Line
	7550 2750 7550 2850
Wire Wire Line
	7450 2850 7450 2750
Wire Wire Line
	7850 1450 7850 1550
Wire Wire Line
	7750 1550 7750 1450
Wire Wire Line
	7650 1450 7650 1550
Wire Wire Line
	7550 1550 7550 1450
Wire Wire Line
	7450 1450 7450 1550
Wire Wire Line
	5150 2850 5150 2750
Wire Wire Line
	5050 2750 5050 2850
Wire Wire Line
	4950 2850 4950 2750
Wire Wire Line
	4850 2750 4850 2850
Wire Wire Line
	4750 2850 4750 2750
Wire Wire Line
	4650 2750 4650 2850
Wire Wire Line
	4550 2850 4550 2750
Wire Wire Line
	4450 2750 4450 2850
Wire Wire Line
	4350 2850 4350 2750
Wire Wire Line
	4250 2750 4250 2850
Wire Wire Line
	5150 1550 5150 1450
Wire Wire Line
	5050 1450 5050 1550
Wire Wire Line
	4950 1550 4950 1450
Wire Wire Line
	4850 1450 4850 1550
Wire Wire Line
	4750 1550 4750 1450
Wire Wire Line
	4650 1450 4650 1550
Wire Wire Line
	4550 1550 4550 1450
Wire Wire Line
	4450 1450 4450 1550
Wire Wire Line
	4350 1550 4350 1450
Wire Wire Line
	4250 1450 4250 1550
NoConn ~ 4150 1450
NoConn ~ 5250 1450
NoConn ~ 4150 2750
NoConn ~ 5250 2750
NoConn ~ 6850 1450
NoConn ~ 7950 1450
NoConn ~ 7950 2750
NoConn ~ 9550 1450
NoConn ~ 10650 1450
NoConn ~ 9550 2750
NoConn ~ 10650 2750
Text GLabel 10550 2850 3    50   Input ~ 0
T6_K_9
Text GLabel 10450 2850 3    50   Input ~ 0
T6_K_8
Text GLabel 10350 2850 3    50   Input ~ 0
T6_K_7
Text GLabel 10250 2850 3    50   Input ~ 0
T6_K_6
Text GLabel 10150 2850 3    50   Input ~ 0
T6_K_5
Text GLabel 10050 2850 3    50   Input ~ 0
T6_K_4
Text GLabel 9950 2850 3    50   Input ~ 0
T6_K_3
Text GLabel 9850 2850 3    50   Input ~ 0
T6_K_2
Text GLabel 9750 2850 3    50   Input ~ 0
T6_K_1
Text GLabel 9650 2850 3    50   Input ~ 0
T6_K_0
Text GLabel 7850 2850 3    50   Input ~ 0
T5_K_9
Text GLabel 7750 2850 3    50   Input ~ 0
T5_K_8
Text GLabel 7650 2850 3    50   Input ~ 0
T5_K_7
Text GLabel 7550 2850 3    50   Input ~ 0
T5_K_6
Text GLabel 7450 2850 3    50   Input ~ 0
T5_K_5
Text GLabel 5150 2850 3    50   Input ~ 0
T4_K_9
Text GLabel 5050 2850 3    50   Input ~ 0
T4_K_8
Text GLabel 4950 2850 3    50   Input ~ 0
T4_K_7
Text GLabel 4850 2850 3    50   Input ~ 0
T4_K_6
Text GLabel 4750 2850 3    50   Input ~ 0
T4_K_5
Text GLabel 4650 2850 3    50   Input ~ 0
T4_K_4
Text GLabel 4550 2850 3    50   Input ~ 0
T4_K_3
Text GLabel 4450 2850 3    50   Input ~ 0
T4_K_2
Text GLabel 4350 2850 3    50   Input ~ 0
T4_K_1
Text GLabel 4250 2850 3    50   Input ~ 0
T4_K_0
Text GLabel 10550 1550 3    50   Input ~ 0
T3_K_9
Text GLabel 10450 1550 3    50   Input ~ 0
T3_K_8
Text GLabel 10350 1550 3    50   Input ~ 0
T3_K_7
Text GLabel 10250 1550 3    50   Input ~ 0
T3_K_6
Text GLabel 10150 1550 3    50   Input ~ 0
T3_K_5
Text GLabel 10050 1550 3    50   Input ~ 0
T3_K_4
Text GLabel 9950 1550 3    50   Input ~ 0
T3_K_3
Text GLabel 9850 1550 3    50   Input ~ 0
T3_K_2
Text GLabel 9750 1550 3    50   Input ~ 0
T3_K_1
Text GLabel 9650 1550 3    50   Input ~ 0
T3_K_0
Text GLabel 7850 1550 3    50   Input ~ 0
T2_K_9
Text GLabel 7750 1550 3    50   Input ~ 0
T2_K_8
Text GLabel 7650 1550 3    50   Input ~ 0
T2_K_7
Text GLabel 7550 1550 3    50   Input ~ 0
T2_K_6
Text GLabel 7450 1550 3    50   Input ~ 0
T2_K_5
Text GLabel 5150 1550 3    50   Input ~ 0
T1_K_9
Text GLabel 5050 1550 3    50   Input ~ 0
T1_K_8
Text GLabel 4950 1550 3    50   Input ~ 0
T1_K_7
Text GLabel 4850 1550 3    50   Input ~ 0
T1_K_6
Text GLabel 4750 1550 3    50   Input ~ 0
T1_K_5
Text GLabel 4650 1550 3    50   Input ~ 0
T1_K_4
Text GLabel 4550 1550 3    50   Input ~ 0
T1_K_3
Text GLabel 4450 1550 3    50   Input ~ 0
T1_K_2
Text GLabel 4350 1550 3    50   Input ~ 0
T1_K_1
Text GLabel 4250 1550 3    50   Input ~ 0
T1_K_0
Wire Wire Line
	3700 2400 3800 2400
Wire Wire Line
	3300 2400 3400 2400
Wire Wire Line
	3300 2300 3300 2400
$Comp
L TubeClock:+V_HV #PWR08
U 1 1 5C8E5872
P 3300 2300
F 0 "#PWR08" H 3300 2150 50  0001 C CNN
F 1 "+V_HV" H 3315 2473 50  0000 C CNN
F 2 "" H 3300 2300 50  0001 C CNN
F 3 "" H 3300 2300 50  0001 C CNN
	1    3300 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5C8E586C
P 3550 2400
F 0 "R4" V 3343 2400 50  0000 C CNN
F 1 "2K2" V 3434 2400 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 3480 2400 50  0001 C CNN
F 3 "~" H 3550 2400 50  0001 C CNN
	1    3550 2400
	0    1    1    0   
$EndComp
Wire Wire Line
	6400 1100 6500 1100
Wire Wire Line
	6000 1100 6100 1100
Wire Wire Line
	6000 1000 6000 1100
$Comp
L TubeClock:+V_HV #PWR04
U 1 1 5C8E4DF7
P 6000 1000
F 0 "#PWR04" H 6000 850 50  0001 C CNN
F 1 "+V_HV" H 6015 1173 50  0000 C CNN
F 2 "" H 6000 1000 50  0001 C CNN
F 3 "" H 6000 1000 50  0001 C CNN
	1    6000 1000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5C8E4DF1
P 6250 1100
F 0 "R2" V 6043 1100 50  0000 C CNN
F 1 "2K2" V 6134 1100 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 6180 1100 50  0001 C CNN
F 3 "~" H 6250 1100 50  0001 C CNN
	1    6250 1100
	0    1    1    0   
$EndComp
Wire Wire Line
	3700 1100 3800 1100
Wire Wire Line
	3300 1100 3400 1100
Wire Wire Line
	3300 1000 3300 1100
$Comp
L TubeClock:+V_HV #PWR03
U 1 1 5C8E4073
P 3300 1000
F 0 "#PWR03" H 3300 850 50  0001 C CNN
F 1 "+V_HV" H 3315 1173 50  0000 C CNN
F 2 "" H 3300 1000 50  0001 C CNN
F 3 "" H 3300 1000 50  0001 C CNN
	1    3300 1000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5C8E3DD1
P 3550 1100
F 0 "R1" V 3343 1100 50  0000 C CNN
F 1 "2K2" V 3434 1100 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 3480 1100 50  0001 C CNN
F 3 "~" H 3550 1100 50  0001 C CNN
	1    3550 1100
	0    1    1    0   
$EndComp
$Comp
L TubeClock:IN-18_Nixie_Tube T6
U 1 1 5C8E340B
P 10100 2400
F 0 "T6" H 10100 2765 50  0000 C CNN
F 1 "IN-18_Nixie_Tube" H 10100 2674 50  0000 C CNN
F 2 "TubeClock:Nixie_IN-18" H 10600 2200 50  0001 C CNN
F 3 "" H 10600 2200 50  0001 C CNN
	1    10100 2400
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:IN-18_Nixie_Tube T4
U 1 1 5C8E20B2
P 4700 2400
F 0 "T4" H 4700 2765 50  0000 C CNN
F 1 "IN-18_Nixie_Tube" H 4700 2674 50  0000 C CNN
F 2 "TubeClock:Nixie_IN-18" H 5200 2200 50  0001 C CNN
F 3 "" H 5200 2200 50  0001 C CNN
	1    4700 2400
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:IN-18_Nixie_Tube T2
U 1 1 5C8E1FA2
P 7400 1100
F 0 "T2" H 7400 1465 50  0000 C CNN
F 1 "IN-18_Nixie_Tube" H 7400 1374 50  0000 C CNN
F 2 "TubeClock:Nixie_IN-18" H 7900 900 50  0001 C CNN
F 3 "" H 7900 900 50  0001 C CNN
	1    7400 1100
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:IN-18_Nixie_Tube T1
U 1 1 5C8E1B63
P 4700 1100
F 0 "T1" H 4700 1465 50  0000 C CNN
F 1 "IN-18_Nixie_Tube" H 4700 1374 50  0000 C CNN
F 2 "TubeClock:Nixie_IN-18" H 5200 900 50  0001 C CNN
F 3 "" H 5200 900 50  0001 C CNN
	1    4700 1100
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:IN-18_Nixie_Tube T3
U 1 1 5C8E155F
P 10100 1100
F 0 "T3" H 10100 1465 50  0000 C CNN
F 1 "IN-18_Nixie_Tube" H 10100 1374 50  0000 C CNN
F 2 "TubeClock:Nixie_IN-18" H 10600 900 50  0001 C CNN
F 3 "" H 10600 900 50  0001 C CNN
	1    10100 1100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 1750 2550 1750
Text GLabel 2800 1750 2    50   Output ~ 0
CLK
Connection ~ 1350 1300
Connection ~ 1350 1400
Wire Wire Line
	1350 1400 1350 1300
Connection ~ 1350 1500
Wire Wire Line
	1350 1500 1350 1400
Wire Wire Line
	1350 1600 1350 1500
$Comp
L power:+3V3 #PWR01
U 1 1 5D1AB426
P 1350 750
F 0 "#PWR01" H 1350 600 50  0001 C CNN
F 1 "+3V3" H 1350 900 50  0000 C CNN
F 2 "" H 1350 750 50  0001 C CNN
F 3 "" H 1350 750 50  0001 C CNN
	1    1350 750 
	1    0    0    -1  
$EndComp
Text GLabel 1050 2100 0    50   Input ~ 0
DATA_IN_DB
Text GLabel 1050 2000 0    50   Input ~ 0
CLK_DB
Text GLabel 1050 1900 0    50   Input ~ 0
~BL_DB~
Text GLabel 1050 1800 0    50   Input ~ 0
~LE_DB~
Wire Wire Line
	2800 1850 2550 1850
Wire Wire Line
	2800 1650 2550 1650
Wire Wire Line
	2700 1500 2800 1500
Wire Wire Line
	2700 1550 2700 1500
Wire Wire Line
	2550 1550 2700 1550
Text GLabel 2800 1850 2    50   Output ~ 0
DATA_IN
Text GLabel 2800 1650 2    50   Output ~ 0
~BL~
Text GLabel 2800 1500 2    50   Output ~ 0
~LE~
Wire Wire Line
	1000 1200 1000 1100
Wire Wire Line
	2550 1100 2550 1200
Wire Wire Line
	2550 800  2050 800 
$Comp
L power:GND #PWR07
U 1 1 5C8DA614
P 2550 1200
F 0 "#PWR07" H 2550 950 50  0001 C CNN
F 1 "GND" H 2550 1050 50  0000 C CNN
F 2 "" H 2550 1200 50  0001 C CNN
F 3 "" H 2550 1200 50  0001 C CNN
	1    2550 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 2100 1050 2100
Wire Wire Line
	1050 2000 1350 2000
Wire Wire Line
	1350 1900 1050 1900
Wire Wire Line
	1050 1800 1350 1800
NoConn ~ 1850 2450
Wire Wire Line
	2050 900  2050 800 
$Comp
L TubeClock:CD40109B U1
U 1 1 5C873FA2
P 1950 1700
F 0 "U1" H 2300 1000 60  0000 C CNN
F 1 "CD40109B" H 1500 1000 60  0000 C CNN
F 2 "Package_SO:SOIC-16W_5.3x10.2mm_P1.27mm" H 2050 2200 60  0001 C CNN
F 3 "" H 2050 2200 60  0000 C CNN
	1    1950 1700
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:+V_IN #PWR02
U 1 1 5A7F40F2
P 2550 750
AR Path="/5A7F40F2" Ref="#PWR02"  Part="1" 
AR Path="/5A04077E/5A7F40F2" Ref="#PWR?"  Part="1" 
AR Path="/5C854791/5A7F40F2" Ref="#PWR?"  Part="1" 
F 0 "#PWR02" H 2550 600 50  0001 C CNN
F 1 "+V_IN" H 2550 890 50  0000 C CNN
F 2 "" H 2550 750 50  0001 C CNN
F 3 "" H 2550 750 50  0001 C CNN
	1    2550 750 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5A04E8D7
P 1000 1200
F 0 "#PWR06" H 1000 950 50  0001 C CNN
F 1 "GND" H 1000 1050 50  0000 C CNN
F 2 "" H 1000 1200 50  0001 C CNN
F 3 "" H 1000 1200 50  0001 C CNN
	1    1000 1200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5A04E84B
P 2050 2500
F 0 "#PWR011" H 2050 2250 50  0001 C CNN
F 1 "GND" H 2050 2350 50  0000 C CNN
F 2 "" H 2050 2500 50  0001 C CNN
F 3 "" H 2050 2500 50  0001 C CNN
	1    2050 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5A04E720
P 2550 950
F 0 "C2" H 2575 1050 50  0000 L CNN
F 1 "100 nF" H 2575 850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2588 800 50  0001 C CNN
F 3 "" H 2550 950 50  0001 C CNN
	1    2550 950 
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5A04E682
P 1000 950
F 0 "C1" H 1025 1050 50  0000 L CNN
F 1 "100 nF" H 1025 850 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1038 800 50  0001 C CNN
F 3 "" H 1000 950 50  0001 C CNN
	1    1000 950 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 3400 2150 3400
Wire Wire Line
	1750 3700 1800 3700
Wire Wire Line
	1200 3700 1250 3700
Wire Wire Line
	1250 3600 1200 3600
Wire Wire Line
	1200 3500 1250 3500
Text GLabel 1200 3700 0    50   Output ~ 0
DATA_IN_DB
Text GLabel 2200 3600 2    50   Output ~ 0
CLK_DB
Text GLabel 1200 3500 0    50   Output ~ 0
~BL_DB~
Text GLabel 1200 3600 0    50   Output ~ 0
~LE_DB~
Wire Wire Line
	2300 3400 2300 3100
$Comp
L power:+3V3 #PWR014
U 1 1 5D195A57
P 2300 3100
F 0 "#PWR014" H 2300 2950 50  0001 C CNN
F 1 "+3V3" H 2300 3250 50  0000 C CNN
F 2 "" H 2300 3100 50  0001 C CNN
F 3 "" H 2300 3100 50  0001 C CNN
	1    2300 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1250 3200 650  3200
Wire Wire Line
	1000 3300 1000 3100
Wire Wire Line
	1250 3300 1000 3300
$Comp
L TubeClock:+V_IN #PWR012
U 1 1 5D1804B4
P 1000 3100
AR Path="/5D1804B4" Ref="#PWR012"  Part="1" 
AR Path="/5A04077E/5D1804B4" Ref="#PWR?"  Part="1" 
AR Path="/5C854791/5D1804B4" Ref="#PWR?"  Part="1" 
F 0 "#PWR012" H 1000 2950 50  0001 C CNN
F 1 "+V_IN" H 1000 3250 50  0000 C CNN
F 2 "" H 1000 3100 50  0001 C CNN
F 3 "" H 1000 3100 50  0001 C CNN
	1    1000 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1250 3400 650  3400
$Comp
L power:GND #PWR015
U 1 1 5D155B29
P 650 3800
F 0 "#PWR015" H 650 3550 50  0001 C CNN
F 1 "GND" H 650 3650 50  0000 C CNN
F 2 "" H 650 3800 50  0001 C CNN
F 3 "" H 650 3800 50  0001 C CNN
	1    650  3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR016
U 1 1 5D141CF7
P 2650 3800
F 0 "#PWR016" H 2650 3550 50  0001 C CNN
F 1 "GND" H 2650 3650 50  0000 C CNN
F 2 "" H 2650 3800 50  0001 C CNN
F 3 "" H 2650 3800 50  0001 C CNN
	1    2650 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 3200 1950 3100
Wire Wire Line
	1750 3200 1950 3200
$Comp
L TubeClock:+V_HV #PWR013
U 1 1 5D12E7ED
P 1950 3100
F 0 "#PWR013" H 1950 2950 50  0001 C CNN
F 1 "+V_HV" H 1950 3250 50  0000 C CNN
F 2 "" H 1950 3100 50  0001 C CNN
F 3 "" H 1950 3100 50  0001 C CNN
	1    1950 3100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J1
U 1 1 5CFD3940
P 1450 3400
F 0 "J1" H 1500 3817 50  0000 C CNN
F 1 "Conn_02x06_Odd_Even" H 1500 3726 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical" H 1450 3400 50  0001 C CNN
F 3 "~" H 1450 3400 50  0001 C CNN
	1    1450 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 3600 2200 3600
Wire Wire Line
	1850 3700 1800 3700
Connection ~ 1800 3700
Wire Wire Line
	2150 3800 2200 3800
Wire Wire Line
	2650 3300 2650 3500
Wire Wire Line
	1750 3300 2650 3300
Wire Wire Line
	1750 3500 2650 3500
Connection ~ 2650 3500
Wire Wire Line
	2650 3500 2650 3800
$Comp
L Device:R R7
U 1 1 5C9538A0
P 5800 2650
F 0 "R7" H 5950 2600 50  0000 R CNN
F 1 "470K" H 6050 2700 50  0000 R CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 5730 2650 50  0001 C CNN
F 3 "~" H 5800 2650 50  0001 C CNN
	1    5800 2650
	-1   0    0    1   
$EndComp
Wire Wire Line
	5800 2500 5800 2400
Wire Wire Line
	5800 2400 6000 2400
$Comp
L TubeClock:IN-18_Nixie_Tube T5
U 1 1 5C8E2CEF
P 7400 2400
F 0 "T5" H 7400 2765 50  0000 C CNN
F 1 "IN-18_Nixie_Tube" H 7400 2674 50  0000 C CNN
F 2 "TubeClock:Nixie_IN-18" H 7900 2200 50  0001 C CNN
F 3 "" H 7900 2200 50  0001 C CNN
	1    7400 2400
	1    0    0    -1  
$EndComp
Text GLabel 6950 1550 3    50   Input ~ 0
T2_K_0
Text GLabel 7050 1550 3    50   Input ~ 0
T2_K_1
Text GLabel 7150 1550 3    50   Input ~ 0
T2_K_2
Text GLabel 7250 1550 3    50   Input ~ 0
T2_K_3
Text GLabel 7350 1550 3    50   Input ~ 0
T2_K_4
Text GLabel 6950 2850 3    50   Input ~ 0
T5_K_0
Text GLabel 7050 2850 3    50   Input ~ 0
T5_K_1
Text GLabel 7150 2850 3    50   Input ~ 0
T5_K_2
Text GLabel 7250 2850 3    50   Input ~ 0
T5_K_3
Text GLabel 7350 2850 3    50   Input ~ 0
T5_K_4
NoConn ~ 6850 2750
Wire Wire Line
	6950 1550 6950 1450
Wire Wire Line
	7050 1450 7050 1550
Wire Wire Line
	7150 1550 7150 1450
Wire Wire Line
	7250 1450 7250 1550
Wire Wire Line
	7350 1550 7350 1450
Wire Wire Line
	6950 2750 6950 2850
Wire Wire Line
	7050 2850 7050 2750
Wire Wire Line
	7150 2750 7150 2850
Wire Wire Line
	7250 2850 7250 2750
Wire Wire Line
	7350 2750 7350 2850
$Comp
L Device:Lamp_Neon NE1
U 1 1 5C952BDD
P 5800 3100
F 0 "NE1" V 5535 3100 50  0000 C CNN
F 1 "Lamp_Neon" V 5626 3100 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" V 5800 3200 50  0001 C CNN
F 3 "~" V 5800 3200 50  0001 C CNN
	1    5800 3100
	-1   0    0    1   
$EndComp
Text GLabel 5800 3350 3    50   Input ~ 0
T7_K
Wire Wire Line
	6400 2400 6500 2400
$Comp
L Device:Lamp_Neon NE2
U 1 1 5C995BA0
P 6250 3100
F 0 "NE2" V 5985 3100 50  0000 C CNN
F 1 "Lamp_Neon" V 6076 3100 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" V 6250 3200 50  0001 C CNN
F 3 "~" V 6250 3200 50  0001 C CNN
	1    6250 3100
	-1   0    0    1   
$EndComp
Wire Wire Line
	8300 1950 6500 1950
Wire Wire Line
	6500 1950 6500 2400
Wire Wire Line
	8700 2400 8500 2400
Wire Wire Line
	8500 2400 8500 2500
Wire Wire Line
	2200 6950 2200 7050
Wire Wire Line
	2100 7050 2100 6950
Wire Wire Line
	2000 6950 2000 7050
Wire Wire Line
	1900 7050 1900 6950
Connection ~ 650  6700
Wire Wire Line
	650  6400 650  6700
Wire Wire Line
	650  6700 650  6800
Wire Wire Line
	1450 6700 650  6700
$Comp
L power:GND #PWR025
U 1 1 5CA5E9C5
P 650 6800
F 0 "#PWR025" H 650 6550 50  0001 C CNN
F 1 "GND" H 650 6650 50  0000 C CNN
F 2 "" H 650 6800 50  0001 C CNN
F 3 "" H 650 6800 50  0001 C CNN
	1    650  6800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5CA5E9B9
P 650 6250
F 0 "C3" H 675 6350 50  0000 L CNN
F 1 "100 nF" H 675 6150 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 688 6100 50  0001 C CNN
F 3 "" H 650 6250 50  0001 C CNN
	1    650  6250
	1    0    0    -1  
$EndComp
NoConn ~ 1450 6500
NoConn ~ 1450 6400
NoConn ~ 1450 6300
NoConn ~ 1450 6200
Text GLabel 1900 7050 3    50   Output ~ 0
T4_K_8
Text GLabel 2000 7050 3    50   Output ~ 0
T4_K_9
Text GLabel 2100 7050 3    50   Output ~ 0
T3_K_0
Text GLabel 2200 7050 3    50   Output ~ 0
T3_K_1
$Comp
L TubeClock:HV5622 U2
U 1 1 5C8CE18D
P 2500 5950
F 0 "U2" H 3250 5050 60  0000 L CNN
F 1 "HV5622" H 3000 6800 60  0000 L CNN
F 2 "Package_LCC:PLCC-44_16.6x16.6mm_P1.27mm" V 2450 6400 60  0001 C CNN
F 3 "" V 2450 6400 60  0000 C CNN
	1    2500 5950
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:HV5622 U3
U 1 1 5C8CE024
P 6000 5950
F 0 "U3" H 6750 5050 60  0000 L CNN
F 1 "HV5622" H 6500 6800 60  0000 L CNN
F 2 "Package_LCC:PLCC-44_16.6x16.6mm_P1.27mm" V 5950 6400 60  0001 C CNN
F 3 "" V 5950 6400 60  0000 C CNN
	1    6000 5950
	1    0    0    -1  
$EndComp
Text GLabel 4850 5800 0    50   Input ~ 0
~LE~
Text GLabel 4850 5500 0    50   Input ~ 0
CLK
Text GLabel 4850 5600 0    50   Input ~ 0
DATA_THROUGH
Text GLabel 4850 5700 0    50   Output ~ 0
DATA_OUT
Text GLabel 4850 5950 0    50   Input ~ 0
~BL~
Wire Wire Line
	4850 5500 4950 5500
Wire Wire Line
	4950 5600 4850 5600
Wire Wire Line
	4850 5700 4950 5700
Wire Wire Line
	4950 5800 4850 5800
Wire Wire Line
	4850 5950 4950 5950
Text GLabel 5500 7050 3    50   Output ~ 0
T8_K
Text GLabel 5400 7050 3    50   Output ~ 0
T7_K
Text GLabel 5550 4900 1    50   Output ~ 0
T4_K_7
Text GLabel 5650 4900 1    50   Output ~ 0
T4_K_6
Text GLabel 5750 4900 1    50   Output ~ 0
T4_K_5
Text GLabel 5850 4900 1    50   Output ~ 0
T4_K_4
Text GLabel 5950 4900 1    50   Output ~ 0
T4_K_3
Text GLabel 6050 4900 1    50   Output ~ 0
T4_K_2
Text GLabel 6150 4900 1    50   Output ~ 0
T4_K_1
Text GLabel 6250 4900 1    50   Output ~ 0
T4_K_0
Text GLabel 6350 4900 1    50   Output ~ 0
T5_K_9
Text GLabel 6450 4900 1    50   Output ~ 0
T5_K_8
Text GLabel 7100 5500 2    50   Output ~ 0
T5_K_7
Text GLabel 7100 5600 2    50   Output ~ 0
T5_K_6
Text GLabel 7100 5700 2    50   Output ~ 0
T5_K_5
Text GLabel 7100 5800 2    50   Output ~ 0
T5_K_4
Text GLabel 7100 5900 2    50   Output ~ 0
T5_K_3
Text GLabel 7100 6000 2    50   Output ~ 0
T5_K_2
Text GLabel 7100 6100 2    50   Output ~ 0
T5_K_1
Text GLabel 7100 6200 2    50   Output ~ 0
T5_K_0
Text GLabel 7100 6300 2    50   Output ~ 0
T6_K_9
Text GLabel 7100 6400 2    50   Output ~ 0
T6_K_8
Text GLabel 6500 7050 3    50   Output ~ 0
T6_K_7
Text GLabel 6400 7050 3    50   Output ~ 0
T6_K_6
Text GLabel 6300 7050 3    50   Output ~ 0
T6_K_5
Text GLabel 6200 7050 3    50   Output ~ 0
T6_K_4
Text GLabel 6100 7050 3    50   Output ~ 0
T6_K_3
Text GLabel 6000 7050 3    50   Output ~ 0
T6_K_2
Text GLabel 5900 7050 3    50   Output ~ 0
T6_K_1
Text GLabel 5800 7050 3    50   Output ~ 0
T6_K_0
Text GLabel 5700 7050 3    50   Output ~ 0
T10_K
Text GLabel 5600 7050 3    50   Output ~ 0
T9_K
$Comp
L TubeClock:+V_IN #PWR024
U 1 1 5CA17702
P 4850 5200
F 0 "#PWR024" H 4850 5050 50  0001 C CNN
F 1 "+V_IN" H 4865 5373 50  0000 C CNN
F 2 "" H 4850 5200 50  0001 C CNN
F 3 "" H 4850 5200 50  0001 C CNN
	1    4850 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 5200 4850 5300
Wire Wire Line
	4850 5300 4950 5300
NoConn ~ 4950 6200
NoConn ~ 4950 6300
NoConn ~ 4950 6400
NoConn ~ 4950 6500
$Comp
L Device:C C4
U 1 1 5CA68FF6
P 4150 6250
F 0 "C4" H 4175 6350 50  0000 L CNN
F 1 "100 nF" H 4175 6150 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4188 6100 50  0001 C CNN
F 3 "" H 4150 6250 50  0001 C CNN
	1    4150 6250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR026
U 1 1 5CA68FFC
P 4150 6800
F 0 "#PWR026" H 4150 6550 50  0001 C CNN
F 1 "GND" H 4150 6650 50  0000 C CNN
F 2 "" H 4150 6800 50  0001 C CNN
F 3 "" H 4150 6800 50  0001 C CNN
	1    4150 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 6700 4150 6700
Wire Wire Line
	4150 6700 4150 6800
Wire Wire Line
	4150 6400 4150 6700
Connection ~ 4150 6700
Wire Wire Line
	4150 6100 4150 6050
Wire Wire Line
	4150 5300 4850 5300
Connection ~ 4850 5300
Wire Wire Line
	4950 6050 4150 6050
Connection ~ 4150 6050
Wire Wire Line
	4150 6050 4150 5300
Wire Wire Line
	5400 6950 5400 7050
Wire Wire Line
	5500 7050 5500 6950
Wire Wire Line
	5600 6950 5600 7050
Wire Wire Line
	5700 7050 5700 6950
Wire Wire Line
	5800 6950 5800 7050
Wire Wire Line
	5900 7050 5900 6950
Wire Wire Line
	6000 6950 6000 7050
Wire Wire Line
	6100 7050 6100 6950
Wire Wire Line
	6200 6950 6200 7050
Wire Wire Line
	6300 7050 6300 6950
Wire Wire Line
	6400 6950 6400 7050
Wire Wire Line
	6500 7050 6500 6950
Wire Wire Line
	7000 6400 7100 6400
Wire Wire Line
	7100 6300 7000 6300
Wire Wire Line
	7000 6200 7100 6200
Wire Wire Line
	7100 6100 7000 6100
Wire Wire Line
	7000 6000 7100 6000
Wire Wire Line
	7100 5900 7000 5900
Wire Wire Line
	7000 5800 7100 5800
Wire Wire Line
	7100 5700 7000 5700
Wire Wire Line
	7000 5600 7100 5600
Wire Wire Line
	7100 5500 7000 5500
Wire Wire Line
	6450 5000 6450 4900
Wire Wire Line
	6350 4900 6350 5000
Wire Wire Line
	6250 5000 6250 4900
Wire Wire Line
	6150 4900 6150 5000
Wire Wire Line
	6050 5000 6050 4900
Wire Wire Line
	5950 4900 5950 5000
Wire Wire Line
	5850 5000 5850 4900
Wire Wire Line
	5750 4900 5750 5000
Wire Wire Line
	5650 5000 5650 4900
Wire Wire Line
	5550 4900 5550 5000
Wire Wire Line
	2550 800  2550 750 
Connection ~ 2550 800 
Wire Wire Line
	2050 2450 2050 2500
Wire Wire Line
	650  3800 650  3400
Connection ~ 650  3400
Wire Wire Line
	650  3400 650  3200
Text GLabel 6250 3350 3    50   Input ~ 0
T8_K
Text GLabel 8500 3350 3    50   Input ~ 0
T9_K
Wire Wire Line
	5800 3350 5800 3300
Wire Wire Line
	6250 3300 6250 3350
Wire Wire Line
	8500 3350 8500 3300
Wire Wire Line
	8950 3300 8950 3350
Wire Wire Line
	5800 2800 5800 2900
Wire Wire Line
	6000 2800 6000 2850
Wire Wire Line
	6000 2850 6250 2850
Wire Wire Line
	6250 2850 6250 2900
Wire Wire Line
	8500 2800 8500 2900
Wire Wire Line
	8700 2800 8700 2850
Wire Wire Line
	8700 2850 8950 2850
Wire Wire Line
	8950 2850 8950 2900
Wire Wire Line
	1350 750  1350 800 
Wire Wire Line
	1850 900  1850 800 
Wire Wire Line
	1850 800  1350 800 
Connection ~ 1350 800 
Wire Wire Line
	1350 800  1350 1300
Wire Wire Line
	1000 800  1350 800 
$Comp
L Mechanical:MountingHole_Pad H1
U 1 1 5C99B394
P 3100 3600
F 0 "H1" H 3200 3651 50  0000 L CNN
F 1 "MountingHole_Pad" H 3200 3560 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 3100 3600 50  0001 C CNN
F 3 "~" H 3100 3600 50  0001 C CNN
	1    3100 3600
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H2
U 1 1 5C99B5DE
P 3400 3600
F 0 "H2" H 3500 3651 50  0000 L CNN
F 1 "MountingHole_Pad" H 3500 3560 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 3400 3600 50  0001 C CNN
F 3 "~" H 3400 3600 50  0001 C CNN
	1    3400 3600
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H3
U 1 1 5C99B630
P 3700 3600
F 0 "H3" H 3800 3651 50  0000 L CNN
F 1 "MountingHole_Pad" H 3800 3560 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 3700 3600 50  0001 C CNN
F 3 "~" H 3700 3600 50  0001 C CNN
	1    3700 3600
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H4
U 1 1 5C99B686
P 4000 3600
F 0 "H4" H 4100 3651 50  0000 L CNN
F 1 "MountingHole_Pad" H 4100 3560 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 4000 3600 50  0001 C CNN
F 3 "~" H 4000 3600 50  0001 C CNN
	1    4000 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR017
U 1 1 5C99B6DE
P 3100 3800
F 0 "#PWR017" H 3100 3550 50  0001 C CNN
F 1 "GND" H 3100 3650 50  0000 C CNN
F 2 "" H 3100 3800 50  0001 C CNN
F 3 "" H 3100 3800 50  0001 C CNN
	1    3100 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR018
U 1 1 5C99B72D
P 3400 3800
F 0 "#PWR018" H 3400 3550 50  0001 C CNN
F 1 "GND" H 3400 3650 50  0000 C CNN
F 2 "" H 3400 3800 50  0001 C CNN
F 3 "" H 3400 3800 50  0001 C CNN
	1    3400 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 5C99B77C
P 3700 3800
F 0 "#PWR019" H 3700 3550 50  0001 C CNN
F 1 "GND" H 3700 3650 50  0000 C CNN
F 2 "" H 3700 3800 50  0001 C CNN
F 3 "" H 3700 3800 50  0001 C CNN
	1    3700 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR020
U 1 1 5C99B7CB
P 4000 3800
F 0 "#PWR020" H 4000 3550 50  0001 C CNN
F 1 "GND" H 4000 3650 50  0000 C CNN
F 2 "" H 4000 3800 50  0001 C CNN
F 3 "" H 4000 3800 50  0001 C CNN
	1    4000 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 3700 3100 3800
Wire Wire Line
	3400 3800 3400 3700
Wire Wire Line
	3700 3700 3700 3800
Wire Wire Line
	4000 3800 4000 3700
$Comp
L Mechanical:MountingHole_Pad H5
U 1 1 5CA11F3F
P 4300 3600
F 0 "H5" H 4400 3651 50  0000 L CNN
F 1 "MountingHole_Pad" H 4400 3560 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 4300 3600 50  0001 C CNN
F 3 "~" H 4300 3600 50  0001 C CNN
	1    4300 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR021
U 1 1 5CA11F45
P 4300 3800
F 0 "#PWR021" H 4300 3550 50  0001 C CNN
F 1 "GND" H 4300 3650 50  0000 C CNN
F 2 "" H 4300 3800 50  0001 C CNN
F 3 "" H 4300 3800 50  0001 C CNN
	1    4300 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 3700 4300 3800
$Comp
L Mechanical:MountingHole_Pad H6
U 1 1 5CA26140
P 4600 3600
F 0 "H6" H 4700 3651 50  0000 L CNN
F 1 "MountingHole_Pad" H 4700 3560 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad_Via" H 4600 3600 50  0001 C CNN
F 3 "~" H 4600 3600 50  0001 C CNN
	1    4600 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR022
U 1 1 5CA26146
P 4600 3800
F 0 "#PWR022" H 4600 3550 50  0001 C CNN
F 1 "GND" H 4600 3650 50  0000 C CNN
F 2 "" H 4600 3800 50  0001 C CNN
F 3 "" H 4600 3800 50  0001 C CNN
	1    4600 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3700 4600 3800
$Comp
L Device:D D1
U 1 1 5DDD194E
P 2000 3800
F 0 "D1" H 2000 4000 50  0000 C CNN
F 1 "1N4148" H 2000 3900 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123" H 2000 3800 50  0001 C CNN
F 3 "~" H 2000 3800 50  0001 C CNN
	1    2000 3800
	-1   0    0    1   
$EndComp
$Comp
L Device:R R11
U 1 1 5CCE46BC
P 2000 3700
F 0 "R11" H 1930 3654 50  0000 R CNN
F 1 "10K" H 1930 3745 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1930 3700 50  0001 C CNN
F 3 "~" H 2000 3700 50  0001 C CNN
	1    2000 3700
	0    1    1    0   
$EndComp
Text GLabel 2200 3800 2    50   Input ~ 0
DATA_OUT
Wire Wire Line
	1850 3800 1800 3800
Wire Wire Line
	1800 3700 1800 3800
Wire Wire Line
	2150 3700 2150 3400
Connection ~ 2150 3400
Wire Wire Line
	2150 3400 2300 3400
$EndSCHEMATC
