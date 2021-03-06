EESchema Schematic File Version 4
LIBS:TubeClock-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 6 6
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
L TubeClock:Teseo-LIV3F U?
U 1 1 5CB7F384
P 8950 3800
AR Path="/5C7E16B1/5CB7F384" Ref="U?"  Part="1" 
AR Path="/5CB696D3/5CB7F384" Ref="U10"  Part="1" 
F 0 "U10" H 9400 3250 60  0000 C CNN
F 1 "Teseo-LIV3F" H 9500 4400 60  0000 C CNN
F 2 "TubeClock:LCC-18" H 8950 3800 60  0001 C CNN
F 3 "https://www.st.com/resource/en/datasheet/teseo-liv3f.pdf" H 8950 2750 60  0000 C CNN
F 4 "ST" H 8530 3360 60  0001 C CNN "Manufacturer"
	1    8950 3800
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5CB7F38A
P 8800 3050
AR Path="/5C7E16B1/5CB7F38A" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CB7F38A" Ref="#PWR075"  Part="1" 
F 0 "#PWR075" H 8800 2900 50  0001 C CNN
F 1 "+3.3V" H 8800 3190 50  0000 C CNN
F 2 "" H 8800 3050 50  0001 C CNN
F 3 "" H 8800 3050 50  0001 C CNN
	1    8800 3050
	1    0    0    -1  
$EndComp
$Comp
L power:+BATT #PWR?
U 1 1 5CB7F390
P 9150 3050
AR Path="/5C7E16B1/5CB7F390" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CB7F390" Ref="#PWR076"  Part="1" 
F 0 "#PWR076" H 9150 2900 50  0001 C CNN
F 1 "+BATT" H 9150 3190 50  0000 C CNN
F 2 "" H 9150 3050 50  0001 C CNN
F 3 "" H 9150 3050 50  0001 C CNN
	1    9150 3050
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_Coaxial J?
U 1 1 5CB7F396
P 1250 3500
AR Path="/5C7E16B1/5CB7F396" Ref="J?"  Part="1" 
AR Path="/5CB696D3/5CB7F396" Ref="J8"  Part="1" 
F 0 "J8" V 1224 3600 50  0000 L CNN
F 1 "Conn_Coaxial" V 1133 3600 50  0000 L CNN
F 2 "Connector_Coaxial:U.FL_Molex_MCRF_73412-0110_Vertical" H 1250 3500 50  0001 C CNN
F 3 " ~" H 1250 3500 50  0001 C CNN
	1    1250 3500
	-1   0    0    -1  
$EndComp
$Comp
L win:GS2 SB?
U 1 1 5CB7F39C
P 9900 3500
AR Path="/5C7E16B1/5CB7F39C" Ref="SB?"  Part="1" 
AR Path="/5CB696D3/5CB7F39C" Ref="SB7"  Part="1" 
F 0 "SB7" H 10000 3650 50  0000 C CNN
F 1 "GS2" H 10000 3351 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm" V 9974 3500 50  0001 C CNN
F 3 "" H 9900 3500 50  0001 C CNN
	1    9900 3500
	0    -1   -1   0   
$EndComp
$Comp
L win:GS2 SB?
U 1 1 5CB7F3A2
P 9900 3600
AR Path="/5C7E16B1/5CB7F3A2" Ref="SB?"  Part="1" 
AR Path="/5CB696D3/5CB7F3A2" Ref="SB8"  Part="1" 
F 0 "SB8" H 10000 3750 50  0000 C CNN
F 1 "GS2" H 10000 3451 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm" V 9974 3600 50  0001 C CNN
F 3 "" H 9900 3600 50  0001 C CNN
	1    9900 3600
	0    -1   -1   0   
$EndComp
Text HLabel 7800 4100 0    60   Output ~ 0
SQW_OUT_GPS
Wire Wire Line
	8800 3150 8900 3150
Wire Wire Line
	9150 3150 9150 3050
Wire Wire Line
	9600 3500 9700 3500
Wire Wire Line
	9600 3600 9700 3600
