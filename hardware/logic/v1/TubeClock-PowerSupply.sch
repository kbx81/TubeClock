EESchema Schematic File Version 4
LIBS:TubeClock-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 6
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
L TubeClock:Jack-DC J?
U 1 1 5A06C211
P 5050 1200
AR Path="/5A06C211" Ref="J?"  Part="1" 
AR Path="/5A06BFE7/5A06C211" Ref="J?"  Part="1" 
AR Path="/5C7A430C/5A06C211" Ref="J?"  Part="1" 
AR Path="/5C7E1816/5A06C211" Ref="J1"  Part="1" 
F 0 "J1" H 5050 1410 50  0000 C CNN
F 1 "Jack-DC" H 5050 1025 50  0000 C CNN
F 2 "Connector_BarrelJack:BarrelJack_Horizontal" H 5100 1160 50  0001 C CNN
F 3 "" H 5100 1160 50  0001 C CNN
	1    5050 1200
	1    0    0    -1  
$EndComp
$Comp
L Device:C C9
U 1 1 5A06C24E
P 8100 3900
F 0 "C9" H 8125 4000 50  0000 L CNN
F 1 "10 uF" H 8125 3800 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 8138 3750 50  0001 C CNN
F 3 "" H 8100 3900 50  0001 C CNN
	1    8100 3900
	1    0    0    -1  
$EndComp
$Comp
L Device:C C8
U 1 1 5A06C283
P 7100 3900
F 0 "C8" H 7125 4000 50  0000 L CNN
F 1 "100 nF" H 7125 3800 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 7138 3750 50  0001 C CNN
F 3 "" H 7100 3900 50  0001 C CNN
	1    7100 3900
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT1
U 1 1 5A06C2CC
P 10650 4050
F 0 "BT1" H 10750 4150 50  0000 L CNN
F 1 "CR2032" H 10750 4050 50  0000 L CNN
F 2 "TubeClock:BatteryHolder_Keystone_3034_1x20mm" V 10650 4110 50  0001 C CNN
F 3 "http://www.keyelco.com/product.cfm/product_id/719" V 10650 4110 50  0001 C CNN
F 4 "103" H 10650 4050 60  0001 C CNN "Part Number"
	1    10650 4050
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 5A0AB81C
P 6350 1250
F 0 "C1" H 6375 1350 50  0000 L CNN
F 1 "100 uF" H 6375 1150 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_8x10" H 6388 1100 50  0001 C CNN
F 3 "https://industrial.panasonic.com/ww/products/capacitors/polymer-capacitors/os-con/svf/25SVF100M" H 6350 1250 50  0001 C CNN
F 4 "25SVF100M" H 6350 1250 60  0001 C CNN "Part Number"
	1    6350 1250
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR012
U 1 1 5A0AB93A
P 8500 3450
F 0 "#PWR012" H 8500 3300 50  0001 C CNN
F 1 "+3.3V" H 8500 3590 50  0000 C CNN
F 2 "" H 8500 3450 50  0001 C CNN
F 3 "" H 8500 3450 50  0001 C CNN
	1    8500 3450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5A0ABA04
P 6350 1600
F 0 "#PWR07" H 6350 1350 50  0001 C CNN
F 1 "GND" H 6350 1450 50  0000 C CNN
F 2 "" H 6350 1600 50  0001 C CNN
F 3 "" H 6350 1600 50  0001 C CNN
	1    6350 1600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR015
U 1 1 5A0ABA30
P 7100 4250
F 0 "#PWR015" H 7100 4000 50  0001 C CNN
F 1 "GND" H 7100 4100 50  0000 C CNN
F 2 "" H 7100 4250 50  0001 C CNN
F 3 "" H 7100 4250 50  0001 C CNN
	1    7100 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR016
U 1 1 5A0ABA5C
P 7600 4250
F 0 "#PWR016" H 7600 4000 50  0001 C CNN
F 1 "GND" H 7600 4100 50  0000 C CNN
F 2 "" H 7600 4250 50  0001 C CNN
F 3 "" H 7600 4250 50  0001 C CNN
	1    7600 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR017
