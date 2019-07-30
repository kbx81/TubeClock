|Reference                                           |Quantity|Value              |Footprint                                                 |
|----------------------------------------------------|--------|-------------------|----------------------------------------------------------|
|C2 C1 C3 C4 C5                                      |5       |100 nF             |Capacitor_SMD:C_0805_2012Metric                           |
|D1                                                  |1       |1N4148             |Diode_SMD:D_SOD-123                                       |
|J1                                                  |1       |Conn_02x06_Odd_Even|Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical|
|J2                                                  |1       |Conn_02x06_Odd_Even|Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Vertical|
|NE4 NE3 NE1 NE2                                     |4       |Lamp_Neon          |Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical|
|R10 R9 R8 R7                                        |4       |470K               |Resistor_SMD:R_1206_3216Metric                            |
|R11 R12 R13 R14 R15 R16 R17 R18 R19 R20 R21 R22 R23 |13      |10K                |Resistor_SMD:R_0805_2012Metric                            |
|R6 R3 R5 R4 R2 R1                                   |6       |10K (2K2 for IN-18)|Resistor_SMD:R_1206_3216Metric                            |
|T1 T2 T3 T4 T5 T6                                   |6       |IN-1x_Nixie_Tube   |TubeClock:Nixie_IN-1x                                     |
|U1                                                  |1       |CD40109B           |Package_SO:SOIC-16W_5.3x10.2mm_P1.27mm                    |
|U2 U3 U4*                                           |3       |HV5622             |Package_LCC:PLCC-44_16.6x16.6mm_P1.27mm                   |

* U4 is not required for IN-18 tubes...or if you don't care about the lovely decimal points in smaller tubes. :) **Bridge the solder jumper if U4 is *not* installed.**

***** **Pay extra super close attention to the polarity of D1!** ***** Bad things will happen if it is installed backwards. :(