$Comp
L power:GND #PWR?
U 1 1 5CB7F3BA
P 8950 4600
AR Path="/5C7E16B1/5CB7F3BA" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CB7F3BA" Ref="#PWR087"  Part="1" 
F 0 "#PWR087" H 8950 4350 50  0001 C CNN
F 1 "GND" H 8950 4450 50  0000 C CNN
F 2 "" H 8950 4600 50  0001 C CNN
F 3 "" H 8950 4600 50  0001 C CNN
	1    8950 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4450 9050 4500
Wire Wire Line
	8950 4450 8950 4500
Wire Wire Line
	8950 4500 9050 4500
Wire Wire Line
	8850 4450 8850 4500
Wire Wire Line
	8850 4500 8950 4500
Connection ~ 8950 4500
NoConn ~ 9600 4100
NoConn ~ 9600 4000
NoConn ~ 9600 3850
NoConn ~ 9600 3750
NoConn ~ 8300 3950
Text HLabel 7800 3800 0    60   Output ~ 0
RESET
Wire Wire Line
	7800 3800 8300 3800
Text HLabel 10200 3600 2    60   Input ~ 0
USART1_TX
Text HLabel 10200 3500 2    60   Output ~ 0
USART1_RX
Wire Wire Line
	10100 3500 10200 3500
Wire Wire Line
	10200 3600 10100 3600
$Comp
L power:GND #PWR?
U 1 1 5CB7F3D4
P 1250 3800
AR Path="/5C7E16B1/5CB7F3D4" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CB7F3D4" Ref="#PWR078"  Part="1" 
F 0 "#PWR078" H 1250 3550 50  0001 C CNN
F 1 "GND" H 1250 3650 50  0000 C CNN
F 2 "" H 1250 3800 50  0001 C CNN
F 3 "" H 1250 3800 50  0001 C CNN
	1    1250 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C34
U 1 1 5CB9BA29
P 1750 2900
F 0 "C34" H 1865 2946 50  0000 L CNN
F 1 "120 pF" H 1865 2855 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1788 2750 50  0001 C CNN
F 3 "~" H 1750 2900 50  0001 C CNN
	1    1750 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:C C35
U 1 1 5CB9BD1B
P 2650 3500
F 0 "C35" H 2765 3546 50  0000 L CNN
F 1 "120 pF" H 2765 3455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2688 3350 50  0001 C CNN
F 3 "~" H 2650 3500 50  0001 C CNN
	1    2650 3500
	0    1    1    0   
$EndComp
$Comp
L Device:C C38
U 1 1 5CB9C0B9
P 4250 3750
F 0 "C38" H 4365 3796 50  0000 L CNN
F 1 "3.9 pF" H 4365 3705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 4288 3600 50  0001 C CNN
F 3 "~" H 4250 3750 50  0001 C CNN
	1    4250 3750
	1    0    0    -1  
$EndComp
$Comp
L Device:L L2
U 1 1 5CB9C5BD
P 2000 2900
F 0 "L2" H 2053 2946 50  0000 L CNN
F 1 "100 nH" H 2053 2855 50  0000 L CNN
F 2 "Inductor_SMD:L_0402_1005Metric" H 2000 2900 50  0001 C CNN
F 3 "~" H 2000 2900 50  0001 C CNN
	1    2000 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:L L3
U 1 1 5CB9C8C3
P 3250 3500
F 0 "L3" H 3303 3546 50  0000 L CNN
F 1 "5.6 nH" H 3303 3455 50  0000 L CNN
F 2 "Inductor_SMD:L_0402_1005Metric" H 3250 3500 50  0001 C CNN
F 3 "~" H 3250 3500 50  0001 C CNN
	1    3250 3500
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8800 3050 8800 3150
Connection ~ 8800 3150
$Comp
L TubeClock:BGA824N6 U9
U 1 1 5CBA446C
P 3700 3500
F 0 "U9" H 3850 3350 50  0000 L CNN
F 1 "BGA824N6" H 3850 3650 50  0000 L CNN
F 2 "TubeClock:TSNP-6-2_1.2x0.8mm" H 3850 3500 50  0001 C CNN
F 3 "https://www.infineon.com/dgdl/Infineon-BGA824N6-DS-v03_00-en.pdf?fileId=db3a30433f764301013f7b53cdd02721" H 3950 3600 50  0001 C CNN
	1    3700 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1250 3700 1250 3800