U 1 1 5A0ABA88
P 8100 4250
F 0 "#PWR017" H 8100 4000 50  0001 C CNN
F 1 "GND" H 8100 4100 50  0000 C CNN
F 2 "" H 8100 4250 50  0001 C CNN
F 3 "" H 8100 4250 50  0001 C CNN
	1    8100 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR020
U 1 1 5A0ABAB4
P 10650 4250
F 0 "#PWR020" H 10650 4000 50  0001 C CNN
F 1 "GND" H 10650 4100 50  0000 C CNN
F 2 "" H 10650 4250 50  0001 C CNN
F 3 "" H 10650 4250 50  0001 C CNN
	1    10650 4250
	1    0    0    -1  
$EndComp
$Comp
L Device:Polyfuse F1
U 1 1 5A0B6BAC
P 5600 1100
F 0 "F1" V 5500 1100 50  0000 C CNN
F 1 "~500mA" V 5700 1100 50  0000 C CNN
F 2 "Fuse:Fuse_2920_7451Metric" H 5650 900 50  0001 L CNN
F 3 "http://m.littelfuse.com/~/media/electronics/datasheets/resettable_ptcs/littelfuse_ptc_2920l_datasheet.pdf.pdf" H 5600 1100 50  0001 C CNN
F 4 "2920L185DR" V 5600 1100 60  0001 C CNN "Part Number"
	1    5600 1100
	0    1    1    0   
$EndComp
$Comp
L win:DIODE D1
U 1 1 5A0B6C19
P 6050 1100
F 0 "D1" H 6050 1200 40  0000 C CNN
F 1 "S3A" H 6050 1000 40  0000 C CNN
F 2 "Diode_SMD:D_SMC" H 6050 1100 60  0001 C CNN
F 3 "http://www.onsemi.com/pub/Collateral/S3N-D.PDF" H 6050 1100 60  0001 C CNN
F 4 "S3A" H 6050 1100 60  0001 C CNN "Part Number"
	1    6050 1100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5A0B6F52
P 5450 1600
F 0 "#PWR06" H 5450 1350 50  0001 C CNN
F 1 "GND" H 5450 1450 50  0000 C CNN
F 2 "" H 5450 1600 50  0001 C CNN
F 3 "" H 5450 1600 50  0001 C CNN
	1    5450 1600
	1    0    0    -1  
$EndComp
$Comp
L win:LD1117S33TR_SOT223 U2
U 1 1 5A06C0AF
P 7600 3750
F 0 "U2" H 7750 3500 50  0000 C CNN
F 1 "LD1117S33TR" H 7350 3900 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:TO-252-2" H 7600 3950 50  0001 C CNN
F 3 "http://www.st.com/content/ccc/resource/technical/document/datasheet/a5/c3/3f/c9/2b/15/40/49/CD00002116.pdf/files/CD00002116.pdf/jcr:content/translations/en.CD00002116.pdf" H 7700 3500 50  0001 C CNN
F 4 "LD1117ADT33TR" H 7600 3750 60  0001 C CNN "Part Number"
	1    7600 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 1300 5450 1600
Wire Wire Line
	5350 1300 5450 1300
Wire Wire Line
	5750 1100 5850 1100
Wire Wire Line
	5350 1100 5450 1100
Wire Wire Line
	8100 4250 8100 4050
Wire Wire Line
	7600 4050 7600 4250
Wire Wire Line
	7100 4250 7100 4050
Wire Wire Line
	6350 1400 6350 1600
Wire Wire Line
	6350 1100 6350 800 
Wire Wire Line
	5350 1200 5350 1300
$Comp
L TubeClock:+V_IN #PWR?
U 1 1 5A7F3BB0
P 6350 800
AR Path="/5A7F3BB0" Ref="#PWR?"  Part="1" 
AR Path="/5A06BFE7/5A7F3BB0" Ref="#PWR?"  Part="1" 
AR Path="/5C7A430C/5A7F3BB0" Ref="#PWR?"  Part="1" 
AR Path="/5C7E1816/5A7F3BB0" Ref="#PWR05"  Part="1" 
F 0 "#PWR05" H 6350 650 50  0001 C CNN
F 1 "+V_IN" H 6350 940 50  0000 C CNN
F 2 "" H 6350 800 50  0001 C CNN
F 3 "" H 6350 800 50  0001 C CNN
	1    6350 800 
	1    0    0    -1  
