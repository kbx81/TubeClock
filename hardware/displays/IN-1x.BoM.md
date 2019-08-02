# Nixie Tube Display Board Bill of Materials (BoM)

## Parts

|Reference                                           |Quantity|Value                    |Footprint                                                 |
|----------------------------------------------------|--------|-------------------------|----------------------------------------------------------|
|C2 C1 C3 C4 C5                                      |5       |100 nF, 16 V             |Capacitor_SMD:C_0805_2012Metric                           |
|D1                                                  |1       |1N4148                   |Diode_SMD:D_SOD-123                                       |
|J1                                                  |1       |Conn_02x06_Odd_Even      |Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical|
|J2                                                  |1       |Conn_02x06_Odd_Even      |Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Vertical|
|NE4 NE3 NE1 NE2                                     |4       |Lamp_Neon                |Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical|
|R10 R9 R8 R7                                        |4       |470K                     |Resistor_SMD:R_1206_3216Metric                            |
|R11 R12 R13 R14 R15 R16 R17 R18 R19 R20 R21 R22 R23 |13      |10K                      |Resistor_SMD:R_0805_2012Metric                            |
|R6 R3 R5 R4 R2 R1                                   |6       |10K (2K2 for IN-18) .25 W|Resistor_SMD:R_1206_3216Metric                            |
|T1 T2 T3 T4 T5 T6                                   |6       |IN-1x_Nixie_Tube         |TubeClock:Nixie_IN-1x                                     |
|U1                                                  |1       |CD40109B                 |Package_SO:SOIC-16W_5.3x10.2mm_P1.27mm                    |
|U2 U3 U4*                                           |3       |HV5622                   |Package_LCC:PLCC-44_16.6x16.6mm_P1.27mm                   |

## Additional parts you may need

For IN-12 and IN-18 boards, machine pin sockets are highly recommended -- **do not** solder the tubes into the boards! `Mill-Max` part number `0327-0-15-01-34-27-10-0` is a perfect fit for these tubes!

For the seperator/colon indicator lamps (NE1 through NE4), any "A1B" neon indicator lamp should suffice. I used `VCC` part number `A1B`. Some `INS-1` tubes should also work well, particularly on the IN-12 boards.

## IMPORTANT!

**Pay extra super close attention to the polarity of D1!** Bad things will happen if it is installed backwards. :(

### Notes

For IN-12 and IN-18 tubes, use machine pin sockets suggested above. Since IN-14 and IN-16 tubes have wire leads, they should be soldered directly into the boards.

*U4 is not present on IN-18 display boards. On the other boards, it is not required if you don't care about the lovely decimal points in the smaller tubes. :) **Bridge the solder jumper if U4 is *not* installed** on these boards.

R12 through R23 are additional series resistors for tube decimal points. They are not present on IN-18 boards and are otherwise not necessary if decimal points will not be used.