$Comp
L power:GND #PWR?
U 1 1 5CBCFAA4
P 1750 3150
AR Path="/5C7E16B1/5CBCFAA4" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CBCFAA4" Ref="#PWR077"  Part="1" 
F 0 "#PWR077" H 1750 2900 50  0001 C CNN
F 1 "GND" H 1750 3000 50  0000 C CNN
F 2 "" H 1750 3150 50  0001 C CNN
F 3 "" H 1750 3150 50  0001 C CNN
	1    1750 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	1450 3500 2000 3500
Wire Wire Line
	2000 3500 2000 3050
Wire Wire Line
	1750 3150 1750 3050
$Comp
L TubeClock:+V_ANT #PWR072
U 1 1 5CBD20C3
P 2000 2650
F 0 "#PWR072" H 2000 2500 50  0001 C CNN
F 1 "+V_ANT" H 2015 2823 50  0000 C CNN
F 2 "" H 2000 2650 50  0001 C CNN
F 3 "" H 2000 2650 50  0001 C CNN
	1    2000 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 2750 2000 2700
Wire Wire Line
	1750 2750 1750 2700
Wire Wire Line
	1750 2700 2000 2700
Connection ~ 2000 2700
Wire Wire Line
	2000 2700 2000 2650
Connection ~ 2000 3500
$Comp
L Device:C C37
U 1 1 5CBD40BA
P 3000 3750
F 0 "C37" H 3115 3796 50  0000 L CNN
F 1 "3.9 pF" H 3115 3705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 3038 3600 50  0001 C CNN
F 3 "~" H 3000 3750 50  0001 C CNN
	1    3000 3750
	-1   0    0    1   
$EndComp
Wire Wire Line
	2800 3500 3000 3500
Wire Wire Line
	3400 3500 3500 3500
Wire Wire Line
	3000 3600 3000 3500
Connection ~ 3000 3500
Wire Wire Line
	3000 3500 3100 3500
Text GLabel 3700 2850 1    50   Input ~ 0
ANT_OFF
Text GLabel 7800 3650 0    50   Output ~ 0
ANT_OFF
Wire Wire Line
	7800 3650 8300 3650
$Comp
L Device:C C39
U 1 1 5CBD8C1B
P 4750 3750
F 0 "C39" H 4865 3796 50  0000 L CNN
F 1 "3.9 pF" H 4865 3705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 4788 3600 50  0001 C CNN
F 3 "~" H 4750 3750 50  0001 C CNN
	1    4750 3750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBD91D7
P 3000 4000
AR Path="/5C7E16B1/5CBD91D7" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CBD91D7" Ref="#PWR079"  Part="1" 
F 0 "#PWR079" H 3000 3750 50  0001 C CNN
F 1 "GND" H 3000 3850 50  0000 C CNN
F 2 "" H 3000 4000 50  0001 C CNN
F 3 "" H 3000 4000 50  0001 C CNN
	1    3000 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBD961D
P 3700 4000
AR Path="/5C7E16B1/5CBD961D" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CBD961D" Ref="#PWR080"  Part="1" 
F 0 "#PWR080" H 3700 3750 50  0001 C CNN
F 1 "GND" H 3700 3850 50  0000 C CNN
F 2 "" H 3700 4000 50  0001 C CNN
F 3 "" H 3700 4000 50  0001 C CNN
	1    3700 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBD9854
P 4250 4000
AR Path="/5C7E16B1/5CBD9854" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CBD9854" Ref="#PWR081"  Part="1" 
F 0 "#PWR081" H 4250 3750 50  0001 C CNN
F 1 "GND" H 4250 3850 50  0000 C CNN
F 2 "" H 4250 4000 50  0001 C CNN
F 3 "" H 4250 4000 50  0001 C CNN
	1    4250 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBDA041