$EndComp
$Comp
L Device:D_x2_Serial_AKC D3
U 1 1 5B261F0B
P 9400 3750
F 0 "D3" H 9450 3650 50  0000 C CNN
F 1 "BAT64-04" H 9400 3850 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 9400 3750 50  0001 C CNN
F 3 "https://www.infineon.com/dgdl/Infineon-BAT64SERIES-DS-v01_01-en.pdf?fileId=db3a304314dca3890115191757980ead" H 9400 3750 50  0001 C CNN
	1    9400 3750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5B261F78
P 9400 4200
F 0 "R3" V 9480 4200 50  0000 C CNN
F 1 "56Ω 0.5W" V 9300 4200 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9330 4200 50  0001 C CNN
F 3 "" H 9400 4200 50  0001 C CNN
	1    9400 4200
	-1   0    0    1   
$EndComp
$Comp
L Device:C C11
U 1 1 5B262008
P 9850 4000
F 0 "C11" H 9875 4100 50  0000 L CNN
F 1 "100 nF" H 9875 3900 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 9888 3850 50  0001 C CNN
F 3 "" H 9850 4000 50  0001 C CNN
	1    9850 4000
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C12
U 1 1 5B2620F6
P 9400 4600
F 0 "C12" H 9425 4700 50  0000 L CNN
F 1 "470 mF" H 9425 4500 50  0000 L CNN
F 2 "TubeClock:CP_Radial_D13.0mm_P5.00mm_HD1.5mm" H 9438 4450 50  0001 C CNN
F 3 "http://www.cooperindustries.com/content/dam/public/bussmann/Electronics/Resources/product-datasheets/Bus_Elx_DS_4327_KR_Series.pdf" H 9400 4600 50  0001 C CNN
F 4 "KR-5R5H474-R" H 9400 4600 60  0001 C CNN "Part Number"
	1    9400 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 3950 9400 4050
Wire Wire Line
	9400 4350 9400 4450
$Comp
L power:GND #PWR030
U 1 1 5B26234F
P 9400 4850
F 0 "#PWR030" H 9400 4600 50  0001 C CNN
F 1 "GND" H 9400 4700 50  0000 C CNN
F 2 "" H 9400 4850 50  0001 C CNN
F 3 "" H 9400 4850 50  0001 C CNN
	1    9400 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 4850 9400 4750
$Comp
L power:GND #PWR019
U 1 1 5B2623AE
P 9850 4250
F 0 "#PWR019" H 9850 4000 50  0001 C CNN
F 1 "GND" H 9850 4100 50  0000 C CNN
F 2 "" H 9850 4250 50  0001 C CNN
F 3 "" H 9850 4250 50  0001 C CNN
	1    9850 4250
	1    0    0    -1  
$EndComp
Wire Wire Line
	9850 4150 9850 4250
Wire Wire Line
	9700 3750 9850 3750
Wire Wire Line
	9850 3450 9850 3750
$Comp
L power:+BATT #PWR013
U 1 1 5B262432
P 9850 3450
F 0 "#PWR013" H 9850 3300 50  0001 C CNN
F 1 "+BATT" H 9850 3590 50  0000 C CNN
F 2 "" H 9850 3450 50  0001 C CNN
F 3 "" H 9850 3450 50  0001 C CNN
	1    9850 3450
	1    0    0    -1  
$EndComp
Connection ~ 9850 3750
$Comp
L Device:D D4
U 1 1 5B2628DF
P 10250 3750
F 0 "D4" H 10250 3650 50  0000 C CNN
F 1 "BAT64-02V" H 10250 3850 50  0000 C CNN
F 2 "Diode_SMD:D_0603_1608Metric" H 10250 3750 50  0001 C CNN
F 3 "https://www.infineon.com/dgdl/Infineon-BAT64SERIES-DS-v01_01-en.pdf?fileId=db3a304314dca3890115191757980ead" H 10250 3750 50  0001 C CNN
	1    10250 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	10650 4150 10650 4250
Wire Wire Line
	10650 3850 10650 3750
Text Notes 9300 4700 2    50   ~ 0
Supercapacitor\nEaton/PowerStor\nKR-5R5C474-R
Text Notes 9250 4350 2    50   ~ 0
Use anti-surge\n SMD chip!\nE.g.: Panasonic\nERJ-P6WF56R0V
Wire Wire Line
	9850 3750 10100 3750
Wire Wire Line
	9850 3750 9850 3850
Wire Wire Line
	7100 3750 7300 3750
$Comp
L Device:CP C2
U 1 1 5C749BD5
P 3450 2650
F 0 "C2" H 3500 2750 50  0000 L CNN
F 1 "100 uF 35 V" H 3200 2550 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-7343-31_Kemet-D" H 3488 2500 50  0001 C CNN
F 3 "~" H 3450 2650 50  0001 C CNN
F 4 "Capacitor, Tantalum, EU 7343 case, polarized" H 3450 2650 50  0001 C CNN "Notes"
F 5 "TPSD107M020R0085" H 3450 2650 50  0001 C CNN "SuggestedPart"
	1    3450 2650
	1    0    0    -1  
$EndComp
$Comp
L Device:L L1
U 1 1 5C749C5B
P 3750 2400
F 0 "L1" V 3940 2400 50  0000 C CNN
F 1 "100µH 2A" V 3849 2400 50  0000 C CNN
F 2 "TubeClock:L_EPCOS_B82479A1_Handsoldering" H 3750 2400 50  0001 C CNN
F 3 "~" H 3750 2400 50  0001 C CNN
F 4 "SMT power inductors" V 3750 2400 50  0001 C CNN "Notes"
F 5 "Epcos B82479-A1-104M" V 3750 2400 50  0001 C CNN "SuggestedPart"
	1    3750 2400
	0    -1   -1   0   
$EndComp
$Comp
L Device:D D2
U 1 1 5C749D22
P 4350 2900
F 0 "D2" H 4350 2684 50  0000 C CNN
F 1 "ES2F" H 4350 2775 50  0000 C CNN
F 2 "Diode_SMD:D_SMB" H 4350 2900 50  0001 C CNN
F 3 "~" H 4350 2900 50  0001 C CNN
	1    4350 2900
	-1   0    0    1   
$EndComp
$Comp
L Device:CP C4
U 1 1 5C749E83
P 1650 3850
F 0 "C4" H 1700 3950 50  0000 L CNN
F 1 "10 uF 25 V" H 1450 3750 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-7343-31_Kemet-D" H 1688 3700 50  0001 C CNN
F 3 "~" H 1650 3850 50  0001 C CNN
F 4 "Capacitor, Tantalum, EU 7343 case, polarized" H 1650 3850 50  0001 C CNN "Notes"
F 5 "TPSD106K035R0300" H 1650 3850 50  0001 C CNN "SuggestedPart"
	1    1650 3850
	1    0    0    -1  
$EndComp
$Comp
L Device:C C5
U 1 1 5C749F05
P 1950 3850
F 0 "C5" H 2000 3950 50  0000 L CNN
F 1 "100 nF" H 2000 3750 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1988 3700 50  0001 C CNN
F 3 "~" H 1950 3850 50  0001 C CNN
F 4 "Any generic cap" H 1950 3850 50  0001 C CNN "Notes"
	1    1950 3850
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 5C749F81
P 2250 3850
F 0 "C6" H 2300 3950 50  0000 L CNN
F 1 "100 nF" H 2300 3750 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2288 3700 50  0001 C CNN
F 3 "~" H 2250 3850 50  0001 C CNN
F 4 "Any generic cap" H 2250 3850 50  0001 C CNN "Notes"
	1    2250 3850
	1    0    0    -1  
$EndComp
$Comp
L Transistor_FET:IRF740 Q1
U 1 1 5C74A11E
P 3900 3400
F 0 "Q1" H 4105 3446 50  0000 L CNN
F 1 " IRF644PBF" H 4050 3350 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-220-3_Horizontal_TabDown" H 4150 3325 50  0001 L CIN
F 3 "http://www.vishay.com/docs/91039/sihf644.pdf" H 3900 3400 50  0001 L CNN
	1    3900 3400
	1    0    0    -1  