P 4750 4000
AR Path="/5C7E16B1/5CBDA041" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CBDA041" Ref="#PWR082"  Part="1" 
F 0 "#PWR082" H 4750 3750 50  0001 C CNN
F 1 "GND" H 4750 3850 50  0000 C CNN
F 2 "" H 4750 4000 50  0001 C CNN
F 3 "" H 4750 4000 50  0001 C CNN
	1    4750 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 3900 3000 4000
Wire Wire Line
	3700 3700 3700 3850
Wire Wire Line
	3800 3700 3800 3850
Wire Wire Line
	3800 3850 3700 3850
Connection ~ 3700 3850
Wire Wire Line
	3700 3850 3700 4000
Wire Wire Line
	4250 3900 4250 4000
Wire Wire Line
	4750 3900 4750 4000
Wire Wire Line
	4100 3500 4250 3500
Wire Wire Line
	4250 3500 4250 3600
Wire Wire Line
	4750 3600 4750 3500
Wire Wire Line
	4750 3500 4250 3500
Connection ~ 4250 3500
$Comp
L TubeClock:+V_RF #PWR073
U 1 1 5CBE644E
P 9000 3000
F 0 "#PWR073" H 9000 2850 50  0001 C CNN
F 1 "+V_RF" H 9000 3150 50  0000 C CNN
F 2 "" H 9000 3000 50  0001 C CNN
F 3 "" H 9000 3000 50  0001 C CNN
	1    9000 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	9000 3000 9000 3150
$Comp
L TubeClock:+V_RF #PWR071
U 1 1 5CBE7B8D
P 3800 2600
F 0 "#PWR071" H 3800 2450 50  0001 C CNN
F 1 "+V_RF" H 3800 2750 50  0000 C CNN
F 2 "" H 3800 2600 50  0001 C CNN
F 3 "" H 3800 2600 50  0001 C CNN
	1    3800 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:C C33
U 1 1 5CBE8E9D
P 4000 2850
F 0 "C33" H 4115 2896 50  0000 L CNN
F 1 "1 nF" H 4115 2805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4038 2700 50  0001 C CNN
F 3 "~" H 4000 2850 50  0001 C CNN
	1    4000 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CBE9F42
P 4000 3050
AR Path="/5C7E16B1/5CBE9F42" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CBE9F42" Ref="#PWR074"  Part="1" 
F 0 "#PWR074" H 4000 2800 50  0001 C CNN
F 1 "GND" H 4000 2900 50  0000 C CNN
F 2 "" H 4000 3050 50  0001 C CNN
F 3 "" H 4000 3050 50  0001 C CNN
	1    4000 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 3000 4000 3050
Wire Wire Line
	3800 2600 3800 2650
Wire Wire Line
	4000 2700 4000 2650
Wire Wire Line
	4000 2650 3800 2650
Connection ~ 3800 2650
Wire Wire Line
	3800 2650 3800 3200
Wire Wire Line
	3700 2850 3700 3200
$Comp
L Device:C C36
U 1 1 5CBEE984
P 7150 3500
F 0 "C36" H 7265 3546 50  0000 L CNN
F 1 "120 pF" H 7265 3455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 7188 3350 50  0001 C CNN
F 3 "~" H 7150 3500 50  0001 C CNN
	1    7150 3500
	0    1    1    0   
$EndComp
Wire Wire Line
	8300 3500 7800 3500
Wire Wire Line
	2000 3500 2350 3500
Connection ~ 2350 3500
Wire Wire Line
	2350 3500 2500 3500
Connection ~ 7800 3500
Wire Wire Line
	7800 3500 7300 3500
$Comp
L TubeClock:B39162B4327P810 FL1
U 1 1 5CC0DEF1
P 5500 3600
F 0 "FL1" H 5500 3967 50  0000 C CNN
F 1 "B39162B4327P810" H 5500 3876 50  0000 C CNN
F 2 "TubeClock:Filter_1411-5_1.4x1.1mm" H 5500 3600 50  0001 C CNN
F 3 "https://en.rf360jv.com/inf/40/ds/ae/B4327.pdf" H 5430 3630 50  0001 C CNN
	1    5500 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 3500 4750 3500
Connection ~ 4750 3500
$Comp
L power:GND #PWR?
U 1 1 5CC11236
P 5150 4000
AR Path="/5C7E16B1/5CC11236" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CC11236" Ref="#PWR083"  Part="1" 
F 0 "#PWR083" H 5150 3750 50  0001 C CNN
F 1 "GND" H 5150 3850 50  0000 C CNN
F 2 "" H 5150 4000 50  0001 C CNN
F 3 "" H 5150 4000 50  0001 C CNN
	1    5150 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CC11825
P 5850 4000
AR Path="/5C7E16B1/5CC11825" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CC11825" Ref="#PWR084"  Part="1" 
F 0 "#PWR084" H 5850 3750 50  0001 C CNN
F 1 "GND" H 5850 3850 50  0000 C CNN
F 2 "" H 5850 4000 50  0001 C CNN
F 3 "" H 5850 4000 50  0001 C CNN
	1    5850 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 4000 5850 3700
Wire Wire Line
	5850 3700 5800 3700
Wire Wire Line
	5150 4000 5150 3700
Wire Wire Line
	5150 3700 5200 3700
Wire Wire Line
	2350 2250 7800 2250
$Comp
L Device:C C40
U 1 1 5CC71CED
P 6250 3750
F 0 "C40" H 6365 3796 50  0000 L CNN
F 1 "3.9 pF" H 6365 3705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 6288 3600 50  0001 C CNN
F 3 "~" H 6250 3750 50  0001 C CNN
	1    6250 3750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C41
U 1 1 5CC7233D
P 6750 3750
F 0 "C41" H 6865 3796 50  0000 L CNN
F 1 "3.9 pF" H 6865 3705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 6788 3600 50  0001 C CNN
F 3 "~" H 6750 3750 50  0001 C CNN
	1    6750 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 3500 6250 3600
Wire Wire Line
	5800 3500 6250 3500
Wire Wire Line
	6250 3500 6750 3500
Connection ~ 6250 3500
Wire Wire Line
	6750 3600 6750 3500
Connection ~ 6750 3500
Wire Wire Line
	6750 3500 7000 3500
$Comp
L power:GND #PWR?
U 1 1 5CC76EEC
P 6250 4000
AR Path="/5C7E16B1/5CC76EEC" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CC76EEC" Ref="#PWR085"  Part="1" 
F 0 "#PWR085" H 6250 3750 50  0001 C CNN
F 1 "GND" H 6250 3850 50  0000 C CNN
F 2 "" H 6250 4000 50  0001 C CNN
F 3 "" H 6250 4000 50  0001 C CNN
	1    6250 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CC773CB
P 6750 4000
AR Path="/5C7E16B1/5CC773CB" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CC773CB" Ref="#PWR086"  Part="1" 
F 0 "#PWR086" H 6750 3750 50  0001 C CNN
F 1 "GND" H 6750 3850 50  0000 C CNN
F 2 "" H 6750 4000 50  0001 C CNN
F 3 "" H 6750 4000 50  0001 C CNN
	1    6750 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	6750 4000 6750 3900
Wire Wire Line
	6250 3900 6250 4000
$Comp
L TubeClock:TPS22943 U11
U 1 1 5CC82772
P 3800 5800
F 0 "U11" H 3800 6267 50  0000 C CNN
F 1 "TPS22943" H 3800 6176 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-353_SC-70-5" H 3800 5400 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/tps22929d.pdf" H 3700 6250 50  0001 C CNN
	1    3800 5800
	1    0    0    -1  