$EndComp
$Comp
L Device:R RSENSE1
U 1 1 5C74A663
P 4000 4150
F 0 "RSENSE1" H 4070 4196 50  0000 L CNN
F 1 ".05" H 4070 4105 50  0000 L CNN
F 2 "Resistor_SMD:R_2010_5025Metric" V 3930 4150 50  0001 C CNN
F 3 "~" H 4000 4150 50  0001 C CNN
F 4 "2A current rating - 1 Watt in a 2010 package" H 4000 4150 50  0001 C CNN "Notes"
F 5 "Welwyn LR2010-R05FW" H 4000 4150 50  0001 C CNN "SuggestedPart"
	1    4000 4150
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C3
U 1 1 5C74A86C
P 4650 3350
F 0 "C3" H 4700 3450 50  0000 L CNN
F 1 "4.7 uF" H 4700 3250 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_10x10" H 4688 3200 50  0001 C CNN
F 3 "~" H 4650 3350 50  0001 C CNN
F 4 "Capacitor, Electrolytic, polarized" H 4650 3350 50  0001 C CNN "Notes"
F 5 "EEV-EB2E100Q" H 4650 3350 50  0001 C CNN "SuggestedPart"
	1    4650 3350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5C74A9A1
P 5150 3150
F 0 "R2" H 5220 3196 50  0000 L CNN
F 1 "1.5M" H 5220 3105 50  0000 L CNN
F 2 "Resistor_SMD:R_2512_6332Metric" V 5080 3150 50  0001 C CNN
F 3 "~" H 5150 3150 50  0001 C CNN
F 4 "1 watt 5%" H 5150 3150 50  0001 C CNN "Notes"
	1    5150 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5C74AB2C
P 5150 4450
F 0 "R4" H 5220 4496 50  0000 L CNN
F 1 "10K" H 5220 4405 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5080 4450 50  0001 C CNN
F 3 "~" H 5150 4450 50  0001 C CNN
F 4 "Use any generic resistor" H 5150 4450 50  0001 C CNN "Notes"
	1    5150 4450
	1    0    0    -1  
$EndComp
$Comp
L Device:R_POT RV1
U 1 1 5C74AC62
P 5150 3950
F 0 "RV1" H 5080 3996 50  0000 R CNN
F 1 "5K" H 5080 3905 50  0000 R CNN
F 2 "Potentiometer_SMD:Potentiometer_Bourns_3214W_Vertical" H 5150 3950 50  0001 C CNN
F 3 "~" H 5150 3950 50  0001 C CNN
F 4 "Multi-turn - Any similar will do" H 5150 3950 50  0001 C CNN "Notes"
F 5 "Bourns 3214W-1-502E" H 5150 3950 50  0001 C CNN "SuggestedPart"
	1    5150 3950
	-1   0    0    -1  
$EndComp
$Comp
L Device:C C7
U 1 1 5C74AD86
P 5650 3850
F 0 "C7" H 5700 3950 50  0000 L CNN
F 1 "100 nF 250 V" H 5400 3750 50  0000 L CNN
F 2 "TubeClock:C_2420_HandSoldering" H 5688 3700 50  0001 C CNN
F 3 "~" H 5650 3850 50  0001 C CNN
F 4 "Optional" H 5650 3850 50  0001 C CNN "Notes"
F 5 "ECWU2104KC9" H 5650 3850 50  0001 C CNN "SuggestedPart"
	1    5650 3850
	1    0    0    -1  
$EndComp
$Comp
L Maxim:MAX1771 U1
U 1 1 5C74BCDB
P 3150 3650
F 0 "U1" H 2900 4050 50  0000 C CNN
F 1 "MAX1771" H 3350 4050 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 3150 3650 50  0001 C CNN
F 3 "" H 3150 3650 50  0001 C CNN
	1    3150 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 3000 3150 3100
Wire Wire Line
	5150 4700 5150 4600
Wire Wire Line
	2650 3900 2550 3900
Wire Wire Line
	2550 3900 2550 4700