$EndComp
Text GLabel 2550 5900 0    50   Input ~ 0
ANT_OFF
$Comp
L TubeClock:+V_RF #PWR088
U 1 1 5CC850E5
P 3300 5500
F 0 "#PWR088" H 3300 5350 50  0001 C CNN
F 1 "+V_RF" H 3300 5650 50  0000 C CNN
F 2 "" H 3300 5500 50  0001 C CNN
F 3 "" H 3300 5500 50  0001 C CNN
	1    3300 5500
	1    0    0    -1  
$EndComp
$Comp
L Device:C C42
U 1 1 5CC87851
P 3300 6150
F 0 "C42" H 3415 6196 50  0000 L CNN
F 1 "10 uF" H 3415 6105 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3338 6000 50  0001 C CNN
F 3 "~" H 3300 6150 50  0001 C CNN
	1    3300 6150
	-1   0    0    1   
$EndComp
$Comp
L TubeClock:+V_ANT #PWR089
U 1 1 5CC8BF68
P 4300 5500
F 0 "#PWR089" H 4300 5350 50  0001 C CNN
F 1 "+V_ANT" H 4300 5650 50  0000 C CNN
F 2 "" H 4300 5500 50  0001 C CNN
F 3 "" H 4300 5500 50  0001 C CNN
	1    4300 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 5600 4300 5600
Wire Wire Line
	4300 5600 4300 5500
$Comp
L Device:C C43
U 1 1 5CC8E543
P 4300 6150
F 0 "C43" H 4415 6196 50  0000 L CNN
F 1 "1 nF" H 4415 6105 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4338 6000 50  0001 C CNN
F 3 "~" H 4300 6150 50  0001 C CNN
	1    4300 6150
	-1   0    0    1   
$EndComp
Connection ~ 4300 5600
$Comp
L power:GND #PWR?
U 1 1 5CC92CF0
P 3800 6500
AR Path="/5C7E16B1/5CC92CF0" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CC92CF0" Ref="#PWR091"  Part="1" 
F 0 "#PWR091" H 3800 6250 50  0001 C CNN
F 1 "GND" H 3800 6350 50  0000 C CNN
F 2 "" H 3800 6500 50  0001 C CNN
F 3 "" H 3800 6500 50  0001 C CNN
	1    3800 6500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 6100 3800 6350
Wire Wire Line
	4300 5600 4300 6000
Wire Wire Line
	3400 5600 3300 5600
Wire Wire Line
	3300 5600 3300 6000
Wire Wire Line
	3300 6300 3300 6350
Wire Wire Line
	3300 6350 3800 6350
Connection ~ 3800 6350
Wire Wire Line
	3800 6350 3800 6500
Wire Wire Line
	3800 6350 4300 6350
Wire Wire Line
	4300 6350 4300 6300
Wire Wire Line
	3300 5500 3300 5600
Connection ~ 3300 5600
$Comp
L Device:R R33
U 1 1 5CCC7858
P 2800 6150
F 0 "R33" H 2870 6196 50  0000 L CNN
F 1 "10K" H 2870 6105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2730 6150 50  0001 C CNN
F 3 "~" H 2800 6150 50  0001 C CNN
	1    2800 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2550 5900 2800 5900
Wire Wire Line
	2800 6000 2800 5900
Connection ~ 2800 5900
Wire Wire Line
	2800 5900 3400 5900
Wire Wire Line
	2800 6300 2800 6350
Wire Wire Line
	2800 6350 3300 6350
Connection ~ 3300 6350
$Comp
L Device:R R32
U 1 1 5CCCEA2A
P 4550 5900
F 0 "R32" V 4343 5900 50  0000 C CNN
F 1 "100K" V 4434 5900 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4480 5900 50  0001 C CNN
F 3 "~" H 4550 5900 50  0001 C CNN
	1    4550 5900
	0    1    1    0   
$EndComp
Wire Wire Line
	4200 5900 4400 5900
$Comp
L TubeClock:+V_RF #PWR090
U 1 1 5CCD1756
P 4750 5500
F 0 "#PWR090" H 4750 5350 50  0001 C CNN
F 1 "+V_RF" H 4750 5650 50  0000 C CNN
F 2 "" H 4750 5500 50  0001 C CNN
F 3 "" H 4750 5500 50  0001 C CNN
	1    4750 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 5500 4750 5900