Wire Wire Line
	4000 4300 4000 4700
Wire Wire Line
	4650 2900 4650 3200
Wire Wire Line
	4650 3500 4650 4700
Wire Wire Line
	5150 3000 5150 2900
Wire Wire Line
	5150 2900 4650 2900
Wire Wire Line
	5650 2900 5150 2900
Connection ~ 5150 2900
Wire Wire Line
	3150 2400 3450 2400
Wire Wire Line
	3450 2500 3450 2400
Wire Wire Line
	3450 2900 3450 2800
Wire Wire Line
	5150 4100 5150 4200
Wire Wire Line
	5000 3950 5000 4200
Wire Wire Line
	5000 4200 5150 4200
Connection ~ 5150 4200
Wire Wire Line
	5150 4200 5150 4300
Text Notes 700  7700 0    69   ~ 14
Note: The physical board layout is critical in order to obtain high efficiency!
Wire Wire Line
	6250 1100 6350 1100
$Comp
L TubeClock:+V_IN #PWR?
U 1 1 5C7B53FC
P 7100 3550
AR Path="/5C7B53FC" Ref="#PWR?"  Part="1" 
AR Path="/5A06BFE7/5C7B53FC" Ref="#PWR?"  Part="1" 
AR Path="/5C7A430C/5C7B53FC" Ref="#PWR?"  Part="1" 
AR Path="/5C7E1816/5C7B53FC" Ref="#PWR014"  Part="1" 
F 0 "#PWR014" H 7100 3400 50  0001 C CNN
F 1 "+V_IN" H 7100 3690 50  0000 C CNN
F 2 "" H 7100 3550 50  0001 C CNN
F 3 "" H 7100 3550 50  0001 C CNN
	1    7100 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 3550 7100 3750
Text Notes 700  7500 0    67   ~ 0
High Efficiency (typically 85%) SMPS for Nixie Tubes - 50 mA, 150 V to 220 V Adjustable\n\nBoost converter notes:\nQ1 should be selected for low RDSon, Qg and Coss. Ex.: Vishay IRF644PBF; an IRF740A (note the “A”) will work but is more lossy.\nR1, C3, C7 must be rated for >220 V.\nD2 must be an ultra-fast (<50nS) recovery — lower if possible. Ex.: Vishay ES2F (Mttr 35nS)\nC2 & C4 must be low ESR types. Ex.: AVX TPS series\nC4 & C5 must be located as close as possible to pin 2 of the MAX1771.\nC3 must be low ESR (<3 ohms) and rated for >220 V.\nRV1 adjusts output between 150 V and 220 V, multi-turn types allow for greater adjustment accuracy.\nL1 should be DC rated at 2+ A for 50+ mA output. Ex.: EPCOS B82479\nL1 may be rated 1 A for output currents up to 25mA; change Rsense to 0R100 ohms in this case. Ex.: Sumida CDRH125-101\nRsense must be capable of handling 2 A.\n\nThe traces between the MAX1771 and the FET must be short and FAT, i.e. be low resistance and inductance.\n\nIn the event of stability issues, a 100 pF 250 V ceramic capacitor may be soldered on top of R1.\nAn RF ferrite bead placed around the gate to the FET may help if the circuit shows a tendency to be sensitive\nto the presence of a hand/finger/etc. (e.g. when you move your hand near, you hear a whistle from the\ninductor or the output voltage varies.)\n\nOrigin: https://desmith.net/NMdS/Electronics/NixiePSU.html
Wire Wire Line
	5650 4000 5650 4700
$Comp
L power:GND #PWR025
U 1 1 5C7B9A0D
P 3150 4700
F 0 "#PWR025" H 3150 4450 50  0001 C CNN
F 1 "GND" H 3150 4550 50  0000 C CNN
F 2 "" H 3150 4700 50  0001 C CNN
F 3 "" H 3150 4700 50  0001 C CNN
	1    3150 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5C7B9D0C
P 3450 2900
F 0 "#PWR011" H 3450 2650 50  0001 C CNN
F 1 "GND" H 3450 2750 50  0000 C CNN
F 2 "" H 3450 2900 50  0001 C CNN
F 3 "" H 3450 2900 50  0001 C CNN
	1    3450 2900
	1    0    0    -1  
$EndComp
$Comp
L TubeClock:+V_HV #PWR09
U 1 1 5C7C1AEF
P 5650 2700
F 0 "#PWR09" H 5650 2550 50  0001 C CNN
F 1 "+V_HV" H 5665 2873 50  0000 C CNN
F 2 "" H 5650 2700 50  0001 C CNN
F 3 "" H 5650 2700 50  0001 C CNN
	1    5650 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5650 2900 5650 3700
Wire Wire Line
	5650 2700 5650 2900
Connection ~ 5650 2900
$Comp
L TubeClock:+V_IN #PWR?
U 1 1 5C7C6F4B
P 3150 2300
AR Path="/5C7C6F4B" Ref="#PWR?"  Part="1" 
AR Path="/5A06BFE7/5C7C6F4B" Ref="#PWR?"  Part="1" 
AR Path="/5C7A430C/5C7C6F4B" Ref="#PWR?"  Part="1" 
AR Path="/5C7E1816/5C7C6F4B" Ref="#PWR08"  Part="1" 
F 0 "#PWR08" H 3150 2150 50  0001 C CNN
F 1 "+V_IN" H 3150 2440 50  0000 C CNN
F 2 "" H 3150 2300 50  0001 C CNN
F 3 "" H 3150 2300 50  0001 C CNN
	1    3150 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 2400 3150 2300
Connection ~ 3150 2400
Wire Wire Line
	3150 2400 3150 3000
Connection ~ 3150 3000
Text HLabel 1100 3400 0    50   Input ~ 0
SHUTDOWN
$Comp
L power:GND #PWR021
U 1 1 5C7D4030
P 1650 4700
F 0 "#PWR021" H 1650 4450 50  0001 C CNN
F 1 "GND" H 1650 4550 50  0000 C CNN
F 2 "" H 1650 4700 50  0001 C CNN
F 3 "" H 1650 4700 50  0001 C CNN
	1    1650 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR022
U 1 1 5C7D412E
P 1950 4700
F 0 "#PWR022" H 1950 4450 50  0001 C CNN
F 1 "GND" H 1950 4550 50  0000 C CNN
F 2 "" H 1950 4700 50  0001 C CNN
F 3 "" H 1950 4700 50  0001 C CNN
	1    1950 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR023
U 1 1 5C7D4211
P 2250 4700
F 0 "#PWR023" H 2250 4450 50  0001 C CNN
F 1 "GND" H 2250 4550 50  0000 C CNN
F 2 "" H 2250 4700 50  0001 C CNN
F 3 "" H 2250 4700 50  0001 C CNN
	1    2250 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR024
U 1 1 5C7D42D9
P 2550 4700
F 0 "#PWR024" H 2550 4450 50  0001 C CNN
F 1 "GND" H 2550 4550 50  0000 C CNN
F 2 "" H 2550 4700 50  0001 C CNN
F 3 "" H 2550 4700 50  0001 C CNN
	1    2550 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR026
U 1 1 5C7D43BC
P 4000 4700
F 0 "#PWR026" H 4000 4450 50  0001 C CNN
F 1 "GND" H 4000 4550 50  0000 C CNN
F 2 "" H 4000 4700 50  0001 C CNN
F 3 "" H 4000 4700 50  0001 C CNN
	1    4000 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR027
U 1 1 5C7D4526
P 4650 4700
F 0 "#PWR027" H 4650 4450 50  0001 C CNN
F 1 "GND" H 4650 4550 50  0000 C CNN
F 2 "" H 4650 4700 50  0001 C CNN
F 3 "" H 4650 4700 50  0001 C CNN
	1    4650 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR028
U 1 1 5C7D4690
P 5150 4700
F 0 "#PWR028" H 5150 4450 50  0001 C CNN
F 1 "GND" H 5150 4550 50  0000 C CNN
F 2 "" H 5150 4700 50  0001 C CNN
F 3 "" H 5150 4700 50  0001 C CNN
	1    5150 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR029