Wire Wire Line
	4750 5900 4700 5900
Wire Wire Line
	8950 4500 8950 4600
Text HLabel 5500 1800 0    60   Output ~ 0
SQW_OUT_GPS
$Comp
L power:+3.3V #PWR?
U 1 1 5CD20BD5
P 5650 900
AR Path="/5C7E16B1/5CD20BD5" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CD20BD5" Ref="#PWR068"  Part="1" 
F 0 "#PWR068" H 5650 750 50  0001 C CNN
F 1 "+3.3V" H 5650 1040 50  0000 C CNN
F 2 "" H 5650 900 50  0001 C CNN
F 3 "" H 5650 900 50  0001 C CNN
	1    5650 900 
	1    0    0    -1  
$EndComp
Text HLabel 5500 1400 0    60   Output ~ 0
USART1_RX
Text HLabel 5500 1500 0    60   Input ~ 0
USART1_TX
$Comp
L power:+BATT #PWR?
U 1 1 5CD21882
P 5900 900
AR Path="/5C7E16B1/5CD21882" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CD21882" Ref="#PWR069"  Part="1" 
F 0 "#PWR069" H 5900 750 50  0001 C CNN
F 1 "+BATT" H 5900 1040 50  0000 C CNN
F 2 "" H 5900 900 50  0001 C CNN
F 3 "" H 5900 900 50  0001 C CNN
	1    5900 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 1800 5500 1800
Wire Wire Line
	6000 1500 5500 1500
Wire Wire Line
	5500 1400 6000 1400
$Comp
L power:GND #PWR?
U 1 1 5CD21350
P 5900 1900
AR Path="/5C7E16B1/5CD21350" Ref="#PWR?"  Part="1" 
AR Path="/5CB696D3/5CD21350" Ref="#PWR070"  Part="1" 
F 0 "#PWR070" H 5900 1650 50  0001 C CNN
F 1 "GND" H 5900 1750 50  0000 C CNN
F 2 "" H 5900 1900 50  0001 C CNN
F 3 "" H 5900 1900 50  0001 C CNN
	1    5900 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 1600 5900 1600
Wire Wire Line
	5900 1600 5900 1900
Wire Wire Line
	6000 1700 5800 1700
Wire Wire Line
	5800 1700 5800 1300
Wire Wire Line
	5800 1300 5650 1300
Wire Wire Line
	5650 1300 5650 900 
Wire Wire Line
	6000 1200 5900 1200
Wire Wire Line
	5900 1200 5900 900 
NoConn ~ 6000 1300
$Comp
L Connector_Generic:Conn_01x07 J7
U 1 1 5C9E16CD
P 6200 1500
F 0 "J7" H 6118 975 50  0000 C CNN
F 1 "Conn_01x07" H 6118 1066 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x07_P2.54mm_Vertical" H 6200 1500 50  0001 C CNN
F 3 "~" H 6200 1500 50  0001 C CNN
	1    6200 1500
	1    0    0    1   
$EndComp
Wire Wire Line
	7800 4100 8300 4100
$Comp
L Device:R R30
U 1 1 5CBFFC30
P 2350 2900
F 0 "R30" H 2420 2946 50  0000 L CNN
F 1 "0" H 2420 2855 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 2280 2900 50  0001 C CNN
F 3 "~" H 2350 2900 50  0001 C CNN
	1    2350 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:R R31
U 1 1 5CC00C18
P 7800 2900
F 0 "R31" H 7870 2946 50  0000 L CNN
F 1 "0" H 7870 2855 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 7730 2900 50  0001 C CNN
F 3 "~" H 7800 2900 50  0001 C CNN
	1    7800 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 2250 7800 2750
Wire Wire Line
	7800 3050 7800 3500
Wire Wire Line
	2350 2250 2350 2750
Wire Wire Line
	2350 3050 2350 3500
$EndSCHEMATC