U 1 1 5C7D4758
P 5650 4700
F 0 "#PWR029" H 5650 4450 50  0001 C CNN
F 1 "GND" H 5650 4550 50  0000 C CNN
F 2 "" H 5650 4700 50  0001 C CNN
F 3 "" H 5650 4700 50  0001 C CNN
	1    5650 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 4700 3150 4200
Wire Wire Line
	2250 3650 2650 3650
Wire Wire Line
	3600 2400 3450 2400
Connection ~ 3450 2400
Wire Wire Line
	4000 3600 4000 3650
Wire Wire Line
	3650 3900 4300 3900
Wire Wire Line
	3650 3650 4000 3650
Connection ~ 4000 3650
Wire Wire Line
	4000 3650 4000 4000
Wire Wire Line
	3650 3400 3700 3400
Wire Wire Line
	3900 2400 4000 2400
Wire Wire Line
	4000 2400 4000 2900
Wire Wire Line
	5150 3300 5150 3700
Wire Wire Line
	4300 3900 4300 3700
Wire Wire Line
	4300 3700 5150 3700
Connection ~ 5150 3700
Wire Wire Line
	5150 3700 5150 3800
Wire Wire Line
	4200 2900 4000 2900
Connection ~ 4000 2900
Wire Wire Line
	4000 2900 4000 3200
Wire Wire Line
	4500 2900 4650 2900
Connection ~ 4650 2900
Text Notes 5050 1900 0    50   ~ 0
DC input jack, fuse, polarity protection
Text Notes 7800 5150 0    50   ~ 0
Low-voltage regulation, battery/super-capacitor back-up
Text Notes 2850 5000 0    50   ~ 0
Boost converter for high voltage for tubes
Wire Wire Line
	1100 3400 1350 3400
Wire Wire Line
	1650 3000 1950 3000
Wire Wire Line
	2250 3700 2250 3650
Wire Wire Line
	1950 3700 1950 3000
Connection ~ 1950 3000
Wire Wire Line
	1950 3000 3150 3000
Wire Wire Line
	1650 3000 1650 3700
Wire Wire Line
	1650 4000 1650 4700
Wire Wire Line
	1950 4000 1950 4700
Wire Wire Line
	2250 4000 2250 4700
Wire Wire Line
	7900 3750 8100 3750
Wire Wire Line
	10400 3750 10650 3750
$Comp
L Device:C C10
U 1 1 5CA6B53C
P 8500 3900
F 0 "C10" H 8525 4000 50  0000 L CNN
F 1 "100 uF" H 8525 3800 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_5x5.3" H 8538 3750 50  0001 C CNN
F 3 "" H 8500 3900 50  0001 C CNN
	1    8500 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 3750 9100 3750
Wire Wire Line
	8100 3750 8500 3750
Connection ~ 8100 3750
Connection ~ 8500 3750
Wire Wire Line
	8500 3750 8500 3450
$Comp
L power:GND #PWR018
U 1 1 5CA78614
P 8500 4250
F 0 "#PWR018" H 8500 4000 50  0001 C CNN
F 1 "GND" H 8500 4100 50  0000 C CNN
F 2 "" H 8500 4250 50  0001 C CNN
F 3 "" H 8500 4250 50  0001 C CNN
	1    8500 4250
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 4050 8500 4250
$Comp
L Device:R R1
U 1 1 5CCA4D3E
P 1350 3150
F 0 "R1" H 1420 3196 50  0000 L CNN
F 1 "100K" H 1420 3105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1280 3150 50  0001 C CNN
F 3 "~" H 1350 3150 50  0001 C CNN
F 4 "Use any generic resistor" H 1350 3150 50  0001 C CNN "Notes"
	1    1350 3150
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR010
U 1 1 5CCA5EA2
P 1350 2750
F 0 "#PWR010" H 1350 2600 50  0001 C CNN
F 1 "+3.3V" H 1350 2890 50  0000 C CNN
F 2 "" H 1350 2750 50  0001 C CNN
F 3 "" H 1350 2750 50  0001 C CNN
	1    1350 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 3300 1350 3400
Connection ~ 1350 3400
Wire Wire Line
	1350 3400 2650 3400
Wire Wire Line
	1350 3000 1350 2750
$EndSCHEMATC
